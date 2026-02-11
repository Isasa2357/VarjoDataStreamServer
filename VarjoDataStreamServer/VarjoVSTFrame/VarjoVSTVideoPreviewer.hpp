#pragma once

#include <string>
#include <format>
#include <vector>
#include <queue>
#include <cstdint>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "varjo_vst_frame_type.hpp"
#include "utility.hpp"
#include "ISubmitFrame.hpp"

namespace VarjoVSTFrame {

	enum class VideoPreviewerType {
		Serial, Parallel
	};

	/**
	 * @brief VarjoVSTFrameの動画プレビューを行うクラス
	 * @detail ffmpegとパイプを使用して，リアルタイムで動画プレビューを行う．
	 */
	class VarjoVSTVideoPreviewer : public ISubmitFramedata {

	public:

		VarjoVSTVideoPreviewer(
			const size_t width,
			const size_t height,
			const size_t row_stride, 
			const InputFramedataPaddingOption pad_opt
		);

		~VarjoVSTVideoPreviewer();

		virtual bool open();

		void submit_framedata(const Framedata& framedata, const Metadata& metadata);
		void submit_framedata(const Framedata& framedata, Metadata&& metadata);
		void submit_framedata(Framedata&& frameData, const Metadata& metadata);
		void submit_framedata(Framedata&& frameData, Metadata&& metadata);

		virtual void close();

	protected:
		virtual void submit_framedata_impl(Framedata&& frameData, Metadata&& metadata) = 0;

		std::string get_ffmpegCmd() const;

	protected:
		const size_t width_;
		const size_t height_;
		const size_t row_stride_;
		const InputFramedataPaddingOption pad_opt_;

		FILE* ffmpeg_pipe_;
	};

	class VarjoVSTSerialVideoPreviewer : public VarjoVSTVideoPreviewer {

	public:
		VarjoVSTSerialVideoPreviewer(
			const size_t width,
			const size_t height,
			const size_t row_stride, 
			const InputFramedataPaddingOption pad_opt
		);

	protected:
		void submit_framedata_impl(Framedata&& frameData, Metadata&& metadata) override;

	protected:
		std::vector<uint8_t> tight_frameData_;
	};

	class VarjoVSTParallelVideoPreviewer : public VarjoVSTVideoPreviewer {

	public:
		VarjoVSTParallelVideoPreviewer(
			const size_t width,
			const size_t height,
			const size_t row_stride, 
			const InputFramedataPaddingOption pad_opt
		);

		bool open() override;

		void close() override;

	protected:

		void submit_framedata_impl(Framedata&& frameData, Metadata&& metadata) override;
		void video_preview_worker();

	protected:
		std::queue<std::vector<uint8_t>> frameData_submitQue_;
		std::mutex submitQue_mutex_;
		std::condition_variable submitQue_cv_;
		std::thread video_preview_worker_thread_;
		std::atomic_bool stop_worker_signal_{true};
	};

	struct VarjoVSTVideoPreviewerOptions {
		size_t width;
		size_t height;
		size_t row_stride;
		InputFramedataPaddingOption pad_opt;
		VideoPreviewerType previewer_type;
	};

	VarjoVSTVideoPreviewerOptions make_VarjoVSTVideoPreviewerOptions(
		const size_t width,
		const size_t height,
		const size_t row_stride,
		const InputFramedataPaddingOption pad_opt,
		const VideoPreviewerType previewer_type
	);

	std::unique_ptr<VarjoVSTVideoPreviewer> factory_VarjoVSTVideoPreviewer_ptr(const VarjoVSTVideoPreviewerOptions& opt);
	std::unique_ptr<ISubmitFramedata> factory_ISubmitFrame_ptr(const VarjoVSTVideoPreviewerOptions& opt);
	

} // namespace VarjoVSTFrame

