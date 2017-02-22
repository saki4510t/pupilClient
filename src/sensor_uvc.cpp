/*
 * sensor_uvc.cpp
 *
 *  Created on: 2017/02/17
 *      Author: saki
 */

#if 1	// set 0 if you need debug log, otherwise set 1
	#ifndef LOG_NDEBUG
		#define LOG_NDEBUG
	#endif
	#undef USE_LOGALL
#else
//	#define USE_LOGALL
	#undef LOG_NDEBUG
	#undef NDEBUG
#endif

#include "utilbase.h"
#include "binutils.h"

#include "sensor_uvc.h"
#include "h264_decoder.h"

namespace serenegiant {
namespace sensor {

/*public*/
UVCSensor::UVCSensor(const char *uuid, const char *name)
:	Sensor(SENSOR_UVC, uuid, name),
	h264(NULL),
	h264_width(0), h264_height(0),
	need_wait_iframe(true),
	last_sequence(-1),
	ofs() {

	ENTER();

	EXIT();
}

/*virtual*/
/*public*/
UVCSensor::~UVCSensor() {

	ENTER();

	SAFE_DELETE(h264);
	ofs.close();

	EXIT();
}

/*virtual*/
/*public*/
int UVCSensor::start(const char *command, const char *notify, const char *data) {
	int result = Sensor::start(command, notify, data);
	last_sequence = -1;
	return result;
}

/*virtual*/
/*protected*/
int UVCSensor::handle_notify_update(const std::string &identity, const std::string &payload) {
	ENTER();

	int result = -1;

	RETURN(result ,int);
}

/*virtual*/
/*protected*/
int UVCSensor::handle_frame_data(const std::string &identity,
	const publish_header_t &header, const size_t &size, const uint8_t *data) {

	ENTER();

	int result = -1;

	const uint32_t format = letoh32_unaligned(&header.uvc.format_le);
	const uint32_t width = letoh32_unaligned(&header.uvc.width_le);
	const uint32_t height = letoh32_unaligned(&header.uvc.height_le);
	const uint32_t sequence = letoh32_unaligned(&header.uvc.sequence_le);
	const int64_t presentation_time_us = letoh64_unaligned((const uint64_t *)&header.uvc.presentation_time_us_le);
	const uint32_t data_bytes = letoh32_unaligned(&header.uvc.data_bytes_le);
	const bool skipped = (last_sequence + 1) != sequence;
	if (UNLIKELY(skipped)) {
		LOGW("frame skipped seq=%d,expect=%d", sequence, last_sequence + 1);
	}
	if (LIKELY((size > 0) && (size == data_bytes))) {
		switch (format) {
		case VIDEO_FRAME_FORMAT_MJPEG:
			result = handle_frame_data_mjpeg(width, height, size, data, presentation_time_us);
			break;
		case VIDEO_FRAME_FORMAT_H264:
			need_wait_iframe |= skipped;
			result = handle_frame_data_h264(width, height, size, data, presentation_time_us);
			break;
		case VIDEO_FRAME_FORMAT_VP8:
			need_wait_iframe |= skipped;
			result = handle_frame_data_vp8(width, height, size, data, presentation_time_us);
			break;
		default:
			LOGW("unexpected frame format:%02x", format);
		}
		result = 0;
	} else {
		LOGW("data_bytes=%u, received=%lu", data_bytes, size);
	}
	last_sequence = sequence;

	RETURN(result ,int);
}

int UVCSensor::handle_frame_data_mjpeg(const uint32_t &width, const uint32_t &height,
	const size_t &size, const uint8_t *data, const int64_t &presentation_time_us) {

	ENTER();

	int result = 0;

	SAFE_DELETE(h264);
	// FIXME 未実装
	if (UNLIKELY(!ofs.is_open())) {
		ofs.open("dump.mjpeg", std::ios::binary | std::ios::out | std::ios::trunc);
	}
	ofs.write((const char *)data, size);

	RETURN(result, int);
}

int UVCSensor::handle_frame_data_h264(const uint32_t &width, const uint32_t &height,
	const size_t &size, const uint8_t *data, const int64_t &presentation_time_us) {

	ENTER();

	int result = 0;

	if (UNLIKELY((h264_width != width) || (h264_height != height))) {
		LOGI("video size changed, re-create decoder");
		SAFE_DELETE(h264);
		h264_width = width;
		h264_height = height;
	}
	if (UNLIKELY(!h264)) {
		h264 = new media::H264Decoder();
		need_wait_iframe = true;
	}

	if (UNLIKELY(!ofs.is_open())) {
		ofs.open("dump.h264", std::ios::binary | std::ios::out | std::ios::trunc);
	}
	ofs.write((const char *)data, size);

	if (need_wait_iframe) {
		LOGD("waiting I-frame");
		if (is_iframe(size, data)) {
			LOGI("I-frame found");
			need_wait_iframe = false;
		} else {
			LOGI("frame dropped %s", bin2hex(data, 128).c_str());
			RETURN(0, int);
		}
	}
	if (LIKELY(h264 && h264->is_initialized())) {
		result = h264->set_input_buffer((uint8_t *)data, size, presentation_time_us);
		if (!result) {
			if (h264->is_frame_ready()) {
				const size_t output_bytes = h264->get_output_bytes();
				if (UNLIKELY(output_bytes > h264_output.size())) {
					LOGI("resize %lu => %lu", h264_output.size(), output_bytes);
					h264_output.resize(output_bytes, 0);
				}
				if (LIKELY(output_bytes <= h264_output.size())) {
					int64_t result_pts;
					int bytes = h264->get_output_buffer(h264_output.data(), h264_output.size(), result_pts);
					if (LIKELY(bytes > 0)) {
						// success, do something
					} else {
						LOGE("H264Decoder::get_output_buffer failed");
					}
				} else {
					LOGE("failed to allocate output buffer,output_bytes=%lu,size=%lu", output_bytes, h264_output.size());
				}
			} else {
				LOGD("not ready");
			}
		} else {
			// when ffmpeg decoder failed
			LOGE("H264Decoder::set_input_buffer returned error %d\n%s", result, bin2hex(data, 128).c_str());
			need_wait_iframe = true;
		}
	}

	RETURN(result, int);
}

int UVCSensor::handle_frame_data_vp8(const uint32_t &width, const uint32_t &height,
	const size_t &size, const uint8_t *data, const int64_t &presentation_time_us) {

	ENTER();

	int result = 0;

	SAFE_DELETE(h264);
	// FIXME 未実装
	if (UNLIKELY(!ofs.is_open())) {
		ofs.open("dump.vp8", std::ios::binary | std::ios::out | std::ios::trunc);
	}
	ofs.write((const char *)data, size);

	RETURN(result, int);
}

/**
 * AnnexBマーカー(N[00] 00 00 01, N>=0)で始まるかどうかをチェックする
 * OKなら0, NGなら-1を返す
 * payloadがNULLでなければannexbマーカーの次の位置を*payloadにセットする
 */
static int find_annexb(const uint8_t *data, const size_t &len, const uint8_t **payload) {
	ENTER();

	if (payload) {
		*payload = NULL;
	}
	for (size_t i = 0; i < len - 4; i++) {	// 本当はlen-3までだけどpayloadが無いのは無効とみなしてlen-4までとする
		// 最低2つは連続して0x00でないとだめ
		if ((data[0] != 0x00) || (data[1] != 0x00)) {
			data++;
			continue;
		}
		// 3つ目が0x01ならOK
		if (data[2] == 0x01) {
			if (payload) {
				*payload = data + 3;
			}
			RETURN(0, int);
		}
		data++;
	}

	RETURN(-1, int);
}

/**
 * I-Frameかどうかをチェック
 * @return I-Frameならtrue
 * */
const bool UVCSensor::is_iframe(const size_t &size, const uint8_t *_data) {
	ENTER();

	bool result = false;

	if (LIKELY(size > 3)) {
		const uint8_t *data = _data;
		const uint8_t *payload = NULL;
		int sz = size;
		LOGD("annexbマーカーを探す");
		int ret = find_annexb(data, sz, &payload);
		if (LIKELY(!ret)) {
			LOGV("annexbマーカーが見つかった");
			bool sps = false, pps = false;
			int ix = payload - data;
			sz -= ix;
			for (uint32_t i = ix; i < size; i++) {
				const nal_unit_type_t type = (nal_unit_type_t)(payload[0] & 0x1f);
				switch (type) {
				case NAL_UNIT_CODEC_SLICE_IDR: // MediaCodecのh.264エンコーダーからの出力はこれが来る...でもffmpegは認識しない
					LOGD("IFrameが見つかった");
//					LOGI("ペイロード:%s", bin2hex(&data[0], 128).c_str());
//					LOGD("SPS/PPS?:%s", bin2hex(&payload[0], 128).c_str());
					result = true;
					goto end;
//					break;
				case NAL_UNIT_SEQUENCE_PARAM_SET:
				{
					LOGD("SPSが見つかった...次のannexbマーカーを探す");
//					LOGI("ペイロード:%s", bin2hex(&data[0], 128).c_str());
//					LOGI("SPS:%s", bin2hex(&payload[0], 128).c_str());
					sps = true;
					ret = find_annexb(&payload[1], sz - 1, &payload);
					if (LIKELY(!ret)) {
						i = payload - data;
						sz = size - i;
					} else {
						goto end;
					}
					break;
				}
				case NAL_UNIT_PICTURE_PARAM_SET:
				{
					if (LIKELY(sps)) {
						LOGD("PPSが見つかった...次のannexbマーカーを探す");
//						LOGI("ペイロード:%s", bin2hex(&data[0], 128).c_str());
//						LOGI("PPS:%s", bin2hex(&payload[0], 128).c_str());
						pps = true;
						ret = find_annexb(&payload[1], sz  -1, &payload);
						if (LIKELY(!ret)) {
							i = payload - data;
							sz = size - i;
						} else {
							goto end;
						}
					}
					break;
				}
				case NAL_UNIT_PICTURE_DELIMITER:
				{
					LOGD("IFrameじゃないけど1フレームを生成できるNALユニットの集まりの区切り");
					result = true;
					goto end;
				}
				case NAL_UNIT_UNSPECIFIED:
				{
					ret = find_annexb(&payload[1], sz  -1, &payload);
					if (LIKELY(!ret)) {
						i = payload - data;
						sz = size - i;
					} else {
						goto end;
					}
					break;
				}
				default:
					// 何かAnnexbマーカーで始まるpayloadの時, SPS+PPSが見つかっていればIFrameとする
					LOGV("type=%x", type);
					result = sps && pps;
					goto end;
				} // end of switch (type)
			} // end of for
end:
			result = sps && pps;
		} else {
			LOGD("annexbマーカーが見つからなかった");
		}
	} else {
		LOGW("短すぎる");
	}
	RETURN(result, bool);
}

} /* namespace sensor */
} /* namespace serenegiant */
