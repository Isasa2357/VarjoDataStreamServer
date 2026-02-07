#pragma once

#include <vector>
#include <span>
#include <cstdint>

namespace VarjoVSTFrame {

	inline void remove_padding(
		const std::vector<uint8_t>& raw_frameData,
		std::vector<uint8_t>& out_frameData,
		const size_t width,
		const size_t height,
		const size_t row_stride
	)
	{
		const size_t frameSize_withoutPadding = width * height;
		const size_t y_plane_size = row_stride * height;
		const size_t uv_plane_size = row_stride * (height / 2);
		const std::span<const uint8_t> y_plane(raw_frameData.data(), y_plane_size);
		const std::span<const uint8_t> uv_plane(raw_frameData.data() + y_plane_size, uv_plane_size);
		// Yプレーンのコピー
		for (auto i = 0; i < height; ++i) {
			memcpy(
				out_frameData.data() + i * width,
				y_plane.data() + i * row_stride,
				width
			);
		}
		// UVプレーンのコピー
		for (auto i = 0; i < height / 2; ++i) {
			memcpy(
				out_frameData.data() + frameSize_withoutPadding + i * width,
				uv_plane.data() + i * row_stride,
				width
			);
		}
	}

	inline std::vector<uint8_t> remove_padding(
		const std::vector<uint8_t>& raw_frameData,
		const size_t width,
		const size_t height,
		const size_t row_stride
	) {
		std::vector<uint8_t> out_frameData(width * height + (width * height) / 2);
		remove_padding(raw_frameData, out_frameData, width, height, row_stride);
		return out_frameData;
	}
}