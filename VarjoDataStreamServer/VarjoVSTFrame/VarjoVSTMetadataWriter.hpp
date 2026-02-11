
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "ISubmitFrame.hpp"


namespace VarjoVSTFrame {

	enum class VarjoVSTMetadataWriterType {
		Serial,
		Parallel
	};

	class VarjoVSTMetadataWriter : public ISubmitMetadata {
		const std::vector<std::string> column_names_{
			"streamFrame.type"
			"streamframe.id"
			"streamFrame.frameNumber"
			"streamFrame.channels"
			"streamFrame.dataFlags"
			"streamFrame.hmdPose.value[0]"
			"streamFrame.hmdPose.value[1]"
			"streamFrame.hmdPose.value[2]"
			"streamFrame.hmdPose.value[3]"
			"streamFrame.hmdPose.value[4]"
			"streamFrame.hmdPose.value[5]"
			"streamFrame.hmdPose.value[6]"
			"streamFrame.hmdPose.value[7]"
			"streamFrame.hmdPose.value[8]"
			"streamFrame.hmdPose.value[9]"
			"streamFrame.hmdPose.value[10]"
			"streamFrame.hmdPose.value[11]"
			"streamFrame.hmdPose.value[12]"
			"streamFrame.hmdPose.value[13]"
			"streamFrame.hmdPose.value[14]"
			"streamFrame.hmdPose.value[15]"
			"streamFrame.metadata.distortedColor.timestamp"
			"streamFrame.metadata.distortedColor.ev"
			"streamFrame.metadata.distortedColor.exposuretime"
			"streamFrame.metadata.distortedColor.whiteBalanceTemperature"
			"streamFrame.metadata.distortedColor.wbNormalizationData.whiteBalanceColorGains[0]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.whiteBalanceColorGains[1]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.whiteBalanceColorGains[2]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.invCCM.value[0]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.invCCM.value[1]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.invCCM.value[2]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.invCCM.value[3]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.invCCM.value[4]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.invCCM.value[5]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.invCCM.value[6]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.invCCM.value[7]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.invCCM.value[8]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.ccm.value[0]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.ccm.value[1]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.ccm.value[2]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.ccm.value[3]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.ccm.value[4]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.ccm.value[5]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.ccm.value[6]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.ccm.value[7]"
			"streamFrame.metadata.distortedColor.wbNormalizationData.ccm.value[8]"
			"streamFrame.metadata.distortedColor.cameraCalibrationConstant"
			"channelIndex"
			"timestamp"
			"extrinsics.value[0]"
			"extrinsics.value[1]"
			"extrinsics.value[2]"
			"extrinsics.value[3]"
			"extrinsics.value[4]"
			"extrinsics.value[5]"
			"extrinsics.value[6]"
			"extrinsics.value[7]"
			"extrinsics.value[8]"
			"extrinsics.value[9]"
			"extrinsics.value[10]"
			"extrinsics.value[11]"
			"extrinsics.value[12]"
			"extrinsics.value[13]"
			"extrinsics.value[14]"
			"extrinsics.value[15]"
			"intrinsics.model"
			"intrinsics.principalPointX"
			"intrinsics.principalPointY"
			"intrinsics.focalLengthX"
			"intrinsics.focalLengthY"
			"intrinsics.distortionCoefficients[0]"
			"intrinsics.distortionCoefficients[1]"
			"intrinsics.distortionCoefficients[2]"
			"intrinsics.distortionCoefficients[3]"
			"intrinsics.distortionCoefficients[4]"
			"intrinsics.distortionCoefficients[5]"
			"intrinsics.distortionCoefficients[6]"
			"intrinsics.distortionCoefficients[7]"
			"bufferMetadata.format"
			"bufferMetadata.type"
			"bufferMetadata.byteSize"
			"bufferMetadata.rowStride"
			"bufferMetadata.width"
			"bufferMetadata.height"
		};

	public:
		VarjoVSTMetadataWriter(const std::string& out_path);

		~VarjoVSTMetadataWriter();

	protected:
		void write_metadata_line(const Metadata& metadata);


	protected:
		const std::string out_path_;
		std::ofstream out_stream_;
	};

	/****************************************************************************************************
	* VarjoVSTSerialMetadataWriter
	* ****************************************************************************************************/

	class VarjoVSTSerialMetadataWriter : public VarjoVSTMetadataWriter {

	public:
		VarjoVSTSerialMetadataWriter(const std::string& out_path);
	
		~VarjoVSTSerialMetadataWriter();

	private:
		void submit_metadata_impl(Metadata&& metadata) override;

	};

	/****************************************************************************************************
	* VarjoVSTParallelMetadataWriter
	* ****************************************************************************************************/

	class VarjoVSTParallelMetadataWriter : public VarjoVSTMetadataWriter {
	public:
		VarjoVSTParallelMetadataWriter(const std::string& out_path);

		~VarjoVSTParallelMetadataWriter();

	protected:
		void submit_metadata_impl(Metadata&& metadata) override;

		void writer_worker();

	private:
		std::queue<Metadata> metadata_queue_;
		std::mutex queue_mutex_;
		std::condition_variable queue_cv_;
		std::thread writer_thread_;
		bool stop_writer_{true};
	};
}