#include "VarjoVSTVideoPreviewer.hpp"

namespace VarjoVSTFrame {
	VarjoVSTVideoPreviewer::VarjoVSTVideoPreviewer(
		const size_t width, 
		const size_t height, 
		const size_t row_stride, 
		const const InputFramedataPaddingOption pad_opt
	)
		: width_(width), height_(height), pad_opt_(pad_opt), row_stride_(row_stride), ffmpeg_pipe_(nullptr)
	{
	}

	VarjoVSTVideoPreviewer::~VarjoVSTVideoPreviewer() {
		this->close();
	}

	bool VarjoVSTVideoPreviewer::open() {
		this->ffmpeg_pipe_ = _popen(this->get_ffmpegCmd().c_str(), "wb");
		return this->ffmpeg_pipe_ != nullptr;
	}

	void VarjoVSTVideoPreviewer::submit_framedata(const Framedata& framedata, const Metadata& metadata)
	{
		this->submit_framedata_impl(Framedata(framedata), Metadata(metadata));
	}

	void VarjoVSTVideoPreviewer::submit_framedata(const Framedata & framedata, Metadata && metadata)
	{
		this->submit_framedata_impl(Framedata(framedata), Metadata(std::move(metadata)));
	}

	void VarjoVSTVideoPreviewer::submit_framedata(Framedata && frameData, const Metadata & metadata)
	{
		this->submit_framedata_impl(std::move(frameData), Metadata(metadata));
	}

	void VarjoVSTVideoPreviewer::submit_framedata(Framedata && frameData, Metadata && metadata)
	{
		this->submit_framedata_impl(std::move(frameData), std::move(metadata));
	}

	void VarjoVSTVideoPreviewer::close() {
		if (this->ffmpeg_pipe_ != nullptr) {
			fflush(this->ffmpeg_pipe_);
			_pclose(this->ffmpeg_pipe_);
		}
	}

	std::string VarjoVSTVideoPreviewer::get_ffmpegCmd() const {
		return std::format("ffplay -hide_banner -loglevel error "
			"-f rawvideo -pixel_format nv12 -video_size {}x{} -framerate 90 "
			"-use_wallclock_as_timestamps 1 -i - -sync ext -fflags nobuffer "
			"-flags low_delay -noinfbuf -framedrop -an -autoexit"
			, this->width_, this->height_);
	}

	//-------------------- VarjoVST Serial Video Previewer --------------------

	VarjoVSTSerialVideoPreviewer::VarjoVSTSerialVideoPreviewer(
		const size_t width,
		const size_t height,
		const size_t row_stride, 
		const InputFramedataPaddingOption pad_opt
	) 
		: VarjoVSTVideoPreviewer(width, height, row_stride, pad_opt) 
	{
		this->tight_frameData_.resize(this->width_ * this->height_ * 3 / 2);
	}

	void VarjoVSTSerialVideoPreviewer::submit_framedata_impl(Framedata&& frameData, Metadata&& metadata) {
		if (this->pad_opt_ == InputFramedataPaddingOption::WithPadding) {
			remove_padding(frameData, this->tight_frameData_, this->width_, this->height_, this->row_stride_);
		} else {
			this->tight_frameData_ = std::move(frameData);
		}

		fwrite(this->tight_frameData_.data(), 1, tight_frameData_.size(), this->ffmpeg_pipe_);
	}
	VarjoVSTParallelVideoPreviewer::VarjoVSTParallelVideoPreviewer(
		const size_t width, 
		const size_t height, 
		const size_t row_stride, 
		const InputFramedataPaddingOption pad_opt)
		: VarjoVSTVideoPreviewer(width, height, row_stride, pad_opt)
	{}

	bool VarjoVSTParallelVideoPreviewer::open()
	{
		// パイプを開く
		int ok = VarjoVSTVideoPreviewer::open();

		if (!ok) return false;

		// スレッド起動
		this->stop_worker_signal_ = false;
		this->video_preview_worker_thread_ = std::thread(&VarjoVSTParallelVideoPreviewer::video_preview_worker, this);

		return ok;
	}

	void VarjoVSTParallelVideoPreviewer::close() {
		// スレッドを終了
		this->stop_worker_signal_ = true;
		this->submitQue_cv_.notify_all();
		this->video_preview_worker_thread_.join();

		// パイプを閉じる
		_pclose(this->ffmpeg_pipe_);
	}

	void VarjoVSTParallelVideoPreviewer::submit_framedata_impl(Framedata&& frameData, Metadata&& metadata) {
		std::lock_guard<std::mutex> lock(this->submitQue_mutex_);
		this->frameData_submitQue_.push(std::move(frameData));
		this->submitQue_cv_.notify_all();
	}
	
	void VarjoVSTParallelVideoPreviewer::video_preview_worker() {

		std::vector<uint8_t> tight_frameData(this->width_ * this->height_ * 3 / 2);
		std::queue<std::vector<uint8_t>> frameData_que_toDisplay;
		
		while (!this->stop_worker_signal_) {

			{
				std::unique_lock<std::mutex> lock(this->submitQue_mutex_);
				// フレーム送信待ち
				this->submitQue_cv_.wait(lock, [this] {
					return !this->frameData_submitQue_.empty() || this->stop_worker_signal_;
					});

				// フレーム取り出し(最新のフレームを)
				std::swap(frameData_que_toDisplay, this->frameData_submitQue_);
			}

			// 表示
			while (!frameData_que_toDisplay.empty()) {
				if (this->pad_opt_ == InputFramedataPaddingOption::WithPadding) {
					tight_frameData = remove_padding(frameData_que_toDisplay.front(), this->width_, this->height_, this->row_stride_);
				} else {
					tight_frameData = std::move(frameData_que_toDisplay.front());
				}

				fwrite(tight_frameData.data(), sizeof(uint8_t), tight_frameData.size(), this->ffmpeg_pipe_);
				frameData_que_toDisplay.pop();
			}
		}
	}
	VarjoVSTVideoPreviewerOptions make_VarjoVSTVideoPreviewerOptions(
		const size_t width,
		const size_t height, 
		const size_t row_stride, 
		const InputFramedataPaddingOption pad_opt,
		const VideoPreviewerType previewer_type)
	{
		VarjoVSTVideoPreviewerOptions opt;

		opt.width = width;
		opt.height = height;
		opt.row_stride = row_stride;
		opt.pad_opt = pad_opt;
		opt.previewer_type = previewer_type;
		return opt;
	}


	std::unique_ptr<VarjoVSTVideoPreviewer> factory_VideoPreviewerPtr(const VarjoVSTVideoPreviewerOptions& opt) {
		switch (opt.previewer_type) {
		case VideoPreviewerType::Serial:
			return std::make_unique<VarjoVSTSerialVideoPreviewer>(
				opt.width,
				opt.height,
				opt.row_stride,
				opt.pad_opt
			);
		case VideoPreviewerType::Parallel:
			return std::make_unique<VarjoVSTParallelVideoPreviewer>(
				opt.width,
				opt.height,
				opt.row_stride,
				opt.pad_opt
			);
		default:
			throw std::runtime_error("Unsupported Video Previewer Type");
		}
	}
}
