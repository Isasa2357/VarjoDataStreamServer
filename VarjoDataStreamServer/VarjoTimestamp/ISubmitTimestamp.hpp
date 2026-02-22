#pragma once

#include <vector>
#include <queue>

#include "../util/BorrowedOrOwned.hpp"
#include "Timestamp_types.hpp"

namespace Timestamp {

	class ISubmitTimestamp {

	public:
		virtual ~ISubmitTimestamp() = default;
	
		virtual void submit_TimestampData(const TimestampData& data) = 0;
		virtual void submit_TimestampData(TimestampData&& data) = 0;

		virtual void submit_TimestampData(const std::vector<TimestampData>& data_vec) = 0;
		virtual void submit_TimestampData(std::vector<TimestampData>&& data_vec) = 0;

		virtual void submit_TimestampData(std::deque<TimestampData>& data_que) = 0;
		virtual void submit_TimestampData(std::deque<TimestampData>&& data_que) = 0;

	protected:
		virtual void submit_TimestampData_impl(BorrowedOrOwned<TimestampData> data) = 0;
	};
}