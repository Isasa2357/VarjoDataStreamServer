#pragma once

#include <vector>
#include <deque>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "../VarjoExample/Session.hpp"
#include "../VarjoExample/DataStreamer.hpp"

#include "EyeCam_types.hpp"

namespace EyeCam {
	class EyeCamDataStreamer {
		
	public:
		EyeCamDataStreamer(
			const std::shared_ptr<Session>& session, 
			const varjo_ChannelFlag channels, 
			const size_t buffer_capacity=20);

		~EyeCamDataStreamer();

		std::optional<varjo_StreamConfig> getConfig() const;

		void start_stream();

		void stop_stream();

		std::pair<std::deque<Framedata>, std::deque<Metadata>> take_lframe_que();

		std::pair<std::deque<Framedata>, std::deque<Metadata>> take_rframe_que();

		inline varjo_ChannelFlag datastream_chnls() const { return this->channels_; }

	private:
		void onFrameReceived(const Frame& frame);

	private:
		std::shared_ptr<Session> session_;

		VarjoExamples::DataStreamer dstreamer_;
		const varjo_ChannelFlag channels_;

		const size_t buffer_capacity_;
		std::deque<Framedata> lframedata_que_;
		std::deque<Metadata> lmetadata_que_;
		std::mutex lframe_que_mtx_;
		std::deque<Framedata> rframedata_que_;
		std::deque<Metadata> rmetadata_que_;
		std::mutex rframe_que_mtx_;
	};
}