#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>

#include "VarjoVSTFrame/VarjoVSTCamStreamer.hpp"
#include "VarjoVSTFrame/VarjoVSTVideoPreviewer.hpp"
#include "VarjoVSTFrame/VarjoVSTVideoWriter.hpp"
#include "util/vec_util.hpp"
#include "util/struct_json_io.hpp"

int main(void)
{

	try {
		// セッションの開始
		auto session = std::make_shared<Session>();
		if (!session->isValid()) {
			throw std::runtime_error("Failed to initialize session. Is Varjo system running?");
		}

		// ストリームの開始
		
		//VarjoVSTFrame::VarjoVSTCamStreamer dstreamer(session, varjo_ChannelFlag_First | varjo_ChannelFlag_Second);
		VarjoVSTFrame::VarjoVSTDummyCamStreamer dstreamer(varjo_ChannelFlag_First | varjo_ChannelFlag_Second);
		auto stream_config = dstreamer.getConfig();
		auto width = stream_config->width;
		auto height = stream_config->height;
		auto row_stride = stream_config->rowStride;
		std::cout << "width: " << width << std::endl;
		std::cout << "height: " << height << std::endl;
		std::cout << "row_stride: " << row_stride << std::endl;
		VarjoVSTFrame::VarjoVSTSerialVideoPreviewer viewer(width, height, row_stride, VarjoVSTFrame::InputFramedataPaddingOption::WithPadding);

		// construct video writer
		auto x264_opt = VarjoVSTFrame::make_X264Options(
			VarjoVSTFrame::X264Options::X264Preset::Veryfast,
			VarjoVSTFrame::X264Options::Mode::Crf,
			0, 0
		);
		auto vw_encode_opt = VarjoVSTFrame::make_VideoWriteEncodeOptions(
			width, height, 90, "left_vst.mp4", VarjoVSTFrame::VideoContainer::mp4, x264_opt
		);
		VarjoVSTFrame::VarjoVSTParallelVideoWriter writer(
			vw_encode_opt, row_stride, VarjoVSTFrame::InputFramedataPaddingOption::WithPadding
		);
		dstreamer.startStream();

		viewer.open();
		writer.open();

		auto start = std::chrono::high_resolution_clock::now();
		while (true) {
			auto ldata = dstreamer.take_lframe_que();
			auto rdata = dstreamer.take_rframe_que();

			while (!ldata.first.empty()) {
				auto f = std::move(ldata.first.front());
				auto m = std::move(ldata.second.front());

				viewer.submit_framedata(f, m);

				writer.submit_framedata(f, m);

				ldata.first.pop();
			}

			auto now = std::chrono::high_resolution_clock::now();
			
			if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
				break;
			}

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

//int  main(void)
//{
//	try {
//		// セッションの開始
//		auto session = std::make_shared<Session>();
//		if (!session->isValid()) {
//			throw std::runtime_error("Failed to initialize session. Is Varjo system running?");
//		}
//
//		// ストリームの開始
//		VarjoVSTFrame::VarjoVSTCamStreamer dstreamer(session, varjo_ChannelFlag_First | varjo_ChannelFlag_Second, 300);
//		std::pair<std::queue<VarjoVSTFrame::Framedata>, std::queue<VarjoVSTFrame::Metadata>> ldata;
//		std::pair<std::queue<VarjoVSTFrame::Framedata>, std::queue<VarjoVSTFrame::Metadata>> rdata;
//
//		dstreamer.startStream();
//		while (true) {
//			std::this_thread::sleep_for(std::chrono::milliseconds(100));
//			std::cout << "left frame que size: " << dstreamer.left_frame_que_size() << std::endl;
//			std::cout << "right frame que size: " << dstreamer.right_frame_que_size() << std::endl;
//			if (dstreamer.left_frame_que_size() > 180 && dstreamer.right_frame_que_size() > 180) {
//				ldata = dstreamer.take_lframe_que();
//				rdata = dstreamer.take_rframe_que();
//				break;
//			}
//		}
//		dstreamer.stopStream();
//
//		// 書き出し
//		std::string directory = "./frame_data/";
//		for (auto i = 0; i < 180; ++i) {
//			std::cout << "write frame data " << i << std::endl;
//			auto lframedata = std::move(ldata.first.front());
//			auto lmetadata = std::move(ldata.second.front());
//			auto rframedata = std::move(rdata.first.front());
//			auto rmetadata = std::move(rdata.second.front());
//
//			// serialize framedata
//			vecutil::serialize_vector(lframedata, directory + "lframedata_" + std::to_string(i) + ".bin");
//			vecutil::serialize_vector(rframedata, directory + "rframedata_" + std::to_string(i) + ".bin");
//
//			// conv json metadata
//			::nlohmann::json lmeta_json = lmetadata;
//			std::ofstream lofs(directory + "lmetadata_" + std::to_string(i) + ".json");
//			lofs << lmeta_json.dump(4);
//
//			::nlohmann::json rmeta_json = rmetadata;
//			std::ofstream rofs(directory + "rmetadata_" + std::to_string(i) + ".json");
//			rofs << rmeta_json.dump(4);
//
//			ldata.first.pop();
//			ldata.second.pop();
//			rdata.first.pop();
//			rdata.second.pop();
//
//		}
//
//
//	}  catch (const std::exception& e) {
//		std::cerr << "Critical error caught: " << e.what();
//		return EXIT_FAILURE;
//	}
//}