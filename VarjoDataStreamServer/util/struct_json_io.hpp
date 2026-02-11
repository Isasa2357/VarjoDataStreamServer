#pragma once

#include "json/json.hpp"
#include "json/json_fwd.hpp"

#include "../DataStreamer.hpp"
#include "Varjo_types.h"

namespace {
	template <typename T>
	nlohmann::json array_to_jsonArray(const T& arr, size_t size) {
		auto json_arr = nlohmann::json::array();
		for (size_t i = 0; i < size; ++i) {
			json_arr.push_back(arr[i]);
		}
		return json_arr;
	}
}

namespace nlohmann {

	/****************************************************************************************************
	* varjo_types.h 用の to_json/from_json 定義
	****************************************************************************************************/

	template <>
	struct adl_serializer<varjo_Matrix> {
		// 構造体 -> json
		static void to_json(json& j, const varjo_Matrix& m)
		{
			auto arr = json::array();
			for (int i = 0; i < 16; ++i) arr.push_back(m.value[i]);
			j["value"] = std::move(arr);
		}

		// json -> 構造体
		static void from_json(const json& j, varjo_Matrix& m)
		{
			// valueの復元
			j.at("value").get_to(m.value);
		}
	};

	template<>
	struct adl_serializer<varjo_Matrix3x3> {
		static void to_json(json& j, const varjo_Matrix3x3& m)
		{
			auto arr = json::array();
			for (int i = 0; i < 9; ++i) arr.push_back(m.value[i]);
			j["value"] = std::move(arr);
		}
		static void from_json(const json& j, varjo_Matrix3x3& m)
		{
			// valueの復元
			j.at("value").get_to(m.value);
		}
	};

	template<>
	struct adl_serializer<varjo_CameraIntrinsics2> {
		static void to_json(json& j, const varjo_CameraIntrinsics2& intrinsics)
		{
			j["model"] = intrinsics.model;
			j["principalPointX"] = intrinsics.principalPointX;
			j["principalPointY"] = intrinsics.principalPointY;
			j["focalLengthX"] = intrinsics.focalLengthX;
			j["focalLengthY"] = intrinsics.focalLengthY;
			j["distortionCoefficients"] = array_to_jsonArray(intrinsics.distortionCoefficients, 8);
		}

		static void from_json(const json& j, varjo_CameraIntrinsics2& intrinsics)
		{
			j.at("model").get_to(intrinsics.model);
			j.at("principalPointX").get_to(intrinsics.principalPointX);
			j.at("principalPointY").get_to(intrinsics.principalPointY);
			j.at("focalLengthX").get_to(intrinsics.focalLengthX);
			j.at("focalLengthY").get_to(intrinsics.focalLengthY);
			j.at("distortionCoefficients").get_to(intrinsics.distortionCoefficients);
		};
	};

	template<>
	struct adl_serializer<varjo_BufferMetadata> {
		static void to_json(json& j, const varjo_BufferMetadata& bufferMetadata)
		{
			j["format"] = bufferMetadata.format;
			j["type"] = bufferMetadata.type;
			j["byteSize"] = bufferMetadata.byteSize;
			j["rowStride"] = bufferMetadata.rowStride;
			j["width"] = bufferMetadata.width;
			j["height"] = bufferMetadata.height;
		}

		static void from_json(const json& j, varjo_BufferMetadata& bufferMetadata)
		{
			j.at("format").get_to(bufferMetadata.format);
			j.at("type").get_to(bufferMetadata.type);
			j.at("byteSize").get_to(bufferMetadata.byteSize);
			j.at("rowStride").get_to(bufferMetadata.rowStride);
			j.at("width").get_to(bufferMetadata.width);
			j.at("height").get_to(bufferMetadata.height);
		}
	};

	/****************************************************************************************************
	* varjo_types_datastream.h 用の to_json/from_json 定義
	****************************************************************************************************/

	template<>
	struct adl_serializer<varjo_WBNormalizationData> {
		static void to_json(json& j, const varjo_WBNormalizationData& wbNormalizationData)
		{
			j["whiteBalanceColorGains"] = json::array({
				wbNormalizationData.whiteBalanceColorGains[0],
				wbNormalizationData.whiteBalanceColorGains[1],
				wbNormalizationData.whiteBalanceColorGains[2]
				});
			j["invCCM"] = wbNormalizationData.invCCM;
			j["ccm"] = wbNormalizationData.ccm;
		}

		static void from_json(const json& j, varjo_WBNormalizationData& wbNormalizationData)
		{
			// whiteBalanceColorGainsの復元
			j.at("whiteBalanceColorGains").get_to(wbNormalizationData.whiteBalanceColorGains);

			// invCCMの復元
			j.at("invCCM").get_to(wbNormalizationData.invCCM);

			// ccmの復元
			j.at("ccm").get_to(wbNormalizationData.ccm);
		}
	};

	template<>
	struct adl_serializer<varjo_DistortedColorFrameMetadata> {
		static void to_json(json& j, const varjo_DistortedColorFrameMetadata& metadata)
		{
			j["timestamp"] = metadata.timestamp;
			j["ev"] = metadata.ev;
			j["exposureTime"] = metadata.exposureTime;
			j["whiteBalanceTemperature"] = metadata.whiteBalanceTemperature;
			j["wbNormalizationData"] = metadata.wbNormalizationData;
			j["cameraCalibrationConstant"] = metadata.cameraCalibrationConstant;
		}

		static void from_json(const json& j, varjo_DistortedColorFrameMetadata& metadata) {
			j.at("timestamp").get_to(metadata.timestamp);
			j.at("ev").get_to(metadata.ev);
			j.at("exposureTime").get_to(metadata.exposureTime);
			j.at("whiteBalanceTemperature").get_to(metadata.whiteBalanceTemperature);
			j.at("wbNormalizationData").get_to(metadata.wbNormalizationData);
			j.at("cameraCalibrationConstant").get_to(metadata.cameraCalibrationConstant);
		}
	};

	template<>
	struct adl_serializer<varjo_EnvironmentCubemapFrameMetadata> {
		static void to_json(json& j, const varjo_EnvironmentCubemapFrameMetadata& metadata)
		{
			j["timestamp"] = metadata.timestamp;
			j["mode"] = metadata.mode;
			j["whiteBalanceTemperature"] = metadata.whiteBalanceTemperature;
			j["brightnessNormalizationGain"] = metadata.brightnessNormalizationGain;
			j["wbNormalizationData"] = metadata.wbNormalizationData;
		}
		static void from_json(const json& j, varjo_EnvironmentCubemapFrameMetadata& metadata)
		{
			j.at("timestamp").get_to(metadata.timestamp);
			j.at("mode").get_to(metadata.mode);
			j.at("whiteBalanceTemperature").get_to(metadata.whiteBalanceTemperature);
			j.at("brightnessNormalizationGain").get_to(metadata.brightnessNormalizationGain);
			j.at("wbNormalizationData").get_to(metadata.wbNormalizationData);
		}
	};

	template<>
	struct adl_serializer <varjo_EyeCameraFrameMetadata> {
		static void to_json(json& j, const varjo_EyeCameraFrameMetadata& metadata)
		{
			j["timestamp"] = metadata.timestamp;
			j["glintMaskLeft"] = metadata.glintMaskLeft;
			j["glintMaskRight"] = metadata.glintMaskRight;
		}
		static void from_json(const json& j, varjo_EyeCameraFrameMetadata& metadata)
		{
			j.at("timestamp").get_to(metadata.timestamp);
			j.at("glintMaskLeft").get_to(metadata.glintMaskLeft);
			j.at("glintMaskRight").get_to(metadata.glintMaskRight);
		}
	};

	template<>
	struct adl_serializer<varjo_StreamFrame> {
		static void to_json(json& j, const varjo_StreamFrame& streamFrame)
		{
			j["type"] = streamFrame.type;
			j["id"] = streamFrame.id;
			j["frameNumber"] = streamFrame.frameNumber;
			j["channels"] = streamFrame.channels;
			j["dataFlags"] = streamFrame.dataFlags;
			j["hmdPose"] = streamFrame.hmdPose;

			// streamFrame.typeの値によって保存内容を変える
			if (streamFrame.type == varjo_StreamType_DistortedColor) {
				j["metadata"] = streamFrame.metadata.distortedColor;
			} else if (streamFrame.type == varjo_StreamType_EnvironmentCubemap) {
				j["metadata"] = streamFrame.metadata.environmentCubemap;
			} else if (streamFrame.type == varjo_StreamType_EyeCamera) {
				j["metadata"] = streamFrame.metadata.eyeCamera;
			}
		}

		static void from_json(const json& j, varjo_StreamFrame& streamFrame) {
			j.at("type").get_to(streamFrame.type);
			j.at("id").get_to(streamFrame.id);
			j.at("frameNumber").get_to(streamFrame.frameNumber);
			j.at("channels").get_to(streamFrame.channels);
			j.at("dataFlags").get_to(streamFrame.dataFlags);
			j.at("hmdPose").get_to(streamFrame.hmdPose);

			// streamFrame.typeの値によって復元内容を変える
			if (streamFrame.type == varjo_StreamType_DistortedColor) {
				j.at("metadata").get_to(streamFrame.metadata.distortedColor);
			} else if (streamFrame.type == varjo_StreamType_EnvironmentCubemap) {
				j.at("metadata").get_to(streamFrame.metadata.environmentCubemap);
			} else if (streamFrame.type == varjo_StreamType_EyeCamera) {
				j.at("metadata").get_to(streamFrame.metadata.eyeCamera);
			}
		}
	};
}
