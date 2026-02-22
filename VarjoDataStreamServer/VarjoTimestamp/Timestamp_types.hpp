#pragma once

#include <chrono>

#include <Varjo_types.h>

namespace Timestamp {

	struct TimestampData {
		varjo_Nanoseconds varjo_timestamp;
		varjo_Nanoseconds varjo_timestamp_unix;
		std::chrono::system_clock::time_point system_timestamp;
	};
}