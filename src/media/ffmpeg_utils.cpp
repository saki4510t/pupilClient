/*
 * ffmpeg_utils.cpp
 *
 *  Created on: 2017/03/08
 *      Author: saki
 */

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
}

#include "utilbase.h"
#include "ffmpeg_utils.h"

namespace serenegiant {
namespace media {

std::string av_error(const int errnum) {
	char errbuf[AV_ERROR_MAX_STRING_SIZE + 1];
	av_strerror(errnum, errbuf, AV_ERROR_MAX_STRING_SIZE);
	errbuf[AV_ERROR_MAX_STRING_SIZE] = '\0';
	return std::string(errbuf);
}

}	// namespace media
}	// namespace serenegiant
