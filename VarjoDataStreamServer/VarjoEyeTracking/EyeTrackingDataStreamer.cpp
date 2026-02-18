#include "EyeTrackingDataStreamer.hpp"

namespace VarjoEyeTracking {
	
	
	EyeTrackingDataStreamer::EyeTrackingDataStreamer(
		const std::shared_ptr<Session>& session, 
		const OutputFilterType outputFilterType,
		const OutputFrequency outputFrequency)
		: m_session(session)
		, outputFilterType_(outputFilterType)
		, outputFrequency_(outputFrequency)
	{
		this->initializeGazeTracking(outputFilterType, outputFrequency);
	}

	GazeTrackingStatus EyeTrackingDataStreamer::getStatus() const
	{
		varjo_SyncProperties(*m_session);

		if (!varjo_GetPropertyBool(*m_session, varjo_PropertyKey_GazeAllowed)) {
			return GazeTrackingStatus::NOT_AVAILABLE;
		}

		if (!varjo_GetPropertyBool(*m_session, varjo_PropertyKey_HMDConnected)) {
			return GazeTrackingStatus::NOT_CONNECTED;
		}

		if (varjo_GetPropertyBool(*m_session, varjo_PropertyKey_GazeCalibrating)) {
			return GazeTrackingStatus::CALIBRATING;
		}

		if (varjo_GetPropertyBool(*m_session, varjo_PropertyKey_GazeCalibrated)) {
			return GazeTrackingStatus::CALIBRATED;
		}

		return GazeTrackingStatus::NOT_CALIBRATED;
	}

	std::deque<EyeTrackingData> EyeTrackingDataStreamer::geteyeTrackingData() const
	{
		// varjo_Gazeとvarjo_EyeMeasurementsのペアのベクタを取得
		auto gaze_measurements_data = getGazeDataWithEyeMeasurements();

		// rendering gazeデータを取得
		auto rendering_gaze_data = getRenderingGazeData();

		// IPDデータを取得
		auto [user_ipd, headset_ipd] = getIPDData();

		std::deque<EyeTrackingData> ret;
	}


}