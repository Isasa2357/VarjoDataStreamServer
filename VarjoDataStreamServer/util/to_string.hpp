#pragma once

#include <string>

#include <Varjo_types.h>

/****************************************************************************************************************
	Varjo_types.h
****************************************************************************************************************/

inline std::string to_string_varjo_types_GazeEyeStatus(const varjo_GazeEyeStatus& status) {
	switch (status) {
	case varjo_GazeEyeStatus_Invalid:
		return "Invalid";
	case varjo_GazeEyeStatus_Visible:
		return "Visible";
	case varjo_GazeEyeStatus_Compensated:
		return "Compensated";
	case varjo_GazeEyeStatus_Tracked:
		return "Tracked";
	default:
		return "Unknown status: " + std::to_string(status);
	}
}

inline std::string to_string_varjo_types_GazeStatus(const varjo_GazeStatus& status) {
	switch (status) {
	case varjo_GazeStatus_Invalid:
		return "Invalid";
	case varjo_GazeStatus_Adjust:
		return "Adjust";
	case varjo_GazeStatus_Valid:
		return "Valid";
	default:
		return "Unknown status: " + std::to_string(status);
	}
}