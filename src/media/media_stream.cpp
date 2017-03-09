/*
 * media_stream.cpp
 *
 *  Created on: 2017/03/08
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

#include "utilbase.h"
#include "app_const.h"
#include "ffmpeg_utils.h"

#include "media_stream.h"

namespace serenegiant {
namespace media {

/*public*/
MediaStream::MediaStream()
:	stream(NULL),
	first_pts_us(0) {

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

	EXIT();
}

/*protected/friend*/
int MediaStream::init(AVFormatContext *format_context, const enum AVCodecID &codec_id) {

	ENTER();

	int result = -1;

	if (!stream) {
		first_pts_us = 0;
		stream = avformat_new_stream(format_context, NULL);
		if (LIKELY(stream)) {
			stream->id = format_context->nb_streams - 1;
			result = init_stream(format_context, codec_id, stream);
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
