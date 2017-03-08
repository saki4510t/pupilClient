/*
 * video_stream.cpp
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

#include "video_stream.h"

namespace serenegiant {
namespace media {

#define STREAM_FRAME_RATE	25	/* 25 frames/s */

VideoStream::VideoStream(AVCodecContext *codec_context)
:	MediaStream(codec_context) {

	ENTER();

	EXIT();
}

VideoStream::~VideoStream() {

	ENTER();

	EXIT();
}

int VideoStream::init_stream(AVFormatContext *format_context,
	const enum AVCodecID &codec_id, AVStream *stream) {

	ENTER();

	// FIXME provide actual frame rate
	stream->time_base = (AVRational) {1, STREAM_FRAME_RATE };

	RETURN(0 ,int);
}

} /* namespace media */
} /* namespace serenegiant */
