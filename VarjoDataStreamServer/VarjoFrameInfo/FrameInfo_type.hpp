#pragma once

#include <Varjo_types.h>

namespace VarjoFrameInfo {

	struct FrameInfoData {
		// varjo_FrameInfo
		varjo_ViewInfo views;
		varjo_Nanoseconds timestamp;
		int64_t frameNumber;
	};


}