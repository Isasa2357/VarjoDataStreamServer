#pragma once

#include <filesystem>
#include <memory>

#include "../VarjoExample/Session.hpp"

#include "FrameInfo_types.hpp"

namespace VarjoFrameInfo {

	class DataLogger {

	public:
		DataLogger();

		~DataLogger();

		bool open_csvWriter();
		void close();
	};
}