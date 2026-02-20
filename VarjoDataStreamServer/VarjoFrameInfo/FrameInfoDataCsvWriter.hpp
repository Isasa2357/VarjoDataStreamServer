#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <queue>
#include <filesystem>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "../util/filesystem_util.hpp"

#include "FrameInfo_types.hpp"
#include "ISubmitFrameInfo.hpp"

namespace VarjoFrameInfo {

	enum class DataCsvWriterType {
		Serial,
		Parallel
	};
	
	class DataCsvWriter : public ISubmitFrameInfo {

	protected:
		inline static std::vector<std::string> csv_header = {
			// views[0]
			"view0_projectionMatrix[0]", "view0_projectionMatrix[1]", "view0_projectionMatrix[2]", "view0_projectionMatrix[3]",
			"view0_projectionMatrix[4]", "view0_projectionMatrix[5]", "view0_projectionMatrix[6]", "view0_projectionMatrix[7]",
			"view0_projectionMatrix[8]", "view0_projectionMatrix[9]", "view0_projectionMatrix[10]", "view0_projectionMatrix[11]",
			"view0_projectionMatrix[12]", "view0_projectionMatrix[13]", "view0_projectionMatrix[14]", "view0_projectionMatrix[15]",
			"view0_viewMatrix[0]", "view0_viewMatrix[1]", "view0_viewMatrix[2]", "view0_viewMatrix[3]",
			"view0_viewMatrix[4]", "view0_viewMatrix[5]", "view0_viewMatrix[6]", "view0_viewMatrix[7]",
			"view0_viewMatrix[8]", "view0_viewMatrix[9]", "view0_viewMatrix[10]", "view0_viewMatrix[11]",
			"view0_viewMatrix[12]", "view0_viewMatrix[13]", "view0_viewMatrix[14]", "view0_viewMatrix[15]",
			"view9_preferrdWidth", "view0_preferredHeight", "view0_enabled", 
			// views[1]
			"view1_projectionMatrix[0]", "view1_projectionMatrix[1]", "view1_projectionMatrix[2]", "view1_projectionMatrix[3]",
			"view1_projectionMatrix[4]", "view1_projectionMatrix[5]", "view1_projectionMatrix[6]", "view1_projectionMatrix[7]",
			"view1_projectionMatrix[8]", "view1_projectionMatrix[9]", "view1_projectionMatrix[10]", "view1_projectionMatrix[11]",
			"view1_projectionMatrix[12]", "view1_projectionMatrix[13]", "view1_projectionMatrix[14]", "view1_projectionMatrix[15]",
			"view1_viewMatrix[0]", "view1_viewMatrix[1]", "view1_viewMatrix[2]", "view1_viewMatrix[3]",
			"view1_viewMatrix[4]", "view1_viewMatrix[5]", "view1_viewMatrix[6]", "view1_viewMatrix[7]",
			"view1_viewMatrix[8]", "view1_viewMatrix[9]", "view1_viewMatrix[10]", "view1_viewMatrix[11]",
			"view1_viewMatrix[12]", "view1_viewMatrix[13]", "view1_viewMatrix[14]", "view1_viewMatrix[15]",
			"view1_preferrdWidth", "view1_preferredHeight", "view1_enabled",
			// views[2]
			"view2_projectionMatrix[0]", "view2_projectionMatrix[1]", "view2_projectionMatrix[2]", "view2_projectionMatrix[3]",
			"view2_projectionMatrix[4]", "view2_projectionMatrix[5]", "view2_projectionMatrix[6]", "view2_projectionMatrix[7]",
			"view2_projectionMatrix[8]", "view2_projectionMatrix[9]", "view2_projectionMatrix[10]", "view2_projectionMatrix[11]",
			"view2_projectionMatrix[12]", "view2_projectionMatrix[13]", "view2_projectionMatrix[14]", "view2_projectionMatrix[15]",
			"view2_viewMatrix[0]", "view2_viewMatrix[1]", "view2_viewMatrix[2]", "view2_viewMatrix[3]",
			"view2_viewMatrix[4]", "view2_viewMatrix[5]", "view2_viewMatrix[6]", "view2_viewMatrix[7]",
			"view2_viewMatrix[8]", "view2_viewMatrix[9]", "view2_viewMatrix[10]", "view2_viewMatrix[11]",
			"view2_viewMatrix[12]", "view2_viewMatrix[13]", "view2_viewMatrix[14]", "view2_viewMatrix[15]",
			"view2_preferrdWidth", "view2_preferredHeight", "view2_enabled",
			// views[3]
			"view3_projectionMatrix[0]", "view3_projectionMatrix[1]", "view3_projectionMatrix[2]", "view3_projectionMatrix[3]",
			"view3_projectionMatrix[4]", "view3_projectionMatrix[5]", "view3_projectionMatrix[6]", "view3_projectionMatrix[7]",
			"view3_projectionMatrix[8]", "view3_projectionMatrix[9]", "view3_projectionMatrix[10]", "view3_projectionMatrix[11]",
			"view3_projectionMatrix[12]", "view3_projectionMatrix[13]", "view3_projectionMatrix[14]", "view3_projectionMatrix[15]",
			"view3_viewMatrix[0]", "view3_viewMatrix[1]", "view3_viewMatrix[2]", "view3_viewMatrix[3]",
			"view3_viewMatrix[4]", "view3_viewMatrix[5]", "view3_viewMatrix[6]", "view3_viewMatrix[7]",
			"view3_viewMatrix[8]", "view3_viewMatrix[9]", "view3_viewMatrix[10]", "view3_viewMatrix[11]",
			"view3_viewMatrix[12]", "view3_viewMatrix[13]", "view3_viewMatrix[14]", "view3_viewMatrix[15]",
			"view3_preferrdWidth", "view3_preferredHeight", "view3_enabled",
			// fovTangents[0]
			"fovTangents0_top", "fovTangents0_bottom", "fovTangents0_left", "fovTangents0_right",
			// fovTangents[1]
			"fovTangents1_top", "fovTangents1_bottom", "fovTangents1_left", "fovTangents1_right",
			// fovTangents[2]
			"fovTangents2_top", "fovTangents2_bottom", "fovTangents2_left", "fovTangents2_right",
			// fovTangents[3]
			"fovTangents3_top", "fovTangents3_bottom", "fovTangents3_left", "fovTangents3_right",
			// timestamp and frameNumber
			"timestamp", "frameNumber"
		};

	public:
		DataCsvWriter(const std::string& path);

		~DataCsvWriter();

		virtual bool open();
		virtual void close();

	protected:

		void write_header();

		void write_line(const FrameInfoData& data);

	public:

		// gettter
		inline std::filesystem::path get_csv_path() const {
			return csv_path_;
		}

		inline bool is_open() const {
			return csv_file_.is_open();
		}

	protected:
		std::filesystem::path csv_path_;
		std::fstream csv_file_;
	};

	class SerialDataCsvWriter : public DataCsvWriter {
	public:
		SerialDataCsvWriter(const std::string& path);

		void submit_FrameInfoData(const FrameInfoData& data) override;
		void submit_FrameInfoData(FrameInfoData&& data) override;
		void submit_FrameInfoData(const std::vector<FrameInfoData>& data) override;
		void submit_FrameInfoData(std::vector<FrameInfoData>&& data) override;
		void submit_FrameInfoData(std::queue<FrameInfoData>& data) override;
		void submit_FrameInfoData(std::queue<FrameInfoData>&& data) override;

	private:
		void submit_FrameInfoData_impl(BorrowedOrOwned<FrameInfoData> data) override;
	};

	class ParallelDataCsvWriter : public DataCsvWriter {

	public:
		ParallelDataCsvWriter(const std::string& path);
		~ParallelDataCsvWriter();

		bool open() override;
		void close() override;

		void submit_FrameInfoData(const FrameInfoData& data) override;
		void submit_FrameInfoData(FrameInfoData&& data) override;
		void submit_FrameInfoData(const std::vector<FrameInfoData>& data) override;
		void submit_FrameInfoData(std::vector<FrameInfoData>&& data) override;
		void submit_FrameInfoData(std::queue<FrameInfoData>& data) override;
		void submit_FrameInfoData(std::queue<FrameInfoData>&& data) override;

	private:
		void submit_FrameInfoData_impl(BorrowedOrOwned<FrameInfoData> data) override;

		void writer_worker();

	private:
		std::deque<FrameInfoData> data_que_;
		std::mutex data_que_mtx_;
		std::condition_variable data_que_cv_;
		std::atomic_bool stop_thread_ = false;
		std::thread worker_thread_;
	};

	struct DataCsvWriterOptions {
		DataCsvWriterType writer_type;
		std::string out_path;
	};

	DataCsvWriterOptions make_DataCsvWriterOptions(
		const DataCsvWriterType& writer_type,
		const std::string& out_path
	);

	std::unique_ptr<DataCsvWriter> make_DataCsvWriterPtr(const DataCsvWriterOptions& opt);
	std::unique_ptr<ISubmitFrameInfo> make_FrameInfoData_asISubmit(const DataCsvWriterOptions& opt);
}