#pragma once

#include <string>
#include <vector>
#include <fstream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

namespace vecutil {
	template<class T>
	void serialize_vector(const std::vector<T>& vec, std::string filename) {
		std::ofstream ofs(filename, std::ios::binary);
		boost::archive::binary_oarchive oa(ofs);
		oa << vec;
	}

	template<class T>
	std::vector<T> deserialize_vector(std::string filename) {
		std::vector<T> vec;
		std::ifstream ifs(filename, std::ios::binary);
		boost::archive::binary_iarchive ia(ifs);
		ia >> vec;
		return vec;
	}
}  // namespace vecutil