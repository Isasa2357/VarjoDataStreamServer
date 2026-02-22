#include "TimestampDataStreamer.hpp"

namespace Timestamp {

	DataStreamer::DataStreamer(const std::shared_ptr<Session>& session, const int separate_ms)
		: session_(session)
		, separate_ms_(separate_ms)
	{}

	DataStreamer::~DataStreamer()
	{
		this->close();
	}

	void DataStreamer::open()
	{
		this->status_ = DataStreamerStatus::Open;

		// スレッドを起動
		this->worker_stop_flag_ = false;
		this->worker_thread_ = std::thread(&DataStreamer::datastream_worker, this);
	}

	void DataStreamer::close()
	{
		// スレッドを停止
		this->worker_stop_flag_ = true;
		if (this->worker_thread_.joinable()) {
			this->worker_thread_.join();
		}

		this->status_ = DataStreamerStatus::Close;
	}

	std::deque<TimestampData> DataStreamer::take_data()
	{
		std::lock_guard<std::mutex> lock(this->data_que_mtx_);
		return std::exchange(this->data_que_, std::deque<TimestampData>{});
	}

	void DataStreamer::datastream_worker()
	{
		while (!this->worker_stop_flag_.load()) {
			auto start = std::chrono::high_resolution_clock::now();

			// TimestampData作成
			TimestampData data;
			data.varjo_timestamp = this->session_->getCurrentTime();
			data.system_timestamp = std::chrono::system_clock::now();
			data.varjo_timestamp_unix = varjo_ConvertToUnixTime(*(this->session_), data.varjo_timestamp);
			{
				std::lock_guard<std::mutex> lock(data_que_mtx_);
				data_que_.push_back(data);
			}

			auto end = std::chrono::high_resolution_clock::now();

			std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

			auto sleep_time = std::chrono::milliseconds(separate_ms_) - elapsed;

			if (sleep_time > std::chrono::milliseconds(0)) {
				std::this_thread::sleep_for(sleep_time);
			}
		}
	}


	std::unique_ptr<DataStreamer> make_DataStreamerPtr(const DataStreamerOptions& opt)
	{
		return std::make_unique<DataStreamer>(opt.session, opt.separate_ms);
	}

} // namespace Timestamp