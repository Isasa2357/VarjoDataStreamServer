#pragma once

#include <vector>
#include <span>
#include <cstdint>

#include "varjo_vst_frame_type.hpp"

namespace VarjoVSTFrame {

	/**
	 * @brief VSTフレームのパディングを削除する
	 * @param raw_frameData DataStream APIから取得した生のVSTフレーム
	 * @param out_frameData パディングが除去されたデータの出力先
	 * @param width パディング除去後のフレームの幅
	 * @param height フレームの高さ
	 * @param row_stride パディング除去前のフレームの幅
	 */
	inline void remove_padding(
		const std::vector<uint8_t>& raw_frameData,
		std::vector<uint8_t>& out_frameData,
		const size_t width,
		const size_t height,
		const size_t row_stride
	)
	{
		const size_t frameSize_withoutPadding = width * height;
		const size_t y_plane_size = row_stride * height;
		const size_t uv_plane_size = row_stride * (height / 2);
		const std::span<const uint8_t> y_plane(raw_frameData.data(), y_plane_size);
		const std::span<const uint8_t> uv_plane(raw_frameData.data() + y_plane_size, uv_plane_size);
		// Yプレーンのコピー
		for (auto i = 0; i < height; ++i) {
			memcpy(
				out_frameData.data() + i * width,
				y_plane.data() + i * row_stride,
				width
			);
		}
		// UVプレーンのコピー
		for (auto i = 0; i < height / 2; ++i) {
			memcpy(
				out_frameData.data() + frameSize_withoutPadding + i * width,
				uv_plane.data() + i * row_stride,
				width
			);
		}
	}

	/**
	 * @brief VSTフレームのパディングを削除する
	 * @param raw_frameData DataStream APIから取得した生のVSTフレーム
	 * @param width パディング除去後のフレームの幅
	 * @param height フレームの高さ
	 * @param row_stride パディング除去前のフレームの幅
	 * @return パディング除去後のフレーム
	 */
	inline std::vector<uint8_t> remove_padding(
		const std::vector<uint8_t>& raw_frameData,
		const size_t width,
		const size_t height,
		const size_t row_stride
	) {
		std::vector<uint8_t> out_frameData(width * height + (width * height) / 2);
		remove_padding(raw_frameData, out_frameData, width, height, row_stride);
		return out_frameData;
	}

	inline std::string videoContainer_toString(const VideoContainer container) {
		switch (container) {
		case VideoContainer::mp4:
			return "mp4";
		case VideoContainer::mkv:
			return "mkv";
		default:
			return "mp4";
		}
	}

	//------------------------------ codec option factory

	/**
	 * @brief x264エンコードオプションを作るヘルパ関数
	 * @param preset presetオプション．動画の圧縮率と処理速度の調整を行える
	 * @param mode crf固定かqp固定かの選択．基本的にcrfを指定すればok
	 * @param crf mode=crf時に有効．crf値を設定．小さいほど高精度．
	 * @param qp mode=qp時に有効．小さいほど高精度
	 * @return x264オプション
	 */
	X264Options make_X264Options(
		const X264Options::X264Preset preset, 
		const X264Options::Mode mode = X264Options::Mode::Crf,
		const int crf = 19, 
		const int qp = 0
	) {
		X264Options opt;
		
		opt.preset = preset;
		opt.mode = mode;
		opt.crf = crf;
		opt.qp = qp;

		return opt;
	}

	/**
	 * @brief x264エンコードオプションを品質を指定して作るヘルパ関数．詳細な設定はできない
	 * @param quality 品質
	 * @return x264オプション
	 */
	X264Options make_X264Options(const Quality quality) {
		X264Options opt;
		opt.preset = X264Options::X264Preset::Veryfast;

		opt.mode = X264Options::Mode::Crf;

		switch (quality) {
		case Quality::Lossless:
			opt.crf = 0;  
			break;
		case Quality::High:
			opt.crf = 18;
			break;
		case Quality::Medium:
			opt.crf = 23;
			break;
		case Quality::Low:
			opt.crf = 28;
			break;
		default:
			opt.crf = 19;
			break;
		}

		opt.qp = 0;
		return opt;
	}

	/**
	 * @brief nvench264エンコードオプションを作るヘルパ関数
	 * @param preset presetオプション．動画の圧縮率と処理速度の調整を行える
	 * @param rc レート制御モード設定．理由がなければVbrHqでok
	 * @param cq rc=VbarHq時に有効．
	 * @param qp 
	 * @param spatial_aq 
	 * @param temporal_aq
	 */
	NvencH264Options make_NvencH264Options(
		const NvencH264Options::NvencPreset preset,
		const NvencH264Options::NvencRc rc = NvencH264Options::NvencRc::VbrHq,
		const int cq = 19,
		const int qp = 0,
		const bool spatial_aq = false,
		const bool temporal_aq = false
	) {
		NvencH264Options opt;

		opt.preset = preset;
		opt.rc = rc;
		opt.cq = cq;
		opt.qp = qp;
		opt.spatial_aq = spatial_aq;
		opt.temporal_aq = temporal_aq;

		return opt;
	}

	/**
	 * @brief nvenc_h264エンコードオプションを品質を指定して作るヘルパ関数．詳細な設定はできない
	 * @param quality 品質
	 * @return nvenc_h264オプション
	 */
	NvencH264Options make_NvencH264Options(const Quality quality) {
		NvencH264Options opt;
		opt.preset = NvencH264Options::NvencPreset::P1;
		opt.spatial_aq = false;
		opt.temporal_aq = false;

		switch (quality) {
		case Quality::Lossless:
			opt.rc = NvencH264Options::NvencRc::ConstQp;
			opt.qp = 0;
			opt.cq = 0;
			break;
		case Quality::High:
			opt.rc = NvencH264Options::NvencRc::VbrHq;
			opt.cq = 19;
			opt.qp = 0;
			break;
		case Quality::Medium:
			opt.rc = NvencH264Options::NvencRc::VbrHq;
			opt.cq = 23;
			opt.qp = 0;
			break;
		case Quality::Low:
			opt.rc = NvencH264Options::NvencRc::VbrHq;
			opt.cq = 28;
			opt.qp = 0;
			break;
		default:
			opt.rc = NvencH264Options::NvencRc::VbrHq;
			opt.cq = 19;
			opt.qp = 0;
			break;
		}

		return opt;
	}

	/**
	 * @brief FFV1エンコードオプションを作るヘルパ関数
	 */
	Ffv1Options make_Ffv1Options(
		const int level
	) {
		Ffv1Options opt;

		opt.level = level;

		return opt;
	}

	/**
	 * @brief FFV1エンコードオプションを品質を指定して作るヘルパ関数．FFV1は品質の調整はできないので，意味はない．
	 */
	Ffv1Options make_Ffv1Options(const Quality quality)
	{
		Ffv1Options opt;
		opt.level = 3; 
		return opt;
	}
}