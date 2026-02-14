#pragma once

#include "varjo_vst_frame_type.hpp"
#include "VarjoVSTCamStreamer.hpp"
#include "VarjoVSTVideoPreviewer.hpp"
#include "VarjoVSTVideoWriter.hpp"
#include "ISubmitFrame.hpp"

namespace VarjoVSTFrame {

	/**
	 * @brief フレームデータをディスクに書き込むクラス．
	 * @detail 医者からのデータセット採取に間に合わせるにはServerを作る時間がなかったため，実装．本実装では使わない
	 */
	class DataLogger : public ISubmitFrame {
	public:
		explicit DataLogger(
			const size_t width,
			const size_t height,
			const size_t row_stride,
			const InputFramedataPaddingOption pad_opt
		);

		void open_writer(const VideoWriterOptions& opt);

		//void open_previewer(const VideoWriterOptinos& opt);

		void close();

		void submit_frame(const Frame& frame) override;
		void submit_frame(Frame&& frame) override;

	private:

		void submit_frame_impl(Frame&& frame) override;

	private:
		const size_t width_;
		const size_t height_;
		const size_t row_stride_;
		const InputFramedataPaddingOption pad_opt_;
		std::unique_ptr<VideoWriter> video_writer_;
		std::unique_ptr<VarjoVSTVideoPreviewer> video_previewer_;

		Framedata tight_framedata_;

	public:

		// getter
		inline bool is_opened_writer() const { return this->video_writer_ != nullptr; }
		inline bool is_opened_previewer() const { return this->video_previewer_ != nullptr; }
	};
}

