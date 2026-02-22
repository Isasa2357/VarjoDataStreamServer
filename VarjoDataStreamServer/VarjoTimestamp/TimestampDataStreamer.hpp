#pragma once

#include <chrono>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

#include <Varjo.h>
#include <Varjo_types.h>

#include "../VarjoExample/Session.hpp"
#include "Timestamp_types.hpp"

namespace Timestamp {

	enum class DataStreamerStatus {
		Close, Open
	};

	class DataStreamer {

	public:
		DataStreamer(const std::shared_ptr<Session>& session, const int separate_ms);

		~DataStreamer();

		void open();

		void close();

		std::deque<TimestampData> take_data();

		void datastream_worker();

		// getter

		bool is_open() const { return this->status_ == DataStreamerStatus::Open; }
		int data_que_size() { 
			std::lock_guard<std::mutex> lock(this->data_que_mtx_);
			return this->data_que_.size(); 
		}

	private:
		std::shared_ptr<Session> session_;

		const int separate_ms_;

		DataStreamerStatus status_{DataStreamerStatus::Close};

		std::deque<TimestampData> data_que_;
		std::thread worker_thread_;
		std::mutex data_que_mtx_;
		std::atomic_bool worker_stop_flag_{true};
	};

	struct DataStreamerOptions {
		std::shared_ptr<Session> session;
		int separate_ms;
	};

	std::unique_ptr<DataStreamer> make_DataStreamerPtr(const DataStreamerOptions& opt);
}