#pragma once

#include <vector>
#include <queue>

#include "../util/BorrowedOrOwned.hpp"

#include "FrameInfo_types.hpp"

namespace VarjoFrameInfo {
	class ISubmitFrameInfo {
	public:
		virtual ~ISubmitFrameInfo() = default;

		virtual void submit_FrameInfoData(const FrameInfoData& data) = 0;
		virtual void submit_FrameInfoData(FrameInfoData&& data) = 0;
		virtual void submit_FrameInfoData(const std::vector<FrameInfoData>& data) = 0;
		virtual void submit_FrameInfoData(std::vector<FrameInfoData>&& data) = 0;
		virtual void submit_FrameInfoData(std::queue<FrameInfoData>& data) = 0;
		virtual void submit_FrameInfoData(std::queue<FrameInfoData>&& data) = 0;

	protected:
		virtual void submit_FrameInfoData_impl(BorrowedOrOwned<FrameInfoData> data) = 0;
	};
}