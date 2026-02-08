#pragma once

#include <vector>
#include <span>
#include <cstdint>

#include "varjo_vst_frame_type.hpp"

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

	inline std::string videoContainer_toString(const VideoContainer container) {
		switch (container) {
		case VideoContainer::mp4:
			return "mp4";
		case VideoContainer::mkv:
			return "mkv";
		default:
			return "mp4";
		}
	}

	//------------------------------ codec option factory

	X264Options make_X264Options(
		const X264Options::X264Preset preset, 
		const X264Options::Mode mode = X264Options::Mode::Crf,
		const int crf = 19, 
		const int qp = 0
	) {
		X264Options opt;
		
		opt.preset = preset;
		opt.mode = mode;
		opt.crf = crf;
		opt.qp = qp;

		return opt;
	}

	X264Options make_X264Options(const Quality quality) {
		X264Options opt;
		opt.preset = X264Options::X264Preset::Veryfast;

		opt.mode = X264Options::Mode::Crf;

		switch (quality) {
		case Quality::Lossless:
			opt.crf = 0;  
			break;
		case Quality::High:
			opt.crf = 18;
			break;
		case Quality::Medium:
			opt.crf = 23;
			break;
		case Quality::Low:
			opt.crf = 28;
			break;
		default:
			opt.crf = 19;
			break;
		}

		opt.qp = 0;
		return opt;
	}


	NvencH264Options make_NvencH264Options(
		const NvencH264Options::NvencPreset preset,
		const NvencH264Options::NvencRc rc = NvencH264Options::NvencRc::VbrHq,
		const int cq = 19,
		const int qp = 0,
		const bool spatial_aq = false,
		const bool temporal_aq = false
	) {
		NvencH264Options opt;

		opt.preset = preset;
		opt.rc = rc;
		opt.cq = cq;
		opt.qp = qp;
		opt.spatial_aq = spatial_aq;
		opt.temporal_aq = temporal_aq;

		return opt;
	}

	NvencH264Options make_NvencH264Options(const Quality quality) {
		NvencH264Options opt;
		opt.preset = NvencH264Options::NvencPreset::P1;
		opt.spatial_aq = false;
		opt.temporal_aq = false;

		switch (quality) {
		case Quality::Lossless:
			// “極力ロスレス狙い”
			opt.rc = NvencH264Options::NvencRc::ConstQp;
			opt.qp = 0;
			opt.cq = 0;   // rc=ConstQp のときは基本無視される
			break;
		case Quality::High:
			opt.rc = NvencH264Options::NvencRc::VbrHq;
			opt.cq = 19;
			opt.qp = 0;
			break;
		case Quality::Medium:
			opt.rc = NvencH264Options::NvencRc::VbrHq;
			opt.cq = 23;
			opt.qp = 0;
			break;
		case Quality::Low:
			opt.rc = NvencH264Options::NvencRc::VbrHq;
			opt.cq = 28;
			opt.qp = 0;
			break;
		default:
			opt.rc = NvencH264Options::NvencRc::VbrHq;
			opt.cq = 19;
			opt.qp = 0;
			break;
		}

		return opt;
	}

	Ffv1Options make_Ffv1Options(
		const int level
	) {
		Ffv1Options opt;

		opt.level = level;

		return opt;
	}

	Ffv1Options make_Ffv1Options(const Quality quality)
	{
		Ffv1Options opt;
		opt.level = 3; 
		return opt;
	}
}