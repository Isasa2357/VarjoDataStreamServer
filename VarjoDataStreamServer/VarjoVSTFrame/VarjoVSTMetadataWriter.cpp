#pragma once

#include "../util/filesystem_util.hpp"
#include "VarjoVSTMetadataWriter.hpp"

VarjoVSTFrame::VarjoVSTMetadataWriter::VarjoVSTMetadataWriter(const std::string& out_path)
	: out_path_(FilesystemUtil::solve_filename_conflict(out_path))
{
	this->out_stream_ = std::ofstream(this->out_path_, std::ios::out);
	if (!this->out_stream_.is_open()) {
		throw std::runtime_error("Failed to open output metadata file: " + this->out_path_);
	}

	for (auto column_name : this->column_names_) {
		this->out_stream_ << column_name << ",";
	}
	this->out_stream_ << std::endl;
}

VarjoVSTFrame::VarjoVSTMetadataWriter::~VarjoVSTMetadataWriter()
{
	if (this->out_stream_.is_open()) {
		this->out_stream_.close();
	}
}

void VarjoVSTFrame::VarjoVSTMetadataWriter::write_metadata_line(const Metadata& metadata)
{
    const size_t hmdPoseSize = 16;
    const size_t invCCMSize = 9;
    const size_t ccmSize = 9;
    this->out_stream_ << metadata.streamFrame.type << ","
        << metadata.streamFrame.id << ","
        << metadata.streamFrame.frameNumber << ","
        << metadata.streamFrame.channels << ","
        << metadata.streamFrame.dataFlags << ",";

    for (auto i = 0; i < hmdPoseSize; ++i)
        this->out_stream_ << metadata.streamFrame.hmdPose.value[i] << ",";

    this->out_stream_ << metadata.streamFrame.metadata.distortedColor.timestamp << ","
        << metadata.streamFrame.metadata.distortedColor.ev << ","
        << metadata.streamFrame.metadata.distortedColor.exposureTime << ","
        << metadata.streamFrame.metadata.distortedColor.whiteBalanceTemperature << ","
        << metadata.streamFrame.metadata.distortedColor.wbNormalizationData.whiteBalanceColorGains[0] << ","
        << metadata.streamFrame.metadata.distortedColor.wbNormalizationData.whiteBalanceColorGains[1] << ","
        << metadata.streamFrame.metadata.distortedColor.wbNormalizationData.whiteBalanceColorGains[2] << ",";

    for (auto i = 0; i < invCCMSize; ++i)
        this->out_stream_ << metadata.streamFrame.metadata.distortedColor.wbNormalizationData.invCCM.value[i] << ",";

    for (auto i = 0; i < ccmSize; ++i)
        this->out_stream_ << metadata.streamFrame.metadata.distortedColor.wbNormalizationData.ccm.value[i] << ",";

    this->out_stream_ << metadata.streamFrame.metadata.distortedColor.cameraCalibrationConstant << ",";

    // channelIndex
    this->out_stream_ << metadata.channelIndex << ",";

    // timestamp
    this->out_stream_ << metadata.timestamp << ",";

    // extrinsics
    const size_t extrinsicsSize = 16;
    for (auto i = 0; i < extrinsicsSize; ++i)
        this->out_stream_ << metadata.extrinsics.value[i] << ",";

    // intrinsics
    const size_t distortionCoefficientsSize = 8;
    this->out_stream_ << metadata.intrinsics.model << ","
        << metadata.intrinsics.principalPointX << ","
        << metadata.intrinsics.principalPointY << ","
        << metadata.intrinsics.focalLengthX << ","
        << metadata.intrinsics.focalLengthY << ",";

    for (auto i = 0; i < distortionCoefficientsSize; ++i)
        this->out_stream_ << metadata.intrinsics.distortionCoefficients[i] << ",";

    // bufferMetadata
    this->out_stream_ << metadata.bufferMetadata.format << ","
        << metadata.bufferMetadata.type << ","
        << metadata.bufferMetadata.byteSize << ","
        << metadata.bufferMetadata.rowStride << ","
        << metadata.bufferMetadata.width << ","
        << metadata.bufferMetadata.height;

    this->out_stream_ << std::endl;
}

void VarjoVSTFrame::VarjoVSTSerialMetadataWriter::submit_metadata_impl(Metadata&& metadata)
{
	this->write_metadata_line(metadata);
}

/****************************************************************************************************
* VarjoVSTSerialMetadataWriter
* ****************************************************************************************************/

VarjoVSTFrame::VarjoVSTSerialMetadataWriter::VarjoVSTSerialMetadataWriter(const std::string& out_path)
	: VarjoVSTMetadataWriter(out_path)
{}

VarjoVSTFrame::VarjoVSTSerialMetadataWriter::~VarjoVSTSerialMetadataWriter()
{}

/****************************************************************************************************
* VarjoVSTParallelMetadataWriter
* ****************************************************************************************************/

VarjoVSTFrame::VarjoVSTParallelMetadataWriter::VarjoVSTParallelMetadataWriter(const std::string& out_path)
	: VarjoVSTMetadataWriter(out_path)
{
	this->stop_writer_ = false;
	this->writer_thread_ = std::thread(&VarjoVSTParallelMetadataWriter::writer_worker, this);
}

VarjoVSTFrame::VarjoVSTParallelMetadataWriter::~VarjoVSTParallelMetadataWriter()
{}

void VarjoVSTFrame::VarjoVSTParallelMetadataWriter::submit_metadata_impl(Metadata && metadata)
{
    std::unique_lock lk(this->queue_mutex_);
    this->metadata_queue_.push(metadata);
}

void VarjoVSTFrame::VarjoVSTParallelMetadataWriter::writer_worker()
{
    while (!this->stop_writer_) {
        // Metadataの提出があるまで待機
        {
			std::unique_lock lk(this->queue_mutex_);
			this->queue_cv_.wait(lk, [this] () { return !this->metadata_queue_.empty() || this->stop_writer_; });
        }

        // 書き込み
		while (!this->metadata_queue_.empty()) {
            Metadata metadata;
            {
                std::lock_guard lg(this->queue_mutex_);
                metadata = std::move(this->metadata_queue_.front());
                this->metadata_queue_.pop();
            }
            this->write_metadata_line(metadata);
        }
    }
}
