#pragma once

#include <string>
#include <vector>
#include <span>
#include <format>
#include <deque>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "varjo_vst_frame_type.hpp"

namespace VarjoVSTFrame {
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
			const size_t width,
			const size_t height,
			const size_t row_stride,
			const Codec codec,
			const std::string out_path,
			const int crf,
			const int framerate
			
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
		inline Codec codec() const { return this->codec_; }
		inline std::string out_path() const { return this->out_path_; }
		inline int crf() const { return this->crf_; }
		inline int framerate() const { return this->framerate_; }

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

		void remove_padding(const std::vector<uint8_t>& raw_frameData, std::vector<uint8_t>& out_frameData) const;

		// frameSize
		const size_t width_;							///! without padding
		const size_t height_;							///! without padding
		const size_t row_stride_;						///! width with padding

		// write video
		const Codec codec_;								///! video codec
		const std::string out_path_;					///! output video path

		const int crf_;									///! quality
		const int framerate_;							///! frame rate

		FILE* ffmpeg_pipe_;								///! ffmpeg pipe
	};

	class VarjoVSTSerialVideoWriter : public VarjoVSTVideoWriter {

	public:
		VarjoVSTSerialVideoWriter(
			const size_t width,
			const size_t height,
			const size_t row_stride,
			const Codec codec,
			const std::string out_path,
			const int crf,
			const int framerate
		);

		~VarjoVSTSerialVideoWriter();

		bool open() override;

		void close() override;

	private:
		/**
		 * @brief submit_frameの共通処理．ffmpegパイプにフレームデータを書き込む
		 */
		void submit_frame_impl(std::vector<uint8_t>&& frameData) override;

		// for serial write
		std::vector<uint8_t> tight_frameData_;
		///! buffer for serial write
	};

	class VarjoVSTParallelVideoWriter : public VarjoVSTVideoWriter {

	public:
		VarjoVSTParallelVideoWriter(
			const size_t width,
			const size_t height,
			const size_t row_stride,
			const Codec codec,
			const std::string out_path,
			const int crf,
			const int framerate,
			const size_t buffer_size
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

		// for write worker
		std::vector<uint8_t> tight_frameData_;
	};

}