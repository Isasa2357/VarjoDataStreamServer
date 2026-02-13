#pragma once

#include <string>
#include <variant>

namespace VarjoVSTFrame {

	struct FramePair {
		varjo_ChannelIndex channel_index;
		Frame left;
		Frame right;
	};

	/**
	 * @brief 動画ファイルのコンテナを示す．
	 */
	enum class VideoContainer {
		mp4, mkv
	};

	/**
	 * @brief 処理デバイスを示す．
	 */
	enum class Device {
		CPU, GPU
	};

	/**
	 * @brief 入力するフレームデータがパディング付きかそうでないかを示す．
	 */
	enum class InputFramedataPaddingOption {
		WithoutPadding, WithPadding
	};

	/**
	 * @brief エンコードのクオリティを示す．
	 */
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

	struct VideoWriteEncodeOptions {
		size_t width;
		size_t height;
		int framerate;
		std::string out_path;
		VideoContainer container;
		EncodeOptions encode_opt;
	};

	

} // namespace VarjoVSTFrame