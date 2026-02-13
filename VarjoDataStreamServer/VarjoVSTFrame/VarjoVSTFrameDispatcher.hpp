#pragma once

#include "VarjoVSTCamStreamer.hpp"
#include "VarjoVSTVideoPreviewer.hpp"
#include "VarjoVSTVideoWriter.hpp"
#include "ISubmitFrame.hpp"

namespace VarjoVSTFrame {

	class VarjoVSTFramedataDispatcher: public ISubmitFrame {
	public:
		VarjoVSTFramedataDispatcher();

		void open_previewer(const VarjoVSTVideoPreviewerOptions previwer_opts);

		void open_videoWriter(const VarjoVSTVideoWriterOptions videoWriter_opts);


	private:
		std::vector<std::unique_ptr<ISubmitFrame>> submitFrameSinks_;
		std::vector<std::unique_ptr<ISubmitFramePair>> submitFramePairSinks_;
		std::vector<std::unique_ptr<ISubmitFramedata>> submitFramedataSinks_;
		std::vector<std::unique_ptr<ISubmitMetadata>> submitMetadataSinks_;
	}
}