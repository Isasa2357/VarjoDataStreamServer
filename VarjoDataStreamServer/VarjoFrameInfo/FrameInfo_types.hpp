#pragma once

#include <array>

#include <Varjo_types.h>

namespace VarjoFrameInfo {

	struct FrameInfoData {
		// varjo_FrameInfo
		std::array<varjo_ViewInfo, 4> views;
		std::array<varjo_FovTangents, 4> fovTangents;
		varjo_Nanoseconds timestamp;
		int64_t frameNumber;
	};

}