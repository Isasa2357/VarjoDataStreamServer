
#include "EyeCamDataStreamer.hpp"

namespace EyeCam {
	EyeCamDataStreamer::EyeCamDataStreamer(
		const std::shared_ptr<Session>& session, 
		const varjo_ChannelFlag channels, 
		const size_t buffer_capacity)
		: session_(session)
		, dstreamer_(*session, std::bind(&EyeCamDataStreamer::onFrameReceived, this, std::placeholders::_1))
		, channels_(channels)
		, buffer_capacity_(buffer_capacity)
	{}

	EyeCamDataStreamer::~EyeCamDataStreamer()
	{
		this->stop_stream();
	}

	std::optional<varjo_StreamConfig> EyeCamDataStreamer::getConfig() const
	{
		return this->dstreamer_.getConfig(varjo_StreamType_EyeCamera);
	}

	void EyeCamDataStreamer::start_stream() {
		this->dstreamer_.startDataStream(varjo_StreamType_EyeCamera, varjo_TextureFormat_Y8_UNORM, this->channels_);
	}

	void EyeCamDataStreamer::stop_stream() {
		this->dstreamer_.stopDataStream(varjo_StreamType_EyeCamera, varjo_TextureFormat_Y8_UNORM);
	}

	std::pair<std::deque<Framedata>, std::deque<Metadata>> EyeCamDataStreamer::take_lframe_que()
	{
		std::lock_guard lk(this->lframe_que_mtx_);
		return std::make_pair(
			std::exchange(this->lframedata_que_, std::deque<Framedata>()),
			std::exchange(this->lmetadata_que_, std::deque<Metadata>())
		);
	}

	std::pair<std::deque<Framedata>, std::deque<Metadata>> EyeCamDataStreamer::take_rframe_que()
	{
		std::lock_guard lk(this->rframe_que_mtx_);
		return std::make_pair(
			std::exchange(this->rframedata_que_, std::deque<Framedata>()),
			std::exchange(this->rmetadata_que_, std::deque<Metadata>())
		);
	}

	void EyeCamDataStreamer::onFrameReceived(const Frame& frame)
	{
		if (frame.metadata.channelIndex == varjo_ChannelIndex_Left) {
			std::lock_guard lk(this->lframe_que_mtx_);

			this->lframedata_que_.push_back(std::move(frame.data));
			this->lmetadata_que_.push_back(std::move(frame.metadata));

			// 容量を超えたものは切り捨て
			auto que_size = this->lframedata_que_.size();
			auto excess_size = que_size > this->buffer_capacity_ ? que_size - this->buffer_capacity_ : 0;
			for (auto i = 0; i < excess_size; ++i) {
				this->lframedata_que_.pop_front();
				this->lmetadata_que_.pop_front();
			}
		} else if (frame.metadata.channelIndex == varjo_ChannelIndex_Right) {
			std::lock_guard lk(this->rframe_que_mtx_);

			this->rframedata_que_.push_back(std::move(frame.data));
			this->rmetadata_que_.push_back(std::move(frame.metadata));

			// 容量を超えたものは切り捨て
			auto que_size = this->rframedata_que_.size();
			auto excess_size = que_size > this->buffer_capacity_ ? que_size - this->buffer_capacity_ : 0;
			for (auto i = 0; i < excess_size; ++i) {
				this->rframedata_que_.pop_front();
				this->rmetadata_que_.pop_front();
			}

		} else {
			throw std::runtime_error("Unkown channel index");
		}
	}

}