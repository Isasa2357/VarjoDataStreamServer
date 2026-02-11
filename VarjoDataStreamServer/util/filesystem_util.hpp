#pragma once

#include <string>
#include <filesystem>

namespace FilesystemUtil{

	/**
	 * @brief ファイル名の衝突を回避する．ファイル名が衝突した場合，連番を付与した新しいファイル名を生成する．
	 */
	std::string solve_filename_conflict(const std::string& original_path) {
		if (!std::filesystem::exists(original_path)) {
			// ファイル名が衝突していない場合はそのまま返す
			return std::string(original_path);
		} else {
			int sequence_num = 1;
			std::filesystem::path path_obj(original_path);
			std::filesystem::path parent_path = path_obj.parent_path();
			std::filesystem::path stem = path_obj.stem();
			std::filesystem::path extension = path_obj.extension();
			std::filesystem::path candidate_path = parent_path / (stem.string() + "_" + std::to_string(sequence_num) + extension.string());

			while (std::filesystem::exists(candidate_path)) {
				sequence_num += 1;
				candidate_path = parent_path / (stem.string() + "_" + std::to_string(sequence_num) + extension.string());
			}
			return candidate_path.string();
		}
	}
};