#include "VarjoVSTVideoWriter.hpp"

namespace {
	std::string check_out_path(
		const std::string& out_path, 
		const VarjoVSTFrame::VideoContainer container, 
		const VarjoVSTFrame::EncodeOptions encode_opt
	) {
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
		const VideoWriteEncodeOptions vw_encode_opt, 
		const size_t row_stride, 
		const InputFramedataPaddingOption pad_opt)
		: width_(vw_encode_opt.width)
		, height_(vw_encode_opt.height)
		, row_stride_(row_stride)
		, framerate_(vw_encode_opt.framerate)
		, out_path_(vw_encode_opt.out_path)
		, vcontainer_(vw_encode_opt.container)
		, encode_opt_(vw_encode_opt.encode_opt)
		, ffmpeg_pipe_(nullptr)
		, pad_opt_(pad_opt)
		, tight_frameData_(std::vector<uint8_t>(this->width_ * this->height_ * 3 / 2))
	{}

	VarjoVSTVideoWriter::~VarjoVSTVideoWriter()
	{
		this->close();
	}

	void VarjoVSTVideoWriter::submit_framedata(const Framedata& framedata, const Metadata& metadata)
	{
		this->submit_framedata_impl(Framedata(framedata), Metadata(metadata));
	}

	void VarjoVSTVideoWriter::submit_framedata(const Framedata & framedata, Metadata && metadata)
	{
		this->submit_framedata_impl(Framedata(framedata), std::move(metadata));
	}

	void VarjoVSTVideoWriter::submit_framedata(Framedata && framedata, const Metadata & metadata)
	{
		this->submit_framedata_impl(std::move(framedata), Metadata(metadata));
	}

	void VarjoVSTVideoWriter::submit_framedata(Framedata && framedata, Metadata && metadata)
	{
		this->submit_framedata_impl(std::move(framedata), std::move(metadata));
	}

	std::string VarjoVSTVideoWriter::get_ffmpegCmd() const
	{
		//----- 入力部分のコマンドを作成
		const std::string input_part = std::format(
			"ffmpeg -hide_banner -loglevel error -y "
			"-f rawvideo -pix_fmt nv12 -video_size {}x{} -framerate {} -i pipe:0 "
			"-an ",
			this->width_, this->height_, this->framerate_
		);

		//----- コンテナ別のコマンドを作成
		std::string each_container_cmd_part = "";
		if (this->vcontainer_ == VideoContainer::mp4) {
			each_container_cmd_part = "-movflags +faststart ";
		} else if (this->vcontainer_ == VideoContainer::mkv) {
			// 何もなし
		} 

		// エンコード部分のコマンドを作成
		std::string encode_part = "";
		if (std::holds_alternative<X264Options>(this->encode_opt_)) {
			auto x264_encodeopt = std::get<X264Options>(this->encode_opt_);
			encode_part = std::format(
				"-c:v libx264 -preset {} -pix_fmt yuv420p {}",
				x264Preset_toString(x264_encodeopt.preset), out_path_
			);
		} else if (std::holds_alternative<NvencH264Options>(this->encode_opt_)) {

		} else if (std::holds_alternative<Ffv1Options>(this->encode_opt_)) {

		}

		return input_part + each_container_cmd_part + encode_part;
	}
	// -------------------- VarjoVST Serial Video Writer --------------------

	VarjoVSTSerialVideoWriter::VarjoVSTSerialVideoWriter(
		const VideoWriteEncodeOptions vw_encode_opt, 
		const size_t row_stride, 
		const InputFramedataPaddingOption pad_opt)
		: VarjoVSTVideoWriter(vw_encode_opt, row_stride, pad_opt)
	{}

	VarjoVSTSerialVideoWriter::~VarjoVSTSerialVideoWriter() {
		this->close();
	}

	bool VarjoVSTSerialVideoWriter::open()
	{
		auto ffmpeg_cmd = this->get_ffmpegCmd();
		printf(ffmpeg_cmd.c_str());

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

	void VarjoVSTSerialVideoWriter::submit_framedata_impl(Framedata&& frameData, Metadata&& metadata)
	{
		if (this->pad_opt_ == InputFramedataPaddingOption::WithPadding) {
			remove_padding(frameData, this->tight_frameData_, this->width_, this->height_, this->row_stride_);
		} else {
			this->tight_frameData_ = std::move(frameData);
		}
		std::cout << "write start\n";
		std::cout << "tight_frameData_ size: " << this->tight_frameData_.size() << "\n";

		size_t written_size = fwrite(
			this->tight_frameData_.data(),
			sizeof(uint8_t),
			this->tight_frameData_.size(),
			this->ffmpeg_pipe_
		);
		std::cout << "write done\n";
	}

	// -------------------- VarjoVST Parallel Video Writer --------------------

	VarjoVSTParallelVideoWriter::VarjoVSTParallelVideoWriter(
		const VideoWriteEncodeOptions vw_encode_opt, 
		const size_t row_stride, 
		const InputFramedataPaddingOption pad_opt)
		: VarjoVSTVideoWriter(vw_encode_opt, row_stride, pad_opt)
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

	void VarjoVSTParallelVideoWriter::submit_framedata_impl(Framedata&& frameData, Metadata&& metadata) {
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
		const VideoWriterType writer_type, 
		const VideoWriteEncodeOptions vw_encode_opt, 
		const size_t row_stride, 
		const InputFramedataPaddingOption pad_opt
	) {
		VarjoVSTVideoWriterOptions opt;
		opt.row_stride = row_stride;
		opt.vw_encode_opt = vw_encode_opt;
		opt.writer_type = writer_type;
		opt.pad_opt = pad_opt;

		return opt;
	}

	std::unique_ptr<VarjoVSTVideoWriter> factory_VideoWriterPtr(const VarjoVSTVideoWriterOptions opt) {
		if (opt.writer_type == VideoWriterType::Serial) {
			return std::unique_ptr<VarjoVSTSerialVideoWriter>(
				new VarjoVSTSerialVideoWriter(
					opt.vw_encode_opt, 
					opt.row_stride, 
					opt.pad_opt
				)
			);
		} else if (opt.writer_type == VideoWriterType::Parallel) {
			return std::unique_ptr<VarjoVSTParallelVideoWriter>(
				new VarjoVSTParallelVideoWriter(
					opt.vw_encode_opt,
					opt.row_stride,
					opt.pad_opt
				)
			);
		} else {
			throw std::invalid_argument("bad VideoWriterType exception");
		}
	}
}