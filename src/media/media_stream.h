/*
 * media_stream.h
 *
 *  Created on: 2017/03/08
 *      Author: saki
 */

#ifndef MEDIA_STREAM_H_
#define MEDIA_STREAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdarg.h>

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>

	#include <libavutil/avassert.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/channel_layout.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/timestamp.h>
}

namespace serenegiant {
namespace media {

typedef enum stream_type {
	STREAM_UNKNOWN = 0,
	STREAM_VIDEO,
	STREAM_AUDIO,
} stream_type_t;

class Mp4Writer;

class MediaStream {
friend class Mp4Writer;
private:
	AVStream *stream;
	int64_t first_pts_us;

protected:
	/**
	 * initialize MediaStream
	 * @param format_context
	 * @param codec_id
	 * @return return >=0 if success otherwise return negative value
	 */
	int init(AVFormatContext *format_context, const enum AVCodecID &codec_id);
	virtual int init_stream(AVFormatContext *format_context,
		const enum AVCodecID &codec_id, AVStream *stream) = 0;
public:
	MediaStream();
	virtual ~MediaStream();
	virtual void release();
	virtual const stream_type_t stream_type() = 0;
};

} /* namespace media */
} /* namespace serenegiant */

#endif /* MEDIA_STREAM_H_ */
