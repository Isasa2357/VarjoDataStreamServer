#pragma once

#include <vector>
#include <variant>

#include <Varjo_types.h>

#include "../VarjoExample/DataStreamer.hpp"

namespace EyeCam {
	
	using Frame = VarjoExamples::DataStreamer::Frame;
	using Framedata = std::vector<uint8_t>;
	using Metadata = Frame::Metadata;

	enum class FrameLayout {
		Contiguous,	Strided
	};

	// 動画エンコードのための構造体

	struct X264Options {
		enum class X264Preset { Ultrafast, Superfast, Veryfast, Faster, Fast, Medium, Slow, Slower, Veryslow };
		enum class Mode { Crf, Qp };

		X264Preset preset = X264Preset::Veryfast;
		Mode mode = Mode::Crf;
		int crf = 18;
		int qp = 0;
	};

	struct NvencH264Options {
		enum class NvencPreset { P1, P2, P3, P4, P5, P6, P7 };
		enum class NvencRc { VbrHq, ConstQp };

		NvencPreset preset = NvencPreset::P1;
		NvencRc rc = NvencRc::VbrHq;
		int cq = 19;
		int qp = 0;
		bool spatial_aq = false;
		bool temporal_aq = false;
	};

	struct Ffv1Options {
		int level = 3;
	};

	using EncodeOptions = std::variant<
		X264Options,
		NvencH264Options,
		Ffv1Options
	>;
}