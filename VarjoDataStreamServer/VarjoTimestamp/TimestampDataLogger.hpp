#pragma once

#include <string>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>

#include "../VarjoExample/Session.hpp"

#include "Timestamp_types.hpp"
#include "TimestampDataStreamer.hpp"
#include "TimestampCsvWriter.hpp"

namespace Timestamp {

	class DataLogger {
	public:
		DataLogger(const int check_interval_ms = 1000);

		~DataLogger();

		bool open(const DataStreamerOptions& dstream_opt, const CsvWriterOptions& writer_opt);

		void close();

	private:

		void logging_worker();

	private:

		const int check_interval_ms_;
		std::unique_ptr<DataStreamer> dstreamer_;
		std::unique_ptr<DataCsvWriter> csvwriter_;
		std::thread logging_thread_;
		std::atomic_bool stop_thread_{true};
	};
}