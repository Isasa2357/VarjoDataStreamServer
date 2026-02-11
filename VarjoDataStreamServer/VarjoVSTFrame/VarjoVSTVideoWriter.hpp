#pragma once

#include <string>
#include <vector>
#include <span>
#include <format>
#include <deque>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <variant>

#include "utility.hpp"
#include "varjo_vst_frame_type.hpp"


namespace VarjoVSTFrame {

	enum class VideoWriterType {
		Serial, Parallel
	};

	struct VarjoVSTVideoWriterOptions;

	/**
	 * @brief VarjoのData Stream APIから取得できるVSTカメラフレームを動画として書き出すクラス
	 * @detail
	 *  - VSTカメラフレームのパディングを削除しつつ，スレッド処理で動画を書き出す．
	 *  - 動画の書き出しにはffmpegを使用．そのため，ffmpegがシステムにインストールされている必要がある．
	 *  - ffmpegはpopenで起動し，パイプを通じてフレームデータを渡す．
	 *  - 内部でspanを使っているため，C++20以降の環境が必要．
	 *
	 * @future
	 *  - ffmpeg APIで書き出しを行う
	 *  - 動画フォーマットの選択肢を増やす
	 *  - エラーハンドリングを充実させる
	 *  - 歪み補正機能の追加
	 *  - 動画のメタデータ（タイムスタンプなど）を保存する機能の追加
	 */
	class VarjoVSTVideoWriter {

	public:

		/**
		 * @brief コンストラクタ
		 *
		 * @param options 動画書き出しオプション
		 */
		VarjoVSTVideoWriter(
			const VideoWriteEncodeOptions vw_encode_opt,
			const size_t row_stride,
			const InputFramedataPaddingOption pad_opt
		);

		/**
		 * @brief デストラクタ
		 */
		~VarjoVSTVideoWriter();

		/**
		 * @brief 動画の書き出しの準備をする
		 *
		 * @return パイプのオープンに成功したらtrue
		 */
		virtual bool open() { return false; };

		/**
		 * @brief フレームを書き出す
		 * @detail パディングつきフレームのパディングを削除し，パイプを経由してffmpegに渡す
		 *
		 * @param frameData NV12フォーマットのフレームデータ
		 *
		 * @note
		 *  - frameDataはData Stream APIから得たフレームデータを直接入れればよい
		 *  - 並列書き出しを有効にしている場合，内部でコピーが発生するため，move推奨．
		 */
		void submit_frame(const std::vector<uint8_t>& frameData);
		void submit_frame(std::vector<uint8_t>&& frameData);

		/**
		 * @brief 動画の書き出しを終了し，ffmpegパイプを閉じる
		 */
		virtual void close() {};

		inline size_t width() const { return this->width_; }
		inline size_t height() const { return this->height_; }
		inline size_t row_stride() const { return this->row_stride_; }
		inline std::string out_path() const { return this->out_path_; }

	protected:

		/**
		 * @brief writeFrameの共通処理
		 *
		 * @param frameData NV12フォーマットのフレームデータ
		 *
		 * @return 書き出しに成功したらtrue, 失敗したらfalse
		 */
		virtual void submit_frame_impl(std::vector<uint8_t>&& frameData) {}

		/**
		 * @brief ffmpegを起動するためのコマンドを取得する
		 *
		 * @return ffmpegを起動するコマンド
		 */
		inline std::string get_ffmpegCmd() const;

		// frameSize
		const size_t width_;							///! without padding
		const size_t height_;							///! without padding
		const size_t row_stride_;						///! width with padding

		// write video
		const int framerate_;
		const std::string out_path_;
		const VideoContainer vcontainer_;
		EncodeOptions encode_opt_;

		FILE* ffmpeg_pipe_;								///! ffmpeg pipe

		const InputFramedataPaddingOption pad_opt_;
		std::vector<uint8_t> tight_frameData_;
	};

	class VarjoVSTSerialVideoWriter : public VarjoVSTVideoWriter {

	public:
		VarjoVSTSerialVideoWriter(
			const VideoWriteEncodeOptions vw_encode_opt,
			const size_t row_stride,
			const InputFramedataPaddingOption pad_opt
		);

		~VarjoVSTSerialVideoWriter();

		bool open() override;

		void close() override;

	private:
		/**
		 * @brief submit_frameの共通処理．ffmpegパイプにフレームデータを書き込む
		 */
		void submit_frame_impl(std::vector<uint8_t>&& frameData) override;
	};

	class VarjoVSTParallelVideoWriter : public VarjoVSTVideoWriter {

	public:
		VarjoVSTParallelVideoWriter(
			const VideoWriteEncodeOptions vw_encode_opt,
			const size_t row_stride,
			const InputFramedataPaddingOption pad_opt
		);

		~VarjoVSTParallelVideoWriter();

		bool open() override;

		void close() override;

		size_t submitQue_size() const { return this->frameData_submitQue.size(); }

	private:
		/**
		 * @brief submit_frameの共通処理．提出バッファにフレームデータを追加する
		 */
		void submit_frame_impl(std::vector<uint8_t>&& frameData) override;

		/**
		 * @brief 動画書き出しワーカースレッド関数
		 */
		void video_write_worker();

		// for parallel write

		std::deque<std::vector<uint8_t>> frameData_submitQue;				///! buffer for parallel write
		std::mutex submitQue_mutex_;
		std::condition_variable submitQue_cv_;
		std::thread video_write_worker_thread_;
		std::atomic_bool stop_worker_signal_{true};

	};

	struct VarjoVSTVideoWriterOptions {
		VideoWriterType writer_type;
		VideoWriteEncodeOptions vw_encode_opt;
		size_t row_stride;
		InputFramedataPaddingOption pad_opt;
	};

	VarjoVSTVideoWriterOptions make_VideoWriterOption(
		const VideoWriterType writer_type,
		const VideoWriteEncodeOptions vw_encode_opt,
		const size_t row_stride,
		const InputFramedataPaddingOption pad_opt
	);

	std::unique_ptr<VarjoVSTVideoWriter> factory_VideoWriterPtr(const VarjoVSTVideoWriterOptions opt);
}
