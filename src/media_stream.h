/*
 * media_stream.h
 *
 *  Created on: 2017/03/08
 *      Author: saki
 */

#ifndef MEDIA_STREAM_H_
#define MEDIA_STREAM_H_

#define USE_NEW_AVCODEC_API 1

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdarg.h>

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
#if USE_NEW_AVCODEC_API
	#include <libavutil/imgutils.h>
#endif
}

namespace serenegiant {
namespace media {

class MediaStream {
public:
	MediaStream();
	virtual ~MediaStream();
};

} /* namespace media */
} /* namespace serenegiant */

#endif /* MEDIA_STREAM_H_ */
