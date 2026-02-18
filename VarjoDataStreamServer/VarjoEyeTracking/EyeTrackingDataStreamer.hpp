#pragma once

#include <memory>
#include <optional>
#include <string>
#include <deque>

#include "../VarjoExample/Session.hpp"
#include "EyeTracking_types.hpp"

namespace VarjoEyeTracking {

	class EyeTrackingDataStreamer {

	public:
		EyeTrackingDataStreamer(
			const std::shared_ptr<Session>& session, 
			const OutputFilterType outputFilterType, 
			const OutputFrequency outputFrequency
		);

		GazeTrackingStatus getStatus() const;

		std::deque<EyeTrackingData> geteyeTrackingData() const;

	private:

		void initializeGazeTracking(const OutputFilterType outputFilterType, const OutputFrequency outputFrequency) const;

		std::pair<std::vector<varjo_Gaze>, std::vector<varjo_EyeMeasurements>> getGazeDataWithEyeMeasurements() const;

		std::vector<varjo_Gaze> getRenderingGazeData() const;

		std::pair<std::optional<double>, std::optional<double>> getIPDData() const;

	private:
		const std::shared_ptr<Session> m_session;
		const OutputFilterType outputFilterType_;
		const OutputFrequency outputFrequency_;
	};
}