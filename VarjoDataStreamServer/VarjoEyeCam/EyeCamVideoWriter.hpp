#pragma once

#include <deque>

#include "EyeCam_types.hpp"
#include "ISubmitEyeCam.hpp"

namespace EyeCam {

	enum class VideoWriterType {
		Serial, Parallel
	};
	
	class VideoWriter : public ISubmitFrame {
	public:

		VideoWriter(
			const varjo_ChannelFlag channel_flag,
			const size_t width,
			const size_t height,
			const size_t row_stride,
			const EncodeOptions& encode_opt, 
			const FrameLayout layout
		);

		~VideoWriter();

		virtual bool open();

		virtual void close();

	protected:

		static std::string get_ffmpegCmd(
			const EncodeOptions& opt, 
			const size_t width,
			const size_t height,
			const int framerate, 
			const std::string& path
		);

		void write_to_pipe();

	public:

		// getter
		inline varjo_ChannelFlag channel_flag() const { return this->channel_flag_; }
		inline size_t width() const { return this->width_; }
		inline size_t height() const { return this->height_; }
		inline size_t row_stride() const { return this->row_stride_; }
		inline EncodeOptions encode_opt() const { return this->encode_opt_; }
		inline FrameLayout layout() const { return this->layout_; }
		inline bool is_opened_left() const { return this->lpipe_ != nullptr; }
		inline bool is_opened_right() const { return this->rpipe_ != nullptr; }
		inline bool is_write_left() const { return (this->channel_flag_ & varjo_ChannelFlag_Left); }
		inline bool is_write_right() const { return (this->channel_flag_ & varjo_ChannelFlag_Right); }

	protected:

		const varjo_ChannelFlag channel_flag_;
		const size_t width_;
		const size_t height_;
		const size_t row_stride_;
		const EncodeOptions encode_opt_;
		const FrameLayout layout_;
		std::deque<Frame> lframe_que_;
		std::deque<Frame> rframe_que_;
		Framedata ltight_framedata_;
		Framedata rtight_framedata_;
		FILE* lpipe_;
		FILE* rpipe_;
	};

	class SerialVideoWriter : public VideoWriter {

	public:
		SerialVideoWriter(
			const varjo_ChannelFlag channel_flag,
			const size_t width,
			const size_t height,
			const size_t row_stride,
			const EncodeOptions& encode_opt,
			const FrameLayout layout
		);

		void submit_Frame(const Frame& frame) override;
		void submit_Frame(Frame&& frame) override;

		void submit_Frame(const std::vector<Frame>& frames) override;
		void submit_Frame(std::vector<Frame>&& frames) override;

		void submit_Frame(std::queue<Frame>& frames) override;
		void submit_Frame(std::queue<Frame>&& frames) override;

	private:
		void submit_frame_impl(BorrowedOrOwned<Frame> frame) override;
	};

};