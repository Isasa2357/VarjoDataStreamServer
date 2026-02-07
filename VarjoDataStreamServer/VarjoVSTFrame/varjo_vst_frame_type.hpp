#pragma once

#include <string>

namespace VarjoVSTFrame {
	enum class Codec {
		libx264, h264_nvenc
	};

	enum class Device {
		CPU, GPU
	};

	/**
	 * @brief コーデック列挙型を文字列に変換する
	 *
	 * @return string型に変換したコーディック
	 */
	inline std::string codec_toString(Codec codec) {
		switch (codec) {
		case Codec::libx264:
			return "libx264";
		case Codec::h264_nvenc:
			return "h264_nvenc";
		default:
			return "libx264";
		}
	}

	/**
	 * @brief コーデックから使用デバイスを取得する
	 *
	 * @return 使用デバイス
	 */
	inline Device get_device_from_codec(Codec codec) {
		switch (codec) {
		case Codec::libx264:
			return Device::CPU;
		case Codec::h264_nvenc:
			return Device::GPU;
		default:
			return Device::CPU;
		}
	}
} // namespace VarjoVSTFrame