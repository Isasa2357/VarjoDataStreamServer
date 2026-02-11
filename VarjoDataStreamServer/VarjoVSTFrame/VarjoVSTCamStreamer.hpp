#pragma once

#include <iostream>
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
#include "../util/vec_util.hpp"
#include "../util/struct_json_io.hpp"

#include "json/json.hpp"
#include "json/json_fwd.hpp"

namespace VarjoVSTFrame {

	using Frame = VarjoExamples::DataStreamer::Frame;
	using Framedata = std::vector<uint8_t>;
	using Metadata = VarjoExamples::DataStreamer::Frame::Metadata;



	/****************************************************************************************************
	* @class VarjoVSTCamStreamer
	* ****************************************************************************************************/

	class VarjoVSTCamStreamer {
		
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

	/****************************************************************************************************
	* @class VarjoVSTDummyCamStreamer
	*****************************************************************************************************/

	/**
	 * @brief 手元にVarjoがない場合にのダミーのVSTカメラストリーマークラス
	 * @detail 保存済みのVSTカメラフレームデータを読み込み，VarjoVSTCamStreamerと同じインターフェースで提供する
	 */
	class VarjoVSTDummyCamStreamer {

	public:
		VarjoVSTDummyCamStreamer(varjo_ChannelFlag chnls = varjo_ChannelFlag_All, const size_t buffer_capacity = 20, const int fps = 90);

		std::optional<varjo_StreamConfig> getConfig() const;

		void startStream();

		void stopStream();

		std::pair<std::queue<Framedata>, std::queue<Metadata>> take_lframe_que();

		std::pair<std::queue<Framedata>, std::queue<Metadata>> take_rframe_que();

		inline varjo_ChannelFlag datastream_chnls() const { return this->chnls_; }
		inline size_t left_frame_que_size() const { return this->lframedata_que_.size(); }
		inline size_t right_frame_que_size() const { return this->rframedata_que_.size(); }
		inline size_t buffer_capacity() const { return this->buffer_capacity_; }

	private:
		void onFrameReceivedworker();

		const varjo_ChannelFlag	chnls_;

		const int fps_;
		std::atomic_bool stop_worker_signal_{true};
		std::thread onFrame_thread_;

		std::vector<Framedata> loaded_lframedata_;
		std::vector<Metadata> loaded_lmetadata_;
		std::vector<Framedata> loaded_rframedata_;
		std::vector<Metadata> loaded_rmetadata_;

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

namespace nlohmann {

	template <>
	struct adl_serializer<VarjoVSTFrame::Metadata> {
		static void to_json(json& j, const VarjoVSTFrame::Metadata& metadata) 
		{
			j["streamFrame"] = metadata.streamFrame;
			j["channelIndex"] = metadata.channelIndex;
			j["timestamp"] = metadata.timestamp;
			j["extrinsics"] = metadata.extrinsics;
			j["intrinsics"] = metadata.intrinsics;
			j["bufferMetadata"] = metadata.bufferMetadata;
		}

		static void from_json(const json& j, VarjoVSTFrame::Metadata& metadata) {
			j.at("streamFrame").get_to(metadata.streamFrame);
			j.at("channelIndex").get_to(metadata.channelIndex);
			j.at("timestamp").get_to(metadata.timestamp);
			j.at("extrinsics").get_to(metadata.extrinsics);
			j.at("intrinsics").get_to(metadata.intrinsics);
			j.at("bufferMetadata").get_to(metadata.bufferMetadata);
		}
	};
}