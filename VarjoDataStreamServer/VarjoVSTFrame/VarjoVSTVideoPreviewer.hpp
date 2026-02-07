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

namespace VarjoVSTFrame {

	/**
	 * @brief VarjoVSTFrameの動画プレビューを行うクラス
	 * @detail ffmpegとパイプを使用して，リアルタイムで動画プレビューを行う．
	 */
	class VarjoVSTVideoPreviewer {

	public:

		VarjoVSTVideoPreviewer(
			const size_t width,
			const size_t height,
			const size_t row_stride = 0
		);

		~VarjoVSTVideoPreviewer();

		virtual bool open();

		void submit_frame(const std::vector<uint8_t>& frameData);
		void submit_frame(std::vector<uint8_t>&& frameData);

		virtual void close();

	protected:
		virtual void submit_frame_impl(std::vector<uint8_t>&& frameData) {};

		std::string get_ffmpegCmd() const;

	protected:
		const size_t width_;
		const size_t height_;
		const size_t row_stride_;
		const bool force_tight_;

		FILE* ffmpeg_pipe_;
	};

	class VarjoVSTSerialVideoPreviewer : public VarjoVSTVideoPreviewer {

	public:
		VarjoVSTSerialVideoPreviewer(
			const size_t width,
			const size_t height,
			const size_t row_stride = 0
		);

	protected:
		void submit_frame_impl(std::vector<uint8_t>&& frameData) override;

	protected:
		std::vector<uint8_t> tight_frameData_;
	};

	class VarjoVSTParallelVideoPreviewer : public VarjoVSTVideoPreviewer {

	public:
		VarjoVSTParallelVideoPreviewer(
			const size_t width,
			const size_t height,
			const size_t row_stride = 0
		);

		bool open() override;

		void close() override;

	protected:

		void submit_frame_impl(std::vector<uint8_t>&& frameData) override;
		void video_preview_worker();

	protected:
		std::queue<std::vector<uint8_t>> frameData_submitQue_;
		std::mutex submitQue_mutex_;
		std::condition_variable submitQue_cv_;
		std::thread video_preview_worker_thread_;
		std::atomic_bool stop_worker_signal_{true};
	};

} // namespace VarjoVSTFrame

