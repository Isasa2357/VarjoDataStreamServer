#include "EyeTrackingDataStreamer.hpp"

namespace VarjoEyeTracking {
	
	
	EyeTrackingDataStreamer::EyeTrackingDataStreamer(
		const std::shared_ptr<Session>& session, 
		const OutputFilterType outputFilterType,
		const OutputFrequency outputFrequency)
		: session_(session)
		, outputFilterType_(outputFilterType)
		, outputFrequency_(outputFrequency)
	{
		this->initializeGazeTracking(outputFilterType, outputFrequency);
	}

	GazeTrackingStatus EyeTrackingDataStreamer::getStatus() const
	{
		varjo_SyncProperties(*(this->session_));

		if (!varjo_GetPropertyBool(*(this->session_), varjo_PropertyKey_GazeAllowed)) {
			return GazeTrackingStatus::NOT_AVAILABLE;
		}

		if (!varjo_GetPropertyBool(*(this->session_), varjo_PropertyKey_HMDConnected)) {
			return GazeTrackingStatus::NOT_CONNECTED;
		}

		if (varjo_GetPropertyBool(*(this->session_), varjo_PropertyKey_GazeCalibrating)) {
			return GazeTrackingStatus::CALIBRATING;
		}

		if (varjo_GetPropertyBool(*(this->session_), varjo_PropertyKey_GazeCalibrated)) {
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

	void EyeTrackingDataStreamer::initializeGazeTracking(const OutputFilterType outputFilterType, const OutputFrequency outputFrequency) const
	{
		varjo_GazeParameters parameters[2];
		parameters[0].key = varjo_GazeParametersKey_OutputFilterType;
		switch (outputFilterType) {
		case OutputFilterType::NONE: parameters[0].value = varjo_GazeParametersValue_OutputFilterNone; break;
		case OutputFilterType::STANDARD: parameters[0].value = varjo_GazeParametersValue_OutputFilterStandard; break;
		default: parameters[0].value = varjo_GazeParametersValue_OutputFilterStandard; break;
		}

		parameters[1].key = varjo_GazeParametersKey_OutputFrequency;
		switch (outputFrequency) {
		case OutputFrequency::_100HZ: parameters[1].value = varjo_GazeParametersValue_OutputFrequency100Hz; break;
		case OutputFrequency::_200HZ: parameters[1].value = varjo_GazeParametersValue_OutputFrequency200Hz; break;
		case OutputFrequency::MAXIMUM:
		default: parameters[1].value = varjo_GazeParametersValue_OutputFrequencyMaximumSupported; break;
		}

		varjo_GazeInitWithParameters(*(this->session_), parameters, static_cast<int32_t>(std::size(parameters)));
	}

	std::pair<std::deque<varjo_Gaze>, std::deque<varjo_EyeMeasurements>> EyeTrackingDataStreamer::getGazeDataWithEyeMeasurements() const
	{
		constexpr size_t c_growStep = 16;

		std::deque<varjo_Gaze> gaze_datas;
		std::deque<varjo_EyeMeasurements> eye_measurements_datas;

		std::array<varjo_Gaze, c_growStep> gaze_buffer;
		std::array<varjo_EyeMeasurements, c_growStep> eyeMeasurements_buffer;

		int32_t new_items = 0;
		while (new_items == c_growStep) {
			// varjo_Gazeとvarjo_EyeMeasurementsを受信
			new_items = varjo_GetGazeDataArray(
				*(this->session_),
				gaze_buffer.data(),
				eyeMeasurements_buffer.data(),
				c_growStep
			);

			// 受信したデータをdequeに追加
			for (auto i = 0; i < new_items; ++i) {
				gaze_datas.push_back(gaze_buffer[i]);
				eye_measurements_datas.push_back(eyeMeasurements_buffer[i]);
			}
		}

		return {gaze_datas, eye_measurements_datas};
	}

	varjo_Gaze EyeTrackingDataStreamer::getRenderingGazeData() const
	{
		varjo_Gaze rendering_gaze;
		varjo_GetRenderingGaze(*(this->session_), &rendering_gaze);
		return rendering_gaze;
	}

	std::pair<std::optional<double>, std::optional<double>> EyeTrackingDataStreamer::getIPDData() const
	{
		varjo_SyncProperties(*(this->session_));

		const double estimate = varjo_GetPropertyDouble(*(this->session_), varjo_PropertyKey_GazeIPDEstimate);
		const double positionInMM = varjo_GetPropertyDouble(*(this->session_), varjo_PropertyKey_IPDPosition);

		return {
			(estimate <= 0.0) ? std::nullopt : std::make_optional(estimate),
			(positionInMM <= 0.0) ? std::nullopt : std::make_optional(positionInMM)
		};
	}


}