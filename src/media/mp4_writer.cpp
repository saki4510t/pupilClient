/*
 * mp4_writer.cpp
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

#include "media_stream.h"
#include "video_stream.h"
#include "mp4_writer.h"

namespace serenegiant {
namespace media {

/*public*/
Mp4Writer::Mp4Writer(const std::string &_file_name)
:	file_name(_file_name),
	format_context(NULL),
	format(NULL),
	option(NULL) {

	ENTER();

	int result = avformat_alloc_output_context2(&format_context, NULL, NULL, file_name.c_str());
	if (UNLIKELY(result < 0)) {
		LOGW("failed to deduce output format from file extension, try mp4");
		if (format_context) {
			avformat_free_context(format_context);
			format_context = NULL;
		}
		result = avformat_alloc_output_context2(&format_context, NULL, "mp4", file_name.c_str());
	}

	if (UNLIKELY(format_context)) {
		LOGE("failed to create format context, result=%d", result);
	}

	EXIT();
}

/*virtual*/
/*public*/
Mp4Writer::~Mp4Writer() {
	ENTER();

	release();

	EXIT();
}

/*virtual*/
/*public*/
void Mp4Writer::release() {
	ENTER();

	free_streams();
	if (format_context) {
		if (format && !(format->flags & AVFMT_NOFILE)) {
			avio_closep(&format_context->pb);
		}
		avformat_free_context(format_context);
		format_context = NULL;
		format = NULL;
	}

	EXIT();
}

/*virtual*/
/*public*/
int Mp4Writer::add(MediaStream *stream) {

	ENTER();

	int result = -1;
	if (UNLIKELY(!format_context)) {
		LOGE("format context is null, already released?");
		goto ret;
	}
	if (LIKELY(stream)) {
		int ix = find_stream(stream);
		if (LIKELY(ix < 0)) {
			enum AVCodecID codec_id = AV_CODEC_ID_NONE;
			if (dynamic_cast<VideoStream *>(stream)) {
				codec_id = format->video_codec;
			} else {
				LOGW("unknown MediaStream");
			}
			result = stream->init(format_context, codec_id);
			if (result >= 0) {
				streams.push_back(stream);
			} else {
				SAFE_DELETE(stream);
			}
		} else {
			LOGW("specific stream was already added");
			result = ix;
		}
	}
ret:
	RETURN(result, int);
}

/*virtual*/
/*public*/
int Mp4Writer::start() {
	ENTER();

	int result = -1;
	if (UNLIKELY(!format_context)) {
		LOGE("format context is null, already released?");
		goto ret;
	}
	if (LIKELY(!streams.empty())) {
		// FIXME do something
	} else {
		LOGE("cound not start because no MediaStream were added");
	}
ret:
	RETURN(result, int);
}

/*virtual*/
/*public*/
void Mp4Writer::stop() {
	ENTER();

	if (LIKELY(format_context) && !streams.empty()) {
		// FIXME do something
		free_streams();
	}

	EXIT();
}

/*private*/
int Mp4Writer::find_stream(const MediaStream *stream) {

	ENTER();

	int result = -1, ix = 0;
	if (!streams.empty()) {
		for (auto itr: streams) {
			if (stream == itr) {
				result = ix;
				break;
			}
			ix++;
		}
	}

	RETURN(result, int);
}

/*private*/
void Mp4Writer::free_streams() {
	ENTER();

	if (!streams.empty()) {
		for (auto itr: streams) {
			MediaStream *stream = itr;
			SAFE_DELETE(stream);
		}
		streams.clear();
		streams.shrink_to_fit();
	}

	EXIT();
}

/*private*/
MediaStream *Mp4Writer::get_stream(const int &index) {
	ENTER();

	MediaStream *result = NULL;

	if ((index >= 0) && (index < (int)streams.size())) {
		result = streams[index];
	}

	RET(result);
}

} /* namespace media */
} /* namespace serenegiant */
