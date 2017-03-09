/*
 * video_stream.h
 *
 *  Created on: 2017/03/08
 *      Author: saki
 */

#ifndef VIDEO_STREAM_H_
#define VIDEO_STREAM_H_

#include "media_stream.h"

namespace serenegiant {
namespace media {

class VideoStream: public virtual MediaStream {
private:
	const AVCodecContext *codec_context;
	const uint32_t width, height;
protected:
	virtual int init_stream(AVFormatContext *format_context,
		const enum AVCodecID &codec_id, AVStream *stream);
public:
	VideoStream(const AVCodecContext *codec_context, const uint32_t &width, const uint32_t &height);
	virtual ~VideoStream();
	virtual const stream_type_t stream_type() { return STREAM_VIDEO; };
};

} /* namespace media */
} /* namespace serenegiant */

#endif /* VIDEO_STREAM_H_ */
