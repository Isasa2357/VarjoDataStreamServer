#include "VSTCamStreamer.hpp"


namespace VarjoVSTFrame {
	VSTCamStreamer::VSTCamStreamer(const std::shared_ptr<Session>& session, varjo_ChannelFlag chnls, const size_t buffer_capacity)
		: session_(session),
		dstreamer_(*session, std::bind(&VSTCamStreamer::onFrameReceived, this, std::placeholders::_1)),
		chnls_(chnls), 
		buffer_capacity_(buffer_capacity)
	{}

	std::optional<varjo_StreamConfig> VSTCamStreamer::getConfig() const
	{
		return this->dstreamer_.getConfig(varjo_StreamType_DistortedColor);
	}

	void VSTCamStreamer::startStream() {
		this->dstreamer_.startDataStream(varjo_StreamType_DistortedColor, varjo_TextureFormat_NV12, this->chnls_);
	}

	void VSTCamStreamer::stopStream() {
		this->dstreamer_.stopDataStream(varjo_StreamType_DistortedColor, varjo_TextureFormat_NV12);
	}

	std::pair<std::queue<VSTCamStreamer::Framedata>, std::queue<VSTCamStreamer::Metadata>> VSTCamStreamer::take_lframe_que() {

		std::lock_guard lk(this->lframe_que_mtx_);
		return std::make_pair(
			std::exchange(this->lframedata_que_, std::queue<VSTCamStreamer::Framedata>()),
			std::exchange(this->lmetadata_que_, std::queue<VSTCamStreamer::Metadata>())
		);

		/*auto ret = std::pair<std::queue<VSTCamStreamer::Framedata>, std::queue<VSTCamStreamer::Metadata>>(this->lframedata_que_, this->lmetadata_que_);
		while (!this->lframedata_que_.empty()) {
			this->lframedata_que_.pop();
			this->lmetadata_que_.pop();
		}
		return ret;*/
	}

	std::pair<std::queue<VSTCamStreamer::Framedata>, std::queue<VSTCamStreamer::Metadata>> VSTCamStreamer::take_rframe_que() {
		std::lock_guard lk(this->rframe_que_mtx_);
		return std::make_pair(
			std::exchange(this->rframedata_que_, std::queue<VSTCamStreamer::Framedata>()), 
			std::exchange(this->rmetadata_que_, std::queue<VSTCamStreamer::Metadata>())
		);
		/*auto ret = std::pair<std::queue<VSTCamStreamer::Framedata>, std::queue<VSTCamStreamer::Metadata>>(this->rframedata_que_, this->rmetadata_que_);
		while (!this->rframedata_que_.empty()) {
			this->rframedata_que_.pop();
			this->rmetadata_que_.pop();
		}
		return ret;*/
	}

	void VSTCamStreamer::onFrameReceived(const Frame& frame) {
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

}