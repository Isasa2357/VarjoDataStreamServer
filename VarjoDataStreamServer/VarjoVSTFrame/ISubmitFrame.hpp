#pragma once

#include "VarjoVSTCamStreamer.hpp"


namespace VarjoVSTFrame {

	class ISubmitFrame {
	public:
		virtual ~ISubmitFrame() = default;

		virtual	void submit_frame(const Frame& frame) = 0;
		virtual void submit_frame(Frame&& frame) = 0;
	protected:
		virtual void submit_frame_impl(Frame&& frame) = 0;
	};

	class ISubmitFramedata {
	public:
		virtual ~ISubmitFramedata() = default;

		virtual void submit_framedata(const Framedata& framedata) = 0;
		virtual void submit_framedata(Framedata&& frameData) = 0;

	protected:
		virtual void submit_framedata_impl(Framedata&& framedta) = 0;
	};

	class ISubmitMetadata {
	public:
		virtual ~ISubmitMetadata() = default;
		virtual void submit_metadata(const Metadata& metadata) = 0;
		virtual void submit_metadata(Metadata&& metadata) = 0;

	protected:
		virtual void submit_metadata_impl(Metadata&& metadata) = 0;
	};

	class ISubmitFramePair {
	public:
		virtual ~ISubmitFramePair() = default;
		virtual void submit_framepair(const FramePair& framepair) = 0;
		virtual void submit_framepair(FramePair&& framepair) = 0;

	protected:
		virtual void submit_framepair_impl(FramePair&& framepair) = 0;
	};
}