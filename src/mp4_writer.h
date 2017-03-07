/*
 * mp4_writer.h
 *
 *  Created on: 2017/03/08
 *      Author: saki
 */

#ifndef MP4_WRITER_H_
#define MP4_WRITER_H_

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

class MediaStream;

class Mp4Writer {
private:
protected:
	bool write_video_frame(AVFormatContext *format_context, MediaStream &stream, AVPacket &pkt, const long &presentation_time_us);
	void close_stream(AVFormatContext *format_context, MediaStream &stream);
public:
	Mp4Writer();
	virtual ~Mp4Writer();
};

} /* namespace media */
} /* namespace serenegiant */

#endif /* MP4_WRITER_H_ */
