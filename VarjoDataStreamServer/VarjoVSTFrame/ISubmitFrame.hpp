#pragma once

#include "VarjoVSTCamStreamer.hpp"


namespace VarjoVSTFrame {


	class ISubmitFramedata {
	public:
		virtual ~ISubmitFramedata() = default;

		virtual void submit_framedata(const Framedata& framedata, const Metadata& metadata) = 0;
		virtual void submit_framedata(const Framedata& framedata, Metadata&& metadata) = 0;
		virtual void submit_framedata(Framedata&& frameData, const Metadata& metadata) = 0;
		virtual void submit_framedata(Framedata&& frameData, Metadata&& metadata) = 0;
		virtual void submit_framedata_impl(Framedata&& framedta, Metadata&& metadata) = 0;
	};

	class ISubmitMetadata {
	public:
		virtual ~ISubmitMetadata() = default;
		virtual void submit_metadata(const Metadata& metadata) = 0;
		virtual void submit_metadata(Metadata&& metadata) = 0;
		virtual void submit_metadata_impl(Metadata&& metadata) = 0;
	};
}