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

VideoStream::VideoStream() {
	ENTER();

	EXIT();
}

VideoStream::~VideoStream() {
	ENTER();

	EXIT();
}

} /* namespace media */
} /* namespace serenegiant */
