
#pragma	once

#include <vector>
#include <queue>

#include "../util/BorrowedOrOwned.hpp"
#include "EyeCam_types.hpp"

namespace EyeCam {

	class ISubmitFrame {
	public:
		~ISubmitFrame() = default;

		virtual void submit_Frame(const Frame& data) = 0;
		virtual void submit_Frame(Frame&& data) = 0;
		
		virtual void submit_Frame(const std::vector<Frame>& data) = 0;
		virtual void submit_Frame(std::vector<Frame>&& data) = 0;

		virtual void submit_Frame(std::queue<Frame>& data) = 0;
		virtual void submit_Frame(std::queue<Frame>&& data) = 0;

	protected:
		virtual void submit_Frame_impl(BorrowedOrOwned<Frame> data) = 0;
	};

	class ISubmitFramedata {

	public:
		~ISubmitFramedata() = default;

		virtual void submit_Framedata(const Framedata& data) = 0;
		virtual void submit_Framedata(Framedata&& data) = 0;

		virtual void submit_Framedata(const std::vector<Framedata>& data) = 0;
		virtual void submit_Framedata(std::vector<Framedata>&& data) = 0;

		virtual void submit_Framedata(std::queue<Framedata>& data) = 0;
		virtual void submit_Framedata(std::queue<Framedata>&& data) = 0;

	protected:
		virtual void submit_Framedata_impl(BorrowedOrOwned<Framedata> data) = 0;
	};

	class ISubmitMetadata {

	public:
		~ISubmitMetadata() = default;

		virtual void submit_Metadata(const Metadata& data) = 0;
		virtual void submit_Metadata(Metadata&& data) = 0;

		virtual void submit_Metadata(const std::vector<Metadata>& data) = 0;
		virtual void submit_Metadata(std::vector<Metadata>&& data) = 0;

		virtual void submit_Metadata(std::queue<Metadata>& data) = 0;
		virtual void submit_Metadata(std::queue<Metadata>&& data) = 0;

	protected:
		virtual void submit_Metadata_impl(BorrowedOrOwned<Metadata> data) = 0;
	};
}