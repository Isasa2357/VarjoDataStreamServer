#pragma once

#include <string>
#include <variant>

namespace VarjoVSTFrame {
	enum class Codec {
		libx264, h264_nvenc, ffv1
	};

	enum class VideoContainer {
		mp4, mkv
	};

	enum class Device {
		CPU, GPU
	};

	enum class InputFramedataPaddingOption {
		WithoutPadding, WithPadding
	};

	enum class Quality {
		Lossless, High, Medium, Low
	};

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

} // namespace VarjoVSTFrame