#include "VarjoVSTCamStreamer.hpp"


namespace VarjoVSTFrame {
	VarjoVSTCamStreamer::VarjoVSTCamStreamer(const std::shared_ptr<Session>& session, varjo_ChannelFlag chnls, const size_t buffer_capacity)
		: session_(session),
		dstreamer_(*session, std::bind(&VarjoVSTCamStreamer::onFrameReceived, this, std::placeholders::_1)),
		chnls_(chnls), 
		buffer_capacity_(buffer_capacity)
	{}

	std::optional<varjo_StreamConfig> VarjoVSTCamStreamer::getConfig() const
	{
		return this->dstreamer_.getConfig(varjo_StreamType_DistortedColor);
	}

	void VarjoVSTCamStreamer::startStream() {
		this->dstreamer_.startDataStream(varjo_StreamType_DistortedColor, varjo_TextureFormat_NV12, this->chnls_);
	}

	void VarjoVSTCamStreamer::stopStream() {
		this->dstreamer_.stopDataStream(varjo_StreamType_DistortedColor, varjo_TextureFormat_NV12);
	}

	std::pair<std::queue<Framedata>, std::queue<Metadata>> VarjoVSTCamStreamer::take_lframe_que() {

		std::lock_guard lk(this->lframe_que_mtx_);
		return std::make_pair(
			std::exchange(this->lframedata_que_, std::queue<Framedata>()),
			std::exchange(this->lmetadata_que_, std::queue<Metadata>())
		);

		/*auto ret = std::pair<std::queue<VSTCamStreamer::Framedata>, std::queue<VSTCamStreamer::Metadata>>(this->lframedata_que_, this->lmetadata_que_);
		while (!this->lframedata_que_.empty()) {
			this->lframedata_que_.pop();
			this->lmetadata_que_.pop();
		}
		return ret;*/
	}

	std::pair<std::queue<Framedata>, std::queue<Metadata>> VarjoVSTCamStreamer::take_rframe_que() {
		std::lock_guard lk(this->rframe_que_mtx_);
		return std::make_pair(
			std::exchange(this->rframedata_que_, std::queue<Framedata>()),
			std::exchange(this->rmetadata_que_, std::queue<Metadata>())
		);
		/*auto ret = std::pair<std::queue<VSTCamStreamer::Framedata>, std::queue<VSTCamStreamer::Metadata>>(this->rframedata_que_, this->rmetadata_que_);
		while (!this->rframedata_que_.empty()) {
			this->rframedata_que_.pop();
			this->rmetadata_que_.pop();
		}
		return ret;*/
	}

	void VarjoVSTCamStreamer::onFrameReceived(const Frame& frame) {
		if (frame.metadata.channelIndex == varjo_ChannelIndex_Left) {
			std::lock_guard lk(this->lframe_que_mtx_);

			this->lframedata_que_.emplace(frame.data);
			this->lmetadata_que_.emplace(frame.metadata);

			// —e—Ê‚ð’´‚¦‚½‚à‚Ì‚ÍØ‚èŽÌ‚Ä
			if (this->lframedata_que_.size() > this->buffer_capacity_) {
				for (auto i = 0; i < this->lframedata_que_.size() - this->buffer_capacity_; ++i) {
					this->lframedata_que_.pop();
					this->lmetadata_que_.pop();
				}
			}
		} else {
			std::lock_guard lk(this->rframe_que_mtx_);

			 this->rframedata_que_.emplace(frame.data);
			 this->rmetadata_que_.emplace(frame.metadata);

			// —e—Ê‚ð’´‚¦‚½‚à‚Ì‚ÍØ‚èŽÌ‚Ä
			if (this->rframedata_que_.size() > this->buffer_capacity_) {
				for (auto i = 0; i < this->rframedata_que_.size() - this->buffer_capacity_; ++i) {
					this->rframedata_que_.pop();
					this->rmetadata_que_.pop();
				}
			}
		}
	}

	VarjoVSTDummyCamStreamer::VarjoVSTDummyCamStreamer(varjo_ChannelFlag chnls, const size_t buffer_capacity, const int fps)
		: chnls_(chnls)
		, fps_(fps)
		, buffer_capacity_(buffer_capacity)
	{
		//----- load left frame
		if (this->chnls_ & varjo_ChannelFlag_Left) {
			printf("load left frame\n");
			this->loaded_lframedata_.resize(180);
			this->loaded_lmetadata_.resize(180);
			for (auto i = 0; i < 180; ++i) {
				// load framedata
				this->loaded_lframedata_[i] = vecutil::deserialize_vector<uint8_t>("./VarjoVSTFrame/DummyFrameData/lframedata_" + std::to_string(i) + ".bin");

				// load metadata
				std::fstream ifs("./VarjoVSTFrame/DummyFrameData/lmetadata_" + std::to_string(i) + ".json");
				nlohmann::json j = nlohmann::json::parse(ifs);
				this->loaded_lmetadata_[i] = j.get<Metadata>();
			}
		}

		if (this->chnls_ & varjo_ChannelFlag_Right) {
			this->loaded_rframedata_.resize(180);
			this->loaded_rmetadata_.resize(180);
			for (auto i = 0; i < 180; ++i) {
				// load framedata
				this->loaded_rframedata_[i] = vecutil::deserialize_vector<uint8_t>("./VarjoVSTFrame/DummyFrameData/rframedata_" + std::to_string(i) + ".bin");
				// load metadata
				std::fstream ifs("./VarjoVSTFrame/DummyFrameData/rmetadata_" + std::to_string(i) + ".json");
				nlohmann::json j = nlohmann::json::parse(ifs);
				this->loaded_rmetadata_[i] = j.get<Metadata>();
			}
		}
	}

	std::optional<varjo_StreamConfig> VarjoVSTDummyCamStreamer::getConfig() const
	{
		varjo_StreamConfig config{};
		config.streamId = 1;
		config.channelFlags = this->chnls_;
		config.streamType = varjo_StreamType_DistortedColor;
		config.bufferType = varjo_BufferType_CPU;
		config.format = varjo_TextureFormat_NV12;
		config.frameRate = this->fps_;
		config.width = 832;
		config.height = 640;
		config.rowStride = 896;
		return config;
	}

	void VarjoVSTDummyCamStreamer::startStream()
	{
		this->stop_worker_signal_ = false;
		this->onFrame_thread_ = std::thread(&VarjoVSTDummyCamStreamer::onFrameReceivedworker, this);
	}

	void VarjoVSTDummyCamStreamer::stopStream()
	{
		this->stop_worker_signal_.store(true);
		this->onFrame_thread_.join();

	}

	std::pair<std::queue<Framedata>, std::queue<Metadata>> VarjoVSTDummyCamStreamer::take_lframe_que()
	{
		std::lock_guard lk(this->lframe_que_mtx_);
		return std::make_pair(
			std::exchange(this->lframedata_que_, std::queue<Framedata>()),
			std::exchange(this->lmetadata_que_, std::queue<Metadata>())
		);
	}

	std::pair<std::queue<Framedata>, std::queue<Metadata>> VarjoVSTDummyCamStreamer::take_rframe_que() 
	{
		std::lock_guard lk(this->rframe_que_mtx_);
		return std::make_pair(
			std::exchange(this->rframedata_que_, std::queue<Framedata>()),
			std::exchange(this->rmetadata_que_, std::queue<Metadata>())
		);
	}

	void VarjoVSTDummyCamStreamer::onFrameReceivedworker()
	{
		int rec_idx = 0;
		while (!this->stop_worker_signal_) {
			if (this->chnls_ & varjo_ChannelFlag_Left) {
				std::lock_guard lk(this->lframe_que_mtx_);
				this->lframedata_que_.push(this->loaded_lframedata_[rec_idx]);
				this->lmetadata_que_.push(this->loaded_lmetadata_[rec_idx]);
			}
			if (this->chnls_ & varjo_ChannelFlag_Right) {
				std::lock_guard lk(this->rframe_que_mtx_);
				this->rframedata_que_.push(this->loaded_rframedata_[rec_idx]);
				this->rmetadata_que_.push(this->loaded_rmetadata_[rec_idx]);
			}
			rec_idx = (rec_idx + 1) % this->loaded_lframedata_.size();

			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / this->fps_));
		}
	}

}