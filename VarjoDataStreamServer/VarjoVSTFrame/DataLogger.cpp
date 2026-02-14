
#include "DataLogger.hpp"
#include <iostream>

namespace VarjoVSTFrame
{
	DataLogger::DataLogger(
		const size_t width,
		const size_t height,
		const size_t row_stride,
		const InputFramedataPaddingOption pad_opt
	) 
		: width_(width)
		, height_(height)
		, row_stride_(row_stride)
		, pad_opt_(pad_opt)
		, video_writer_(nullptr)
		, video_previewer_(nullptr)
	{
		this->tight_framedata_.resize(width * height * 3 / 2);
		std::cout << "tight size: " << this->tight_framedata_.size() << std::endl;
	}

	void DataLogger::open_writer(const VideoWriterOptions& opt)
	{
		// 既にVideoWriterが閉じているなら例外を投げる
		if (this->video_writer_ != nullptr) {
			throw std::runtime_error("Video writer is already opened.");
		}

		this->video_writer_ = factory_VideoWriterPtr(opt);
	}

	void DataLogger::close()
	{
		this->video_writer_->close();
		this->video_writer_ = nullptr;
		this->video_previewer_->close();
		this->video_previewer_ = nullptr;
	}

	void DataLogger::submit_frame(const Frame& frame)
	{
		this->submit_frame_impl(Frame(frame));
	}

	void DataLogger::submit_frame(Frame&& frame)
	{
		this->submit_frame_impl(std::move(frame));
	}

	void DataLogger::submit_frame_impl(Frame&& frame)
	{
		if (this->pad_opt_ == InputFramedataPaddingOption::WithPadding) {
			remove_padding(frame.data, this->tight_framedata_, this->width_, this->height_, this->row_stride_);
		} else {
			this->tight_framedata_ = std::move(frame.data);
		}

		if (this->is_opened_writer()) {
			this->video_writer_->submit_frame(Frame{
				.metadata = frame.metadata,
				.data = this->tight_framedata_
			});
		}

		if (this->is_opened_previewer()) {
			this->video_previewer_->submit_framedata(this->tight_framedata_);
		}
	}
}