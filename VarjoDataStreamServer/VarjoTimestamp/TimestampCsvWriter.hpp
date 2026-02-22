#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

#include "Timestamp_types.hpp"
#include "ISubmitTimestamp.hpp"

namespace Timestamp {

	enum class CsvWriterType {
		Serial, Parallel
	};
	
	class DataCsvWriter : public ISubmitTimestamp {

		inline static std::vector<std::string> header_ = {
			"varjo_timestamp",
			"varjo_timestamp_unix",
			"system_timestamp_utc",
			"sytem_timestamp_local"
		};

	public:
		DataCsvWriter(const std::string& path);

		~DataCsvWriter();

		virtual bool open();
		virtual void close();
		
		bool is_open() const {
			return csv_file_.is_open();
		}

	protected:
		void write_header();
		void write_line(const TimestampData& data);

	protected:
		const std::string path_;
		std::fstream csv_file_;
	};

	class SerialDataCsvWriter : public DataCsvWriter {

	public:
		SerialDataCsvWriter(const std::string& path);

		void submit_TimestampData(const TimestampData& data) override;
		void submit_TimestampData(TimestampData&& data) override;

		void submit_TimestampData(const std::vector<TimestampData>& data) override;
		void submit_TimestampData(std::vector<TimestampData>&& data) override;

		void submit_TimestampData(std::deque<TimestampData>& data_que) override;
		void submit_TimestampData(std::deque<TimestampData>&& data_que) override;

	private:
		void submit_TimestampData_impl(BorrowedOrOwned<TimestampData> data) override;
	};

	class ParallelDataCsvWriter : public DataCsvWriter {

	public:
		ParallelDataCsvWriter(const std::string& path);
		~ParallelDataCsvWriter();

		bool open() override;
		void close() override;

		void submit_TimestampData(const TimestampData& data) override;
		void submit_TimestampData(TimestampData&& data) override;

		void submit_TimestampData(const std::vector<TimestampData>& data) override;
		void submit_TimestampData(std::vector<TimestampData>&& data) override;

		void submit_TimestampData(std::deque<TimestampData>& data_que) override;
		void submit_TimestampData(std::deque<TimestampData>&& data_que) override;

	private:
		void submit_TimestampData_impl(BorrowedOrOwned<TimestampData> data) override;

		void writer_worker();

	private:
		std::deque<TimestampData> data_que_;
		std::mutex data_que_mtx_;
		std::condition_variable data_que_cv_;
		std::thread worker_thread_;
		std::atomic_bool stop_thread_{true};
	};

	struct CsvWriterOptions {
		CsvWriterType type;
		std::string path;
	};

	std::unique_ptr<DataCsvWriter> make_DataCsvWrierPtr(const CsvWriterOptions& opt);
	std::unique_ptr<ISubmitTimestamp> make_DataCsvWrierPtr_asISubmit(const CsvWriterOptions& opt);
}