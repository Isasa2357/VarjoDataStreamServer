#include "VarjoVSTVideoWriter.hpp"

namespace {
	std::string check_out_path(const std::string& out_path, const VarjoVSTFrame::Codec codec, const VarjoVSTFrame::VideoContainer container) {
		//----- out_pathのコンテナ確認．設定と違えば強制変更
		auto container_str = VarjoVSTFrame::videoContainer_toString(container);

		// out pathの拡張子確認
		auto dot_pos = out_path.rfind(".");
		if (dot_pos != std::string::npos) {
			// 拡張子あり ("."があるなら拡張子があるという断定)

			auto extension = out_path.substr(dot_pos + 1, out_path.length() - dot_pos);

			// コンテナ不一致
			if (extension != container_str) return out_path + "." + container_str;

			// コンテナ一致
			return out_path;
		}
		
		// 拡張子なし
		return out_path + "." + container_str;
	}
}

namespace VarjoVSTFrame {

	VarjoVSTVideoWriter::VarjoVSTVideoWriter(
		const size_t width,
		const size_t height,
		const size_t row_stride,
		const Codec codec,
		const VideoContainer container,
		const std::string out_path,
		const int crf,
		const int framerate, 
		const InputFramedataPaddingOption pad_opt
	) : width_(width),
		height_(height),
		row_stride_(row_stride),
		codec_(codec),
		container_(container), 
		out_path_(check_out_path(out_path, codec, container)),
		crf_(crf),
		framerate_(framerate),
		ffmpeg_pipe_(nullptr), 
		pad_opt_(pad_opt)
	{
		if (this->pad_opt_ == InputFramedataPaddingOption::WithPadding) {
			this->tight_frameData_.resize(this->width_ * this->height_ * 3 / 2);
		} else {
			this->tight_frameData_.resize(0);
		}
	}

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
		const std::string input_part = std::format(
			"ffmpeg -y "
			"-f rawvideo -pix_fmt nv12 -video_size {}x{} -framerate {} -i pipe:0 "
			"-an ",
			this->width_, this->height_, this->framerate_
		);

		std::string ffmpegCmd_each_container_part = "";
		switch (this->container_) {
		case VideoContainer::mp4:
			ffmpegCmd_each_container_part = "-movflags +faststart ";
			break;
		case VideoContainer::mkv:
			break;
		}

		std::string encode_part = "";
		switch (this->codec_) {
		case Codec::libx264:
			encode_part = std::format(
				"-c:v libx264 -preset veryfase -crf {} -pix_fmt yuv420p {} {}",
				this->crf_, ffmpegCmd_each_container_part, this->out_path_
			);
			break;
		case Codec::h264_nvenc:
			encode_part = std::format(
				"-c:v h264_nvenc -preset p1 -rc vbr_hq -cq {} -pix_fmt yuv420p {} {}",
				this->crf_, ffmpegCmd_each_container_part, this->out_path_
			);
			break;
		case Codec::ffv1:
			encode_part = std::format(
				"-c:v ffv1 -level 3 -pix_fmt yuv420p {} {}", 
				this
			)
		}

		if (device == Device::GPU) {
			return std::format(
				"ffmpeg -y "
				"-f rawvideo -pix_fmt nv12 -video_size {}x{} -framerate {} -i pipe:0 "
				"-an -c:v {} "
				"-preset p1 "
				"-rc constqp -qp 0 "
				"-spatial_aq 0 -temporal_aq 0 "
				"-pix_fmt yuv420p "
				"-movflags +faststart \"{}\"",
				this->width_, this->height_, this->framerate_,
				codec_toString(this->codec_), this->out_path_);
		} else {
			return std::format(
				"ffmpeg -y "
				"-f rawvideo -pix_fmt nv12 -video_size {}x{} -framerate {} -i pipe:0 "
				"-an -c:v libx264 -preset veryfast -crf 0 "
				"-pix_fmt yuv420p "
				"-movflags +faststart \"{}\"",
				this->width_, this->height_, this->framerate_, this->out_path_);
		}
	}
	// -------------------- VarjoVST Serial Video Writer --------------------

	VarjoVSTSerialVideoWriter::VarjoVSTSerialVideoWriter(
		const size_t width,
		const size_t height,
		const size_t row_stride,
		const Codec codec,
		const VideoContainer container,
		const std::string out_path,
		const int crf,
		const int framerate, 
		const InputFramedataPaddingOption pad_opt
	) : VarjoVSTVideoWriter(
		width,
		height,
		row_stride,
		codec,
		container, 
		out_path,
		crf,
		framerate, 
		pad_opt)
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
		if (this->pad_opt_ == InputFramedataPaddingOption::WithPadding) {
			remove_padding(frameData, this->tight_frameData_, this->width_, this->height_, this->row_stride_);
		} else {
			this->tight_frameData_ = std::move(frameData);
		}
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
		const VideoContainer container, 
		const std::string out_path, 
		const int crf,
		const int framerate, 
		const InputFramedataPaddingOption pad_opt
	) : VarjoVSTVideoWriter(
		width,
		height,
		row_stride,
		codec,
		container,
		out_path,
		crf,
		framerate,
		pad_opt), 
		frameData_submitQue(std::deque<std::vector<uint8_t>>())
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

				if (this->pad_opt_ == InputFramedataPaddingOption::WithPadding) {
					remove_padding(frameData, this->tight_frameData_, this->width_, this->height_, this->row_stride_);
				} else {
					this->tight_frameData_ = std::move(frameData);
				}

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


	VarjoVSTVideoWriterOptions make_VideoWriterOption(
		const size_t width,
		const size_t height,
		const size_t row_stride,
		const Codec codec,
		const VideoContainer container,
		const std::string out_path,
		const int crf,
		const int framerate,
		const VideoWriterType writer_type,
		const InputFramedataPaddingOption pad_opt) {
		VarjoVSTVideoWriterOptions opt;

		opt.width = width;
		opt.height = height;
		opt.row_stride = row_stride;
		opt.codec = codec;
		opt.container = container;
		opt.out_path = out_path;
		opt.crf = crf;
		opt.framerate;
		opt.writer_type = writer_type;
		opt.pad_opt = pad_opt;

		return opt;
	}

	std::unique_ptr<VarjoVSTVideoWriter> factory_VideoWriterPtr(const VarjoVSTVideoWriterOptions opt) {
		if (opt.writer_type == VideoWriterType::Serial) {
			return std::unique_ptr<VarjoVSTSerialVideoWriter>(
				new VarjoVSTSerialVideoWriter(
					opt.width,
					opt.height,
					opt.row_stride,
					opt.codec,
					opt.container, 
					opt.out_path,
					opt.crf,
					opt.framerate, 
					opt.pad_opt
				)
			);
		} else if (opt.writer_type == VideoWriterType::Parallel) {
			return std::unique_ptr<VarjoVSTParallelVideoWriter>(
				new VarjoVSTParallelVideoWriter(
					opt.width,
					opt.height,
					opt.row_stride,
					opt.codec,
					opt.container, 
					opt.out_path,
					opt.crf,
					opt.framerate, 
					opt.pad_opt
				)
			);
		} else {
			throw std::invalid_argument("bad VideoWriterType exception");
		}
	}
}