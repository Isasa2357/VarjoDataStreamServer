
#include "EyeCamVideoWriter.hpp"

#include "EyeCam_util.hpp"

namespace EyeCam {

	VideoWriter::VideoWriter(
		const varjo_ChannelFlag channel_flag, 
		const size_t width, 
		const size_t height, 
		const size_t row_stride,
		const EncodeOptions& encode_opt, 
		const FrameLayout layout)
		: channel_flag_(channel_flag)
		, width_(width)
		, height_(height)
		, row_stride_(row_stride)
		, encode_opt_(encode_opt)
		, layout_(layout)
		, lpipe_(nullptr)
		, rpipe_(nullptr)
	{
		if (this->is_write_left()) {
			this->ltight_framedata_.resize(this->width_ * this->height_);
		}

		if (this->is_write_right()) {
			this->rtight_framedata_.resize(this->width_ * this->height_);
		}
	}

	VideoWriter::~VideoWriter()
	{
		this->close();
	}

	bool VideoWriter::open() {
		if (this->is_write_left()) {
			auto ffmpeg_cmd = this->get_ffmpegCmd(this->encode_opt_);
			this->lpipe_ = _popen(ffmpeg_cmd.c_str(), "wb");
			if (this->lpipe_ == nullptr) return false;
		}

		if (this->is_write_right()) {
			auto ffmpeg_cmd = this->get_ffmpegCmd(this->encode_opt_);
			this->rpipe_ = _popen(ffmpeg_cmd.c_str(), "wb");
			if (this->rpipe_ == nullptr) return false;
		}

		return true;
	}

	std::string EyeCam::VideoWriter::get_ffmpegCmd(
		const EncodeOptions& opt, 
		const size_t width, 
		const size_t height, 
		const int framerate, 
		const std::string& path)
	{
		if (std::holds_alternative<X264Options>(opt)) {
			auto x264_encodeopt = std::get<X264Options>(opt);
			
		} else if (std::holds_alternative<NvencH264Options>(opt)) {
			auto nvenc_encodeopt = std::get<NvencH264Options>(opt);

		} else if (std::holds_alternative<Ffv1Options>(opt)) {
			auto ffv1_encodeopt = std::get<Ffv1Options>(opt);

			return std::format("ffmpeg -y -loglevel error -f rawvideo -pix_fmt gray -s {}x{} -r {} -i pipe:0 -c:v ffv1 -level {} -g 1 -pix_fmt gray -an {}",
				width, height, framerate, framerate, ffv1_encodeopt.level, path);
		}
		
		throw std::invalid_argument("unsupported encode options");
	}

	void VideoWriter::write_to_pipe()
	{
		// left
		while (!this->lframe_que_.empty()) {
			
			// 取り出し
			Frame f = std::move(this->lframe_que_.front());
			Framedata framedata = std::move(f.data);
			Metadata metadata = std::move(f.metadata);
			this->lframe_que_.pop_front();

			// パディング処理
			if (this->layout_ == FrameLayout::Strided) {
				remove_padding(f.data, this->ltight_framedata_, this->width_, this->height_, this->row_stride_);
			} else {
				this->ltight_framedata_ = std::move(framedata);
			}

			// 書き込み
			fwrite(
				f.data.data(),
				sizeof(uint8_t),
				f.data.size(),
				this->lpipe_
			);
		}
	}

} // namespace EyeCam