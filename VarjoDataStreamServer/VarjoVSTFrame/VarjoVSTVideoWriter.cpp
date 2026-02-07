#include "VarjoVSTVideoWriter.hpp"

namespace VarjoVSTFrame {

	VarjoVSTVideoWriter::VarjoVSTVideoWriter(
		const size_t width,
		const size_t height,
		const size_t row_stride,
		const Codec codec,
		const std::string out_path,
		const int crf,
		const int framerate
	) : width_(width),
		height_(height),
		row_stride_(row_stride),
		codec_(codec),
		out_path_(out_path),
		crf_(crf),
		framerate_(framerate),
		ffmpeg_pipe_(nullptr)
	{}

	VarjoVSTVideoWriter::~VarjoVSTVideoWriter()
	{
		this->close();
	}

	void VarjoVSTVideoWriter::submit_frame(const std::vector<uint8_t>& frameData)
	{
		this->submit_frame_impl(std::move(std::vector<uint8_t>(frameData)));
	}

	void VarjoVSTVideoWriter::submit_frame(std::vector<uint8_t>&& frameData)
	{
		this->submit_frame_impl(std::move(frameData));
	}

	std::string VarjoVSTVideoWriter::get_ffmpegCmd() const
	{
		auto device = get_device_from_codec(this->codec_);

		if (device == Device::GPU) {
			return std::format(
				"ffmpeg -y -f rawvideo -pix_fmt nv12 -s:v {}x{} -framerate {} -i pipe:0 -an -c:v {} -preset p1 -rc vbr_hq -cq {} -pix_fmt yuv420p -movflags +faststart \"{}\"",
				this->width_, this->height_, this->framerate_, codec_toString(this->codec_), this->crf_, this->out_path_);
		} else if (device == Device::CPU) {
			return std::format(
				"ffmpeg -y -f rawvideo -pix_fmt nv12 -s:v {}x{} -framerate {} -i pipe:0 -an -c:v {} -preset veryfast -crf {} -pix_fmt yuv420p -movflags +faststart \"{}\"",
				this->width_, this->height_, this->framerate_, codec_toString(this->codec_), this->crf_, this->out_path_);
		} else {
			// デフォルトはCPUとする
			return std::format(
				"ffmpeg -y -f rawvideo -pix_fmt nv12 -s:v {}x{} -framerate {} -i pipe:0 -an -c:v {} -preset veryfast -crf {} -pix_fmt yuv420p -movflags +faststart \"{}\"",
				this->width_, this->height_, this->framerate_, codec_toString(this->codec_), this->crf_, this->out_path_);
		}
	}

	void VarjoVSTVideoWriter::remove_padding(const std::vector<uint8_t>& raw_frameData, std::vector<uint8_t>& out_frameData) const
	{
		// ----- Y, UVプレーンの作成
		size_t frameSize_withPadding = this->row_stride_ * this->height_;
		size_t frameSize_withoutPadding = this->width_ * this->height_;
		const std::span<const uint8_t> y_plane(raw_frameData.data(), frameSize_withPadding);
		const std::span<const uint8_t> uv_plane(raw_frameData.data() + frameSize_withPadding, int(frameSize_withPadding / 2));

		// Yプレーンのコピー
		for (auto i = 0; i < this->height_; ++i) {
			memcpy(
				out_frameData.data() + i * this->width_,
				y_plane.data() + i * this->row_stride_,
				this->width_
			);
		}

		// UVプレーンのコピー
		for (auto i = 0; i < this->height_ / 2; ++i) {
			memcpy(
				out_frameData.data() + frameSize_withoutPadding + i * this->width_,
				uv_plane.data() + i * this->row_stride_,
				this->width_
			);
		}
	}



	// -------------------- VarjoVST Serial Video Writer --------------------

	VarjoVSTSerialVideoWriter::VarjoVSTSerialVideoWriter(
		const size_t width,
		const size_t height,
		const size_t row_stride,
		const Codec codec,
		const std::string out_path,
		const int crf,
		const int framerate
	) : VarjoVSTVideoWriter(
		width,
		height,
		row_stride,
		codec,
		out_path,
		crf,
		framerate),
		tight_frameData_(width* height + (width * height) / 2)
	{}

	VarjoVSTSerialVideoWriter::~VarjoVSTSerialVideoWriter() {
		this->close();
	}

	bool VarjoVSTSerialVideoWriter::open()
	{
		auto ffmpeg_cmd = this->get_ffmpegCmd();

		this->ffmpeg_pipe_ = _popen(ffmpeg_cmd.c_str(), "wb");

		return this->ffmpeg_pipe_ != nullptr;
	}

	void VarjoVSTSerialVideoWriter::close()
	{
		if (this->ffmpeg_pipe_ != nullptr) {
			fflush(this->ffmpeg_pipe_);
			int rc = _pclose(this->ffmpeg_pipe_);
		}
	}

	void VarjoVSTSerialVideoWriter::submit_frame_impl(std::vector<uint8_t>&& frameData)
	{
		this->remove_padding(frameData, this->tight_frameData_);
		size_t written_size = fwrite(
			this->tight_frameData_.data(),
			sizeof(uint8_t),
			this->tight_frameData_.size(),
			this->ffmpeg_pipe_
		);
	}

	// -------------------- VarjoVST Parallel Video Writer --------------------

	VarjoVSTParallelVideoWriter::VarjoVSTParallelVideoWriter(
		const size_t width,
		const size_t height,
		const size_t row_stride,
		const Codec codec,
		const std::string out_path,
		const int crf,
		const int framerate,
		const size_t buffer_size
	) : VarjoVSTVideoWriter(
		width,
		height,
		row_stride,
		codec,
		out_path,
		crf,
		framerate),
		frameData_submitQue(std::deque<std::vector<uint8_t>>()),
		tight_frameData_(std::vector<uint8_t>(width* height + (width * height) / 2))
	{}

	VarjoVSTParallelVideoWriter::~VarjoVSTParallelVideoWriter() {
		this->close();
	}

	bool VarjoVSTParallelVideoWriter::open() {
		auto ffmpeg_cmd = this->get_ffmpegCmd();

		this->ffmpeg_pipe_ = _popen(ffmpeg_cmd.c_str(), "wb");

		// パイプを開けなかったら書き込みスレッドを起動せずに終了
		if (this->ffmpeg_pipe_ == nullptr) return false;

		// スレッドを起動
		this->stop_worker_signal_ = false;
		this->video_write_worker_thread_ = std::thread(&VarjoVSTParallelVideoWriter::video_write_worker, this);

		return this->ffmpeg_pipe_ != nullptr;
	}

	void VarjoVSTParallelVideoWriter::close() {

		// スレッド停止
		this->stop_worker_signal_ = true;

		// submitキューを全て書き出し
		this->submitQue_cv_.notify_all();

		if (this->video_write_worker_thread_.joinable()) this->video_write_worker_thread_.join();

		if (this->ffmpeg_pipe_ != nullptr) {
			_pclose(this->ffmpeg_pipe_);
		}
	}

	void VarjoVSTParallelVideoWriter::submit_frame_impl(std::vector<uint8_t>&& frameData) {
		{
			std::lock_guard submitQue_lk(this->submitQue_mutex_);

			//printf("que size: %d\n", this->frameData_submitQue.size());

			this->frameData_submitQue.push_back(std::move(frameData));
		}

		this->submitQue_cv_.notify_all();
	}

	void VarjoVSTParallelVideoWriter::video_write_worker()
	{
		while (!this->stop_worker_signal_) {

			// wait for submit
			std::unique_lock submitQue_lk(this->submitQue_mutex_);
			this->submitQue_cv_.wait(submitQue_lk, [this] {
				return !this->frameData_submitQue.empty() || this->stop_worker_signal_;
				});

			// move frame data
			std::deque<std::vector<uint8_t>> frameData_toWrite;
			std::swap(frameData_toWrite, this->frameData_submitQue);

			submitQue_lk.unlock();

			// write frame data
			while (!frameData_toWrite.empty()) {
				auto& frameData = frameData_toWrite.front();

				this->remove_padding(frameData, this->tight_frameData_);			// remove padding

				fwrite(
					this->tight_frameData_.data(),
					sizeof(uint8_t),
					this->tight_frameData_.size(),
					this->ffmpeg_pipe_
				);
				frameData_toWrite.pop_front();

			}
		}
	}


}