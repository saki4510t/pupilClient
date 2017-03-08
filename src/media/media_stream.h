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

class Mp4Writer;

class MediaStream {
friend class Mp4Writer;
private:
	AVStream *stream;

	int64_t next_pts;
	int sample_count;

	AVFrame *frame;
	AVFrame *frame_tmp;

	float t, tinc, tinc2;

	struct SwsContext *sws_context;
	struct SwrContext *swr_context;
protected:
	/**
	 * initialize MediaStream
	 * @param format_context
	 * @param codec_id
	 * @return return >=0 if success otherwise return negative value
	 */
	int init(AVFormatContext *format_context, const enum AVCodecID &codec_id);
public:
	MediaStream();
	virtual ~MediaStream();
	virtual void release();
};

} /* namespace media */
} /* namespace serenegiant */

#endif /* MEDIA_STREAM_H_ */
