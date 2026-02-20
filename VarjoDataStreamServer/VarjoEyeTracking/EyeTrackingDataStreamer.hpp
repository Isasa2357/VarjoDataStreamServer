#pragma once

#include <memory>
#include <optional>
#include <string>
#include <deque>
#include <array>

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

		std::pair<std::deque<varjo_Gaze>, std::deque<varjo_EyeMeasurements>> getGazeDataWithEyeMeasurements() const;

		varjo_Gaze getRenderingGazeData() const;

		std::pair<std::optional<double>, std::optional<double>> getIPDData() const;

	private:
		const std::shared_ptr<Session> session_;
		const OutputFilterType outputFilterType_;
		const OutputFrequency outputFrequency_;
	};

	struct EyeTrackingDataStreamerOptions {
		const std::shared_ptr<Session> session;
		const OutputFilterType outputFilterType;
		const OutputFrequency outputFrequency;
	};

	EyeTrackingDataStreamer make_EyeTrackingDataStreamer(const EyeTrackingDataStreamerOptions& opt);
	std::unique_ptr<EyeTrackingDataStreamer> make_EyeTrackingDataStreamerPtr(const EyeTrackingDataStreamerOptions& opt);
}