

#include "VarjoVSTFrameDispatcher.hpp"

namespace VarjoVSTFrame {

	class Dispatcher : public ISubmitFrame {
	public:
		Dispatcher();


	private:
		std::vector<std::unique_ptr<ISubmitFramedata>> submit_framedata_sink_;
		std::vector<std::unique_ptr<ISubmitMetadata>> submit_metadata_sink_;
	};
}