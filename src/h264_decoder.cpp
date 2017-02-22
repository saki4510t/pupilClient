/*
 * h264_decoder.cpp
 *
 *  Created on: 2017/01/26
 *      Author: saki
 */

#if 0	// set 0 if you need debug log, otherwise set 1
	#ifndef LOG_NDEBUG
		#define LOG_NDEBUG
	#endif
	#undef USE_LOGALL
#else
//	#define USE_LOGALL
	#undef LOG_NDEBUG
	#undef NDEBUG
#endif

#include <string>

#include "utilbase.h"

#include "h264_decoder.h"

namespace serenegiant {
namespace media {

#define USE_NEW_AVCODEC_API 0

static std::string av_error(const int errnum) {
	char errbuf[AV_ERROR_MAX_STRING_SIZE + 1];
	av_strerror(errnum, errbuf, AV_ERROR_MAX_STRING_SIZE);
	errbuf[AV_ERROR_MAX_STRING_SIZE] = '\0';
	return std::string(errbuf);
}

H264Decoder::H264Decoder(const color_format_t &_color_format)
:	color_format(AV_PIX_FMT_YUV420P),
	codec_context(NULL),
	src(NULL), dst(NULL),
	sws_context(NULL),
	frame_ready(false)
{
	ENTER();

	switch (_color_format) {
	case COLOR_FORMAT_YUV420:
		color_format = AV_PIX_FMT_YUV420P;
		break;
	case COLOR_FORMAT_RGB565LE:
		color_format = AV_PIX_FMT_RGB565LE;
		break;
	case COLOR_FORMAT_BGR32:
		color_format = AV_PIX_FMT_BGR32;
		break;
	default:
		color_format = AV_PIX_FMT_YUV420P;
		break;
	}

	struct AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (LIKELY(codec)) {
		codec_context = avcodec_alloc_context3(codec);
		if (LIKELY(codec_context)) {
			codec_context->pix_fmt = color_format;
			codec_context->flags2 |= CODEC_FLAG2_CHUNKS;
			if (codec->capabilities & CODEC_CAP_TRUNCATED) {
				codec_context->flags |= CODEC_FLAG_TRUNCATED;
			}
			int result = avcodec_open2(codec_context, codec, NULL);
			if (LIKELY(!result)) {
				src = av_frame_alloc();
				dst = av_frame_alloc();
			} else {
				LOGE("avcodec_open2 failed with error %d:%s", result, av_error(result).c_str());
				avcodec_close(codec_context);
				av_free(codec_context);
				codec_context = NULL;
			}
		}
	}

	EXIT();
}

H264Decoder::~H264Decoder() {

	ENTER();

	if (codec_context) {
		avcodec_close(codec_context);
		av_free(codec_context);
		codec_context = NULL;
	}
	if (src) {
		av_free(src);
		src = NULL;
	}
	if (dst) {
		av_free(dst);
		dst = NULL;
	}

	EXIT();
}

int H264Decoder::set_input_buffer(uint8_t *nal_units, const size_t &bytes, const int64_t &presentation_time_us) {

	ENTER();

	int result = -1;

	if (UNLIKELY(!is_initialized())) RETURN(result, int);

	AVPacket packet;

	av_init_packet(&packet);
	packet.data = nal_units;
	packet.size = bytes;
	packet.pts = presentation_time_us;

#if USE_NEW_AVCODEC_API
	result = avcodec_send_packet(codec_context, &packet);
	if (!result) {
		for ( ; !result ; ) {
			result = avcodec_receive_frame(codec_context, src);
			if (!result) {
				LOGI("got frame");
				frame_ready = true;
				// FIXME avcodec_send_packetは複数フレームを生成する可能性があるので
				// 今のテスト実装だとフレームがドロップする
				// 必要ならデコードしたフレームデータをコピーしてキューに入れること
			} else if ((result < 0) && (result != AVERROR(EAGAIN)) && (result != AVERROR_EOF)) {
				LOGE("avcodec_receive_frame returned error %d:%s", result, av_error(result).c_str());
			} else {
				// まだ準備出来てない時?
				switch (result) {
				case AVERROR(EAGAIN):
					LOGV("avcodec_receive_frame EAGAIN");
					result = 0;
					goto ret;
				case AVERROR_EOF:
					LOGV("avcodec_receive_frame AVERROR_EOF");
					result = 0;
					goto ret;
				default:
					LOGE("avcodec_receive_frame returned error %d:%s", result, av_error(result).c_str());
					break;
				}
			}
		}
	} else {
		switch (result) {
		case AVERROR(EAGAIN):
			LOGE("avcodec_send_packet EAGAIN");
			break;
		case AVERROR_EOF:
			LOGE("avcodec_send_packet AVERROR_EOF");
			result = 0;
			break;
		default:
			LOGE("avcodec_send_packet returned error %d:%s", result, av_error(result).c_str());
			break;
		}
	}
#else
	int frame_finished = 0;
	result = avcodec_decode_video2(codec_context, src, &frame_finished, &packet);
	if ((result > 0) && frame_finished) {
		LOGI("got frame");
		frame_ready = true;
		result = 0;
	}
#endif
ret:
	RETURN(result, int);
}

int H264Decoder::get_output_buffer(uint8_t *buf, const size_t &capacity, int64_t &result_pts) {

	ENTER();

	if (UNLIKELY(!is_initialized())) RETURN(-1, int);

	result_pts = AV_NOPTS_VALUE;
	size_t result = get_output_bytes();

	if (LIKELY(capacity >= result)) {
		if (color_format == codec_context->pix_fmt) {
			memcpy(src->data, buf, result);
		} else {
			const int width = this->width();
			const int height = this->height();
			if (UNLIKELY(!sws_context)) {
				sws_context = sws_getContext(width, height, codec_context->pix_fmt,
					width, height, color_format, SWS_FAST_BILINEAR, NULL, NULL, NULL);
			}
			avpicture_fill((AVPicture *)dst, buf, color_format, width, height);
			sws_scale(sws_context, (const uint8_t **)src->data, src->linesize, 0, height,
				dst->data, dst->linesize);
		}
		frame_ready = false;
		result_pts = src->pkt_pts;
		if (UNLIKELY(result_pts == AV_NOPTS_VALUE)) {
			LOGW("No PTS");
		}
	} else {
		LOGE("capacity is smaller than required");
		result = -1;
	}

	RETURN(result, int);
}

}	// namespace media
}	// namespace serenegiant
