#pragma once

#include "VarjoVSTCamStreamer.hpp"
#include "VarjoVSTVideoPreviewer.hpp"
#include "VarjoVSTVideoWriter.hpp"

namespace VarjoVSTFrame {

	struct VarjoVSTFramedataDispatcherOptions;

	class VarjoVSTFramedataDispatcher {

		VarjoVSTFramedataDispatcher(const VarjoVSTFramedataDispatcherOptions& options);

		void dispatch(const VSTCamStreamer::Framedata& framedata);

	private:

		std::unique_ptr<VarjoVSTVideoWriter> writer_{nullptr};
		std::unique_ptr<VarjoVSTVideoPreviewer> previewer_{nullptr};

	};

	struct VarjoVSTFramedataDispatcherOptions {
		// common
		size_t width;
		size_t height;
		size_t row_stride;

		bool writer_option;
		bool previewer_option;
		
		// VarjoVSTVideoWriter
		Codec writer_codec;
		std::string writer_out_path;
		int writer_crf;
		int writer_framerate;
		bool writer_parallel_write_option;
		size_t writer_buffer_size;

		// VarjoVSTVideoPreviewer
		bool preview_parallel_option;
	};

	inline VarjoVSTFramedataDispatcherOptions make_dispatcherOptions(
		const size_t width,
		const size_t height,
		const size_t row_stride,
		const bool writer_option,
		const bool previewer_option,
		const Codec writer_codec = Codec::libx264,
		const std::string writer_out_path = "sample.mp4",
		const int writer_crf = 18,
		const int writer_framerate = 90,
		const bool writer_parallel_write_option = true,
		const size_t writer_buffer_size = 20,
		const bool preview_parallel_option = false
	) {
		VarjoVSTFramedataDispatcherOptions options;
		options.width = width;
		options.height = height;
		options.row_stride = row_stride;

		options.writer_option = writer_option;
		options.previewer_option = previewer_option;

		options.writer_codec = writer_codec;
		options.writer_out_path = writer_out_path;
		options.writer_crf = writer_crf;
		options.writer_framerate = writer_framerate;
		options.writer_parallel_write_option = writer_parallel_write_option;
		options.writer_buffer_size = writer_buffer_size;

		options.preview_parallel_option = preview_parallel_option;
	}
}