
#include <iomanip>

#include "../util/filesystem_util.hpp"
#include "TimestampCsvWriter.hpp"

namespace {
	std::string format_system_clock_local_yyymmdd_hhmmss_mmm(std::chrono::system_clock::time_point tp) {
		const auto ms_since_epoch = duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
		const auto sec_since_epoch = duration_cast<std::chrono::seconds>(ms_since_epoch);
		const auto msec = static_cast<int>((ms_since_epoch - sec_since_epoch).count());

		std::time_t tt = std::chrono::system_clock::to_time_t(tp);

		std::tm local_tm{};
		localtime_s(&local_tm, &tt);

		std::ostringstream oss;
		oss << std::put_time(&local_tm, "%Y-%m-%d_%H:%M:%S") << "." << std::setw(3) << std::setfill('0') << msec;
		return oss.str();
	}

	std::string format_system_clock_utc_yyymmdd_hhmmss_mmm(std::chrono::system_clock::time_point tp) {
		const auto ms_since_epoch = duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
		const auto sec_since_epoch = duration_cast<std::chrono::seconds>(ms_since_epoch);
		const auto msec = static_cast<int>((ms_since_epoch - sec_since_epoch).count());

		std::time_t tt = std::chrono::system_clock::to_time_t(tp);
		std::tm utc_tm{};

		gmtime_s(&utc_tm, &tt);

		std::ostringstream oss;
		oss << std::put_time(&utc_tm, "%Y-%m-%d_%H:%M:%S") << "." << std::setw(3) << std::setfill('0') << msec;
		return oss.str();
	}
}

namespace Timestamp {

	DataCsvWriter::DataCsvWriter(const std::string& path)
		: path_(solve_filename_conflict(path))
	{}

	DataCsvWriter::~DataCsvWriter()
	{
		this->close();
	}

	bool DataCsvWriter::open()
	{
		this->csv_file_.open(this->path_, std::ios::out);
		if (!this->csv_file_.is_open()) {
			return false;
		}
		this->write_header();

		return true;
	}

	void DataCsvWriter::close()
	{
		if (this->csv_file_.is_open()) {
			this->csv_file_.flush();
			this->csv_file_.close();
		}
	}

	void DataCsvWriter::write_header()
	{
		if (this->csv_file_.is_open()) {
			for (size_t i = 0; i < header_.size(); ++i) {
				this->csv_file_ << header_[i];
				if (i != header_.size() - 1) {
					this->csv_file_ << ",";
				}
			}
			this->csv_file_ << "\n";
		}
	}

	void DataCsvWriter::write_line(const TimestampData& data)
	{
		if (this->csv_file_.is_open()) {
			this->csv_file_ << data.varjo_timestamp << ",";
			this->csv_file_ << data.varjo_timestamp_unix << ",";
			this->csv_file_ << format_system_clock_utc_yyymmdd_hhmmss_mmm(data.system_timestamp) << ",";
			this->csv_file_ << format_system_clock_local_yyymmdd_hhmmss_mmm(data.system_timestamp) << "\n";
		}
	}

	SerialDataCsvWriter::SerialDataCsvWriter(const std::string& path)
		: DataCsvWriter(path)
	{}

	void SerialDataCsvWriter::submit_TimestampData(const TimestampData & data)
	{
		this->submit_TimestampData_impl(data);
	}

	void SerialDataCsvWriter::submit_TimestampData(TimestampData&& data)
	{
		this->submit_TimestampData_impl(std::move(data));
	}

	void SerialDataCsvWriter::submit_TimestampData(const std::vector<TimestampData>&data)
	{
		for (auto& d : data) {
			this->submit_TimestampData_impl(d);
		}
	}

	void SerialDataCsvWriter::submit_TimestampData(std::vector<TimestampData>&& data)
	{
		for (auto& d : data) {
			this->submit_TimestampData_impl(std::move(d));
		}
	}

	void SerialDataCsvWriter::submit_TimestampData(std::deque<TimestampData>& data_que)
	{
		for (auto& d : data_que) {
			this->submit_TimestampData_impl(d);
		}
	}

	void SerialDataCsvWriter::submit_TimestampData(std::deque<TimestampData>&& data_que)
	{
		for (auto& d : data_que) {
			this->submit_TimestampData_impl(std::move(d));
		}
	}

	void SerialDataCsvWriter::submit_TimestampData_impl(BorrowedOrOwned<TimestampData> data)
	{
		this->write_line(data.view());
	}

	ParallelDataCsvWriter::ParallelDataCsvWriter(const std::string& path)
		: DataCsvWriter(path)
	{}

	ParallelDataCsvWriter::~ParallelDataCsvWriter()
	{
		this->close();
	}

	bool ParallelDataCsvWriter::open()
	{
		// csvファイルを開く
		if (!DataCsvWriter::open()) return false;

		// スレッドを起動
		this->stop_thread_.store(false);
		this->worker_thread_ = std::thread(&ParallelDataCsvWriter::writer_worker, this);

		return true;
	}

	void ParallelDataCsvWriter::close()
	{
		// スレッドを停止
		this->stop_thread_.store(true);
		this->data_que_cv_.notify_all();
		if (this->worker_thread_.joinable()) {
			this->worker_thread_.join();
		}

		// csvファイルを閉じる
		DataCsvWriter::close();
	}

	void ParallelDataCsvWriter::submit_TimestampData(const TimestampData& data)
	{
		std::lock_guard<std::mutex> lock(this->data_que_mtx_);
		this->submit_TimestampData_impl(data);
		this->data_que_cv_.notify_all();
	}

	void ParallelDataCsvWriter::submit_TimestampData(TimestampData&& data)
	{
		std::lock_guard<std::mutex> lock(this->data_que_mtx_);
		this->submit_TimestampData_impl(std::move(data));
		this->data_que_cv_.notify_all();
	}

	void ParallelDataCsvWriter::submit_TimestampData(const std::vector<TimestampData>& data)
	{
		std::lock_guard<std::mutex> lock(this->data_que_mtx_);
		for (auto& d : data) {
			this->submit_TimestampData_impl(d);
		}
		this->data_que_cv_.notify_all();
	}

	void ParallelDataCsvWriter::submit_TimestampData(std::vector<TimestampData>&& data)
	{
		std::lock_guard<std::mutex> lock(this->data_que_mtx_);
		for (auto& d : data) {
			this->submit_TimestampData_impl(std::move(d));
		}
		this->data_que_cv_.notify_all();
	}

	void ParallelDataCsvWriter::submit_TimestampData(std::deque<TimestampData>& data_que)
	{
		std::lock_guard<std::mutex> lock(this->data_que_mtx_);
		for (auto& d : data_que) {
			this->submit_TimestampData_impl(d);
		}
		this->data_que_cv_.notify_all();
	}

	void ParallelDataCsvWriter::submit_TimestampData(std::deque<TimestampData>&& data_que)
	{
		std::lock_guard<std::mutex> lock(this->data_que_mtx_);
		for (auto& d : data_que) {
			this->submit_TimestampData_impl(std::move(d));
		}
		this->data_que_cv_.notify_all();
	}

	void ParallelDataCsvWriter::submit_TimestampData_impl(BorrowedOrOwned<TimestampData> data)
	{
		this->data_que_.push_back(std::move(data).materialize());
	}

	void ParallelDataCsvWriter::writer_worker()
	{
		while (!this->stop_thread_.load()) {
			std::deque<TimestampData> data_que_copy;
			{
				// 提出通知まで待機
				std::unique_lock<std::mutex> lock(this->data_que_mtx_);
				this->data_que_cv_.wait(lock, [this] { return !this->data_que_.empty() || this->stop_thread_.load(); });
				if (this->stop_thread_.load()) {
					break;
				}

				// データキューを退避
				data_que_copy = std::exchange(this->data_que_, std::deque<TimestampData>{});
			}

			// 書き込み
			for (auto& data : data_que_copy) {
				this->write_line(data);
			}
		}
	}

	std::unique_ptr<DataCsvWriter> make_DataCsvWrierPtr(const CsvWriterOptions& opt)
	{
		if (opt.type == CsvWriterType::Serial) {
			return std::make_unique<SerialDataCsvWriter>(opt.path);
		} else if (opt.type == CsvWriterType::Parallel) {
			return std::make_unique<ParallelDataCsvWriter>(opt.path);
		} else {
			throw std::invalid_argument("Invalid CsvWriterType");
		}
	}

	std::unique_ptr<ISubmitTimestamp> make_DataCsvWrierPtr_asISubmit(const CsvWriterOptions& opt)
	{
		if (opt.type == CsvWriterType::Serial) {
			return std::make_unique<SerialDataCsvWriter>(opt.path);
		} else if (opt.type == CsvWriterType::Parallel) {
			return std::make_unique<ParallelDataCsvWriter>(opt.path);
		} else {
			throw std::invalid_argument("Invalid CsvWriterType");
		}
	}

} // namespace Timestamp