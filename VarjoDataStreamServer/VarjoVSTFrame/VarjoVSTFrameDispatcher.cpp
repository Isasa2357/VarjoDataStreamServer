

#include "VarjoVSTFrameDispatcher.hpp"

namespace VarjoVSTFrame {

	VarjoVSTFramedataDispatcher::VarjoVSTFramedataDispatcher(const VarjoVSTFramedataDispatcherOptions& options) {
		if (options.writer_option) {
			if (options.writer_parallel_write_option) {
				this->writer_ = std::unique_ptr<VarjoVSTParallelVideoWriter>(
					new VarjoVSTParallelVideoWriter(
						options.width,
						options.height,
						options.row_stride,
						options.writer_codec,
						options.writer_out_path,
						options.writer_crf,
						options.writer_framerate,
						options.writer_buffer_size
					)
				);
			} else {
				this->writer_ = std::unique_ptr<VarjoVSTSerialVideoWriter>(
					new VarjoVSTSerialVideoWriter(
						options.width,
						options.height,
						options.row_stride,
						options.writer_codec,
						options.writer_out_path,
						options.writer_crf,
						options.writer_framerate,
						)
				);
			}

			if (options.previewer_option) {
				if (options.preview_parallel_option) {
					this->previewer_ = std::unique_ptr<VarjoVSTParallelVideoPreviewer>(
						new VarjoVSTParallelVideoPreviewer(options.width, options.height, 0)
					);
				} else {
					this->previewer_ = std::unique_ptr<VarjoVSTSerialVideoPreviewer>(
						new VarjoVSTSerialVideoPreviewer(options.width, options.height, 0)
					);
				}
			}
		}
	}
	void VarjoVSTFramedataDispatcher::dispatch(const VSTCamStreamer::Framedata& framedata) {

	}
}