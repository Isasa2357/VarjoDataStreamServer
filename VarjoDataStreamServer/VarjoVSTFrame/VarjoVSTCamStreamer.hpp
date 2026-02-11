#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <queue>
#include <functional>
#include <utility>
#include <mutex>

#include "../DataStreamer.hpp"
#include "../Session.hpp"
#include "varjo_vst_frame_type.hpp"


namespace VarjoVSTFrame {
	class VarjoVSTCamStreamer {
	public:
		using Frame = VarjoExamples::DataStreamer::Frame;
		using Framedata = std::vector<uint8_t>;
		using Metadata = VarjoExamples::DataStreamer::Frame::Metadata;
		
	public:
		VarjoVSTCamStreamer(const std::shared_ptr<Session>& session, varjo_ChannelFlag chnls, const size_t buffer_capacity=20);
	
		/**
		 * @brief Datastreamの詳細を取得する．
		 * @detail ちょっと何言っているのかわからない．
		 */
		std::optional<varjo_StreamConfig> getConfig() const;

		/**
		 * @brief Datastreamを開始する
		 */
		void startStream();

		/**
		 * @brief Datastreamを閉じる
		 */
		void stopStream();

		/**
		 * @brief left VST frame queを取得する
		 * @detail moveにより，left VST frame queueを渡し，元のqueueは初期化する
		 */
		std::pair<std::queue<Framedata>, std::queue<Metadata>> take_lframe_que();

		/**
		 * @brief right VST frame queを取得する
		 * @detail moveにより，right VST frame queueを渡し，元のqueueは初期化する
		 */
		std::pair<std::queue<Framedata>, std::queue<Metadata>> take_rframe_que();

		inline varjo_ChannelFlag datastream_chnls() const { return this->chnls_; }
		inline size_t left_frame_que_size() const { return this->lframedata_que_.size(); }
		inline size_t right_frame_que_size() const { return this->rframedata_que_.size(); }
		inline size_t buffer_capacity() const { return this->buffer_capacity_; }

	private:

		/**
		 * @brief Datastream apiのコールバック
		 * @detail 
		 *   Varjo公式のサンプルのDataStreamerクラスに登録するコールバック関数．
		 *   左右両方のVSTフレームを受信し，それぞれのframe queueに入れる
		 * 
		 * @params frame 受信フレーム
		 */
		void onFrameReceived(const Frame& frame);

	private:

		// for Session
		const std::shared_ptr<Session> session_;

		// for datastream api
		VarjoExamples::DataStreamer dstreamer_;
		const varjo_ChannelFlag	chnls_;

		// for frame que
		const size_t buffer_capacity_;
		std::queue<Framedata> lframedata_que_;
		std::queue<Metadata> lmetadata_que_;
		std::mutex lframe_que_mtx_;
		std::queue<Framedata> rframedata_que_;
		std::queue<Metadata> rmetadata_que_;
		std::mutex rframe_que_mtx_;
	};
}

