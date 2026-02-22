
#include "TimestampDataLogger.hpp"

namespace Timestamp {
	
	DataLogger::DataLogger(const int check_interval_ms)
		: check_interval_ms_(check_interval_ms) 
	{}

	DataLogger::~DataLogger() 
	{
		this->close();
	}

	bool DataLogger::open(const DataStreamerOptions& dstream_opt, const CsvWriterOptions& writer_opt)
	{
		this->dstreamer_ = make_DataStreamerPtr(dstream_opt);
		this->csvwriter_ = make_DataCsvWrierPtr(writer_opt);

		if (!this->dstreamer_->is_open()) {
			printf("DataLogger: opening dstreamer...\n");
			this->dstreamer_->open();
		}

		if (!this->csvwriter_->is_open()) {
			this->csvwriter_->open();
		}

		printf("DataLogger: dstreamer open=%d, csvwriter open=%d\n", this->dstreamer_->is_open(), this->csvwriter_->is_open());

		if (this->dstreamer_->is_open() && this->csvwriter_->is_open()) {
			this->stop_thread_ = false;
			this->logging_thread_ = std::thread(&DataLogger::logging_worker, this);
			return true;
		}
		return false;
	}

	void DataLogger::close()
	{
		this->stop_thread_.store(true);

		if (this->dstreamer_ && this->dstreamer_->is_open()) {
			this->dstreamer_->close();
		}
		if (this->csvwriter_ && this->csvwriter_->is_open()) {
			this->csvwriter_->close();
		}

		if (this->logging_thread_.joinable()) {
			this->logging_thread_.join();
		}
	}

	void DataLogger::logging_worker()
	{
		while (!this->stop_thread_) {
			std::deque<TimestampData> data_que = this->dstreamer_->take_data();
			printf("DataLogger: got data_que with size=%d\n", data_que.size());

			if (this->csvwriter_ && this->csvwriter_->is_open()) {
				printf("DataLogger: submit data_que to csvwriter...\n");
				this->csvwriter_->submit_TimestampData(std::move(data_que));
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(this->check_interval_ms_));
		}
	}

} // namespace Timestamp