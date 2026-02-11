#include "VarjoVSTVideoPreviewer.hpp"

namespace VarjoVSTFrame {
	VarjoVSTVideoPreviewer::VarjoVSTVideoPreviewer(const size_t width, const size_t height, const size_t row_stride)
		: width_(width), height_(height), force_tight_(row_stride != 0), row_stride_(row_stride), ffmpeg_pipe_(nullptr)
	{
	}

	VarjoVSTVideoPreviewer::~VarjoVSTVideoPreviewer() {
		this->close();
	}

	bool VarjoVSTVideoPreviewer::open() {
		this->ffmpeg_pipe_ = _popen(this->get_ffmpegCmd().c_str(), "wb");
		return this->ffmpeg_pipe_ != nullptr;
	}

	void VarjoVSTVideoPreviewer::submit_frame(const std::vector<uint8_t>& frameData) {
		this->submit_frame_impl(std::vector<uint8_t>(frameData));
	}

	void VarjoVSTVideoPreviewer::submit_frame(std::vector<uint8_t>&& frameData) {
		this->submit_frame_impl(std::move(frameData));
	}

	void VarjoVSTVideoPreviewer::close() {
		if (this->ffmpeg_pipe_ != nullptr) {
			fflush(this->ffmpeg_pipe_);
			_pclose(this->ffmpeg_pipe_);
		}
	}

	std::string VarjoVSTVideoPreviewer::get_ffmpegCmd() const {
		return std::format("ffplay -hide_banner -loglevel error -f rawvideo -pixel_format nv12 -video_size {}x{} -framerate 90 -use_wallclock_as_timestamps 1 -i - -sync ext -fflags nobuffer -flags low_delay -noinfbuf -framedrop -an -autoexit", this->width_, this->height_);
	}

	//-------------------- VarjoVST Serial Video Previewer --------------------

	VarjoVSTSerialVideoPreviewer::VarjoVSTSerialVideoPreviewer(
		const size_t width,
		const size_t height,
		const size_t row_stride
	) 
		: VarjoVSTVideoPreviewer(width, height, row_stride) 
	{
		this->tight_frameData_.resize(this->width_ * this->height_ * 3 / 2);
	}

	void VarjoVSTSerialVideoPreviewer::submit_frame_impl(std::vector<uint8_t>&& frameData) {
		if (this->force_tight_) 
			remove_padding(frameData, tight_frameData_, this->width_, this->height_, this->row_stride_);

		fwrite((this->force_tight_ ? tight_frameData_.data() : frameData.data()), 1, tight_frameData_.size(), this->ffmpeg_pipe_);
	}
	VarjoVSTParallelVideoPreviewer::VarjoVSTParallelVideoPreviewer(const size_t width, const size_t height, const size_t row_stride)
		: VarjoVSTVideoPreviewer(width, height, row_stride)
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

	void VarjoVSTParallelVideoPreviewer::submit_frame_impl(std::vector<uint8_t> && frameData) {
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
				if (this->force_tight_) {
					tight_frameData = remove_padding(frameData_que_toDisplay.front(), this->width_, this->height_, this->row_stride_);
				} else {
					tight_frameData = std::move(frameData_que_toDisplay.front());
				}

				fwrite(tight_frameData.data(), sizeof(uint8_t), tight_frameData.size(), this->ffmpeg_pipe_);
				frameData_que_toDisplay.pop();
			}
		}
	}
}
