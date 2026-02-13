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
			j["value"] = std::move(array_to_jsonArray(m.value, 9));
		}
		static void from_json(const json& j, varjo_Matrix3x3& m)
		{
			j.at("value").get_to(m.value);
		}
	};

	template<>
	struct adl_serializer<varjo_Ray> {
		static void to_json(json& j, const varjo_Ray& ray)
		{
			j["origin"] = std::move(array_to_jsonArray(ray.origin, 3));
			j["forward"] = std::move(array_to_jsonArray(ray.forward, 3));
		}

		static void from_json(const json& j, varjo_Ray& ray)
		{
			j.at("origin").get_to(ray.origin);
			j.at("forward").get_to(ray.forward);
		}
	};

	template<>
	struct adl_serializer<varjo_Vector2Df> {
		static void to_json(json& j, const varjo_Vector2Df& vec)
		{
			j["x"] = vec.x;
			j["y"] = vec.y;
		}

		static void from_json(const json& j, varjo_Vector2Df& vec)
		{
			j.at("x").get_to(vec.x);
			j.at("y").get_to(vec.y);
		}
	};

	template<>
	struct adl_serializer<varjo_Vector3D> {
		static void to_json(json& j, const varjo_Vector3D& vec)
		{
			j["x"] = vec.x;
			j["y"] = vec.y;
			j["z"] = vec.z;
		}

		static void from_json(const json& j, varjo_Vector3D& vec) {
			j.at("x").get_to(vec.x);
			j.at("y").get_to(vec.y);
			j.at("z").get_to(vec.z);
		}
	};

	template<>
	struct adl_serializer<varjo_Vector3Df> {
		static void to_json(json& j, const varjo_Vector3Df& vec)
		{
			j["x"] = vec.x;
			j["y"] = vec.y;
			j["z"] = vec.z;
		}
		static void from_json(const json& j, varjo_Vector3Df& vec) {
			j.at("x").get_to(vec.x);
			j.at("y").get_to(vec.y);
			j.at("z").get_to(vec.z);
		}
	};

	template<>
	struct adl_serializer<varjo_Vector3Di> {
		static void to_json(json& j, const varjo_Vector3Di& vec)
		{
			j["x"] = vec.x;
			j["y"] = vec.y;
			j["z"] = vec.z;
		}
		static void from_json(const json& j, varjo_Vector3Di& vec) {
			j.at("x").get_to(vec.x);
			j.at("y").get_to(vec.y);
			j.at("z").get_to(vec.z);
		}
	};

	template<>
	struct adl_serializer<varjo_Size3D> {
		static void to_json(json& j, const varjo_Size3D& size)
		{
			j["width"] = size.width;
			j["height"] = size.height;
			j["depth"] = size.depth;
		}
		static void from_json(const json& j, varjo_Size3D& size)
		{
			j.at("width").get_to(size.width);
			j.at("height").get_to(size.height);
			j.at("depth").get_to(size.depth);
		}
	};

	template<>
	struct adl_serializer<varjo_AlignedView> {
		static void to_json(json& j, const varjo_AlignedView& view) 
		{
			j["projectionTop"] = view.projectionTop;
			j["projectionBottom"] = view.projectionBottom;
			j["projectionLeft"] = view.projectionLeft;
			j["projectionRight"] = view.projectionRight;
		}

		static void from_json(const json& j, varjo_AlignedView& view)
		{
			j.at("projectionTop").get_to(view.projectionTop);
			j.at("projectionBottom").get_to(view.projectionBottom);
			j.at("projectionLeft").get_to(view.projectionLeft);
			j.at("projectionRight").get_to(view.projectionRight);
		}
	};

	template<>
	struct adl_serializer<varjo_ViewInfo> {
		static void to_json(json& j, const varjo_ViewInfo& viewInfo)
		{
			j["projectionMatrix"] = array_to_jsonArray(viewInfo.projectionMatrix, 16);
			j["viewMatrix"] = array_to_jsonArray(viewInfo.viewMatrix, 16);
			j["preferredWidth"] = viewInfo.preferredWidth;
			j["preferredHeight"] = viewInfo.preferredHeight;
			j["enabled"] = viewInfo.enabled;
		}

		static void from_json(const json& j, varjo_ViewInfo& viewInfo) 
		{
			j["projectionMatrix"].get_to(viewInfo.projectionMatrix);
			j["viewMatrix"].get_to(viewInfo.viewMatrix);
			j["preferredWidth"].get_to(viewInfo.preferredWidth);
			j["preferredHeight"].get_to(viewInfo.preferredHeight);
			j["enabled"].get_to(viewInfo.enabled);
		}
	};

	template<>
	struct adl_serializer<varjo_ViewDescription> {
		static void to_json(json& j, const varjo_ViewDescription& viewDesc)
		{
			j["width"] = viewDesc.width;
			j["height"] = viewDesc.height;
			j["display"] = viewDesc.display;
			j["eye"] = viewDesc.eye;
		}
		static void from_json(const json& j, varjo_ViewDescription& viewDesc)
		{
			j.at("width").get_to(viewDesc.width);
			j.at("height").get_to(viewDesc.height);
			j.at("display").get_to(viewDesc.display);
			j.at("eye").get_to(viewDesc.eye);
		}
	};

	template<>
	struct adl_serializer<varjo_Gaze> {
		static void to_json(json& j, const varjo_Gaze& gaze)
		{
			j["leftEye"] = gaze.leftEye;
			j["rightEye"] = gaze.rightEye;
			j["gaze"] = gaze.gaze;
			j["focusDistance"] = gaze.focusDistance;
			j["stability"] = gaze.stability;
			j["captureTime"] = gaze.captureTime;
			j["leftStatus"] = gaze.leftStatus;
			j["rightStatus"] = gaze.rightStatus;
			j["status"] = gaze.status;
			j["frameNumber"] = gaze.frameNumber;
		}

		static void from_json(const json& j, varjo_Gaze& gaze)
		{
			j.at("leftEye").get_to(gaze.leftEye);
			j.at("rightEye").get_to(gaze.rightEye);
			j.at("gaze").get_to(gaze.gaze);
			j.at("focusDistance").get_to(gaze.focusDistance);
			j.at("stability").get_to(gaze.stability);
			j.at("captureTime").get_to(gaze.captureTime);
			j.at("leftStatus").get_to(gaze.leftStatus);
			j.at("rightStatus").get_to(gaze.rightStatus);
			j.at("status").get_to(gaze.status);
			j.at("frameNumber").get_to(gaze.frameNumber);
		}
	};

	template<>
	struct adl_serializer<varjo_EyeMeasurements> {
		static void to_json(json& j, const varjo_EyeMeasurements& eyeMeasurements)
		{
			j["frameNumber"] = eyeMeasurements.frameNumber;
			j["captureTime"] = eyeMeasurements.captureTime;
			j["interPupillaryDistanceInMM"] = eyeMeasurements.interPupillaryDistanceInMM;
			j["leftPupilIrisDiameterRatio"] = eyeMeasurements.leftPupilIrisDiameterRatio;
			j["rightPupilIrisDiameterRatio"] = eyeMeasurements.rightPupilIrisDiameterRatio;
			j["leftPupilDiameterInMM"] = eyeMeasurements.leftPupilDiameterInMM;
			j["rightPupilDiameterInMM"] = eyeMeasurements.rightPupilDiameterInMM;
			j["leftIrisDiameterInMM"] = eyeMeasurements.leftIrisDiameterInMM;
			j["rightIrisDiameterInMM"] = eyeMeasurements.rightIrisDiameterInMM;
			j["leftEyeOpenness"] = eyeMeasurements.leftEyeOpenness;
			j["rightEyeOpenness"] = eyeMeasurements.rightEyeOpenness;
		}
		static void from_json(const json& j, varjo_EyeMeasurements& eyeMeasurements)
		{
			j.at("frameNumber").get_to(eyeMeasurements.frameNumber);
			j.at("captureTime").get_to(eyeMeasurements.captureTime);
			j.at("interPupillaryDistanceInMM").get_to(eyeMeasurements.interPupillaryDistanceInMM);
			j.at("leftPupilIrisDiameterRatio").get_to(eyeMeasurements.leftPupilIrisDiameterRatio);
			j.at("rightPupilIrisDiameterRatio").get_to(eyeMeasurements.rightPupilIrisDiameterRatio);
			j.at("leftPupilDiameterInMM").get_to(eyeMeasurements.leftPupilDiameterInMM);
			j.at("rightPupilDiameterInMM").get_to(eyeMeasurements.rightPupilDiameterInMM);
			j.at("leftIrisDiameterInMM").get_to(eyeMeasurements.leftIrisDiameterInMM);
			j.at("rightIrisDiameterInMM").get_to(eyeMeasurements.rightIrisDiameterInMM);
			j.at("leftEyeOpenness").get_to(eyeMeasurements.leftEyeOpenness);
			j.at("rightEyeOpenness").get_to(eyeMeasurements.rightEyeOpenness);
		}
	};

	template<>
	struct adl_serializer<varjo_SwapChainLimits> {
		static void to_json(json& j, const varjo_SwapChainLimits& limits)
		{
			j["minimumNumberOfTextures"] = limits.minimumNumberOfTextures;
			j["maximumNumberOfTextures"] = limits.maximumNumberOfTextures;
			j["minimumTextureWidth"] = limits.minimumTextureWidth;
			j["minimumTextureHeight"] = limits.minimumTextureHeight;
			j["maximumTextureWidth"] = limits.maximumTextureWidth;
			j["maximumTextureHeight"] = limits.maximumTextureHeight;
		}

		static void from_json(const json& j, varjo_SwapChainLimits& limits)
		{
			j.at("minimumNumberOfTextures").get_to(limits.minimumNumberOfTextures);
			j.at("maximumNumberOfTextures").get_to(limits.maximumNumberOfTextures);
			j.at("minimumTextureWidth").get_to(limits.minimumTextureWidth);
			j.at("minimumTextureHeight").get_to(limits.minimumTextureHeight);
			j.at("maximumTextureWidth").get_to(limits.maximumTextureWidth);
			j.at("maximumTextureHeight").get_to(limits.maximumTextureHeight);
		}
	};

	template<>
	struct adl_serializer<varjo_Viewport> {
		static void to_json(json& j, const varjo_Viewport& viewport)
		{
			j["x"] = viewport.x;
			j["y"] = viewport.y;
			j["width"] = viewport.width;
			j["height"] = viewport.height;
		}

		static void from_json(const json& j, varjo_Viewport& viewport)
		{
			j.at("x").get_to(viewport.x);
			j.at("y").get_to(viewport.y);
			j.at("width").get_to(viewport.width);
			j.at("height").get_to(viewport.height);
		}
	};

	template<>
	struct adl_serializer<varjo_FovTangents> {
		static void to_json(json& j, const varjo_FovTangents& fov)
		{
			j["top"] = fov.top;
			j["bottom"] = fov.bottom;
			j["left"] = fov.left;
			j["right"] = fov.right;
		}
		static void from_json(const json& j, varjo_FovTangents& fov)
		{
			j["top"].get_to(fov.top);
			j["bottom"].get_to(fov.bottom);
			j["left"].get_to(fov.left);
			j["right"].get_to(fov.right);
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
