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
protected:
	virtual int init_stream(AVFormatContext *format_context,
		const enum AVCodecID &codec_id, AVStream *stream);
public:
	VideoStream(AVCodecContext *codec_context);
	virtual ~VideoStream();
};

} /* namespace media */
} /* namespace serenegiant */

#endif /* VIDEO_STREAM_H_ */
