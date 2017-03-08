/*
 * media_stream.cpp
 *
 *  Created on: 2017/03/08
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
#include "ffmpeg_utils.h"

#include "media_stream.h"

namespace serenegiant {
namespace media {

/*public*/
MediaStream::MediaStream(AVCodecContext *_codec_context)
:	codec_context(_codec_context),
	stream(NULL),
	next_pts(0),
	sample_count(0),
	frame(NULL),
	frame_tmp(NULL),
	t(0), tinc(0), tinc2(0),
	sws_context(NULL),
	swr_context(NULL) {

	ENTER();

	EXIT();
}

/*virtual*/
/*public*/
MediaStream::~MediaStream() {

	ENTER();

	release();

	EXIT();
}

/*virtual*/
/*public*/
void MediaStream::release() {

	ENTER();

	if (frame) {
		av_frame_free(&frame);
		frame = NULL;
	}
	if (frame_tmp) {
		av_frame_free(&frame_tmp);
		frame_tmp = NULL;
	}
	if (sws_context) {
		sws_freeContext(sws_context);
		sws_context = NULL;
	}
	if (swr_context) {
		swr_free(&swr_context);
		swr_context = NULL;
	}

	EXIT();
}

/*protected/friend*/
int MediaStream::init(AVFormatContext *format_context, const enum AVCodecID &codec_id) {

	ENTER();

	int result = -1;

	if (!stream) {
		stream = avformat_new_stream(format_context, NULL);
		if (LIKELY(stream)) {
			stream->id = format_context->nb_streams - 1;
			result = avcodec_parameters_from_context(stream->codecpar, codec_context);
			if (UNLIKELY(result < 0)) {
				LOGE("avcodec_parameters_from_context failed, err=%s", av_error(result).c_str());
			} else {
				result = init_stream(format_context, codec_id, stream);
			}
		} else {
			LOGE("avformat_new_stream failed, errno=%d", errno);
		}
	} else {
		LOGE("already initialized");
	}

	RETURN(result, int);
}

} /* namespace media */
} /* namespace serenegiant */
