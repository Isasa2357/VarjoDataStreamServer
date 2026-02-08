#include <iostream>
#include <thread>
#include <chrono>

#include "VarjoVSTFrame/VarjoVSTCamStreamer.hpp"
#include "VarjoVSTFrame/VarjoVSTVideoPreviewer.hpp"
#include "VarjoVSTFrame/VarjoVSTVideoWriter.hpp"

int main(void)
{

	try {
		// セッションの開始
		auto session = std::make_shared<Session>();
		if (!session->isValid()) {
			throw std::runtime_error("Failed to initialize session. Is Varjo system running?");
		}

		// ストリームの開始
		
		VarjoVSTFrame::VarjoVSTCamStreamer dstreamer(session, varjo_ChannelFlag_First | varjo_ChannelFlag_Second);
		auto stream_config = dstreamer.getConfig();
		auto width = stream_config->width;
		auto height = stream_config->height;
		auto row_stride = stream_config->rowStride;
		VarjoVSTFrame::VarjoVSTSerialVideoPreviewer viewer(width, height, row_stride);
		VarjoVSTFrame::VarjoVSTParallelVideoWriter writer(width, height, row_stride, VarjoVSTFrame::Codec::h264_nvenc, VarjoVSTFrame::VideoContainer::mp4, "left_vstvideo_byVSTWriter.mp4", 18, 90, VarjoVSTFrame::InputFramedataPaddingOption::WithPadding);
		dstreamer.startStream();
		viewer.open();
		writer.open();

		auto start = std::chrono::high_resolution_clock::now();
		while (true) {
			auto ldata = dstreamer.take_lframe_que();
			auto rdata = dstreamer.take_rframe_que();

			std::cout << "l frame que size: " << dstreamer.left_frame_que_size() << std::endl;
			std::cout << "r frame que size: " << dstreamer.right_frame_que_size() << std::endl;

			while (!ldata.first.empty()) {
				auto f = std::move(ldata.first.front());
				viewer.submit_frame(f);
				writer.submit_frame(f);
				ldata.first.pop();
			}

			auto now = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
			if (duration >= 3 * 1000) break;

			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		dstreamer.stopStream();
		viewer.close();
		writer.close();
	} catch (const std::exception& e) {
		std::cerr << "Critical error caught: " << e.what();
		return EXIT_FAILURE;
	}
}