/*
 * sensor_uvc.cpp
 *
 *  Created on: 2017/02/17
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
#include "binutils.h"

#include "sensor_uvc.h"
#include "h264_decoder.h"
#include "video_stream.h"

namespace serenegiant {
namespace sensor {

using namespace serenegiant::media;

/*public*/
UVCSensor::UVCSensor(const char *uuid, const char *name)
:	Sensor(SENSOR_UVC, uuid, name),
	h264(NULL),
	h264_width(0), h264_height(0),
	need_wait_iframe(true),
	last_sequence(-1),
	ofs(),
	received_frames(0),
	error_frames(0),
	skipped_frames(0),
	received_bytes(0),
	start_time(0),
	frame_index(0),
	mp4_writer(NULL),
	video_stream_index(-1) {

	ENTER();

	EXIT();
}

/*virtual*/
/*public*/
UVCSensor::~UVCSensor() {

	ENTER();

	SAFE_DELETE(h264);
	SAFE_DELETE(mp4_writer);
	ofs.close();

	EXIT();
}

/*virtual*/
/*public*/
int UVCSensor::start(const char *command, const char *notify, const char *data) {
	int result = Sensor::start(command, notify, data);
	last_sequence = -1;
	received_frames = error_frames = skipped_frames = 0;
	received_bytes = 0;
	start_time = systemTime();
	frame_index = 0;
	return result;
}

/*virtual*/
/*protected*/
int UVCSensor::handle_notify_update(const std::string &identity, const std::string &payload) {
	ENTER();

	int result = -1;

	RETURN(result ,int);
}

/*virtual*/
/*protected*/
int UVCSensor::handle_frame_data(const std::string &identity,
	const publish_header_t &header, const size_t &size, const uint8_t *data) {

	ENTER();

	int result = -1;

	const uint32_t format = letoh32_unaligned(&header.uvc.format_le);
	const uint32_t width = letoh32_unaligned(&header.uvc.width_le);
	const uint32_t height = letoh32_unaligned(&header.uvc.height_le);
	const uint32_t sequence = letoh32_unaligned(&header.uvc.sequence_le);
	const double presentation_time_s = letoh64_unaligned(&header.uvc.presentation_time_s_le);
	const uint32_t data_bytes = letoh32_unaligned(&header.uvc.data_bytes_le);
	const bool skipped = (last_sequence + 1) != sequence;
	if (UNLIKELY(skipped)) {
		LOGW("frame skipped seq=%d,expect=%d", sequence, last_sequence + 1);
		skipped_frames += sequence - (last_sequence + 1);
	}
	if (LIKELY((size > 0) && (size == data_bytes))) {
		switch (format) {
		case VIDEO_FRAME_FORMAT_MJPEG:
			result = handle_frame_data_mjpeg(width, height, size, data, presentation_time_s);
			break;
		case VIDEO_FRAME_FORMAT_H264:
			need_wait_iframe |= skipped;
			result = handle_frame_data_h264(width, height, size, data, presentation_time_s);
			break;
		case VIDEO_FRAME_FORMAT_VP8:
			need_wait_iframe |= skipped;
			result = handle_frame_data_vp8(width, height, size, data, presentation_time_s);
			break;
		default:
			LOGW("unexpected frame format:%02x", format);
		}
		result = 0;
	} else {
		LOGW("data_bytes=%u, received=%lu", data_bytes, size);
	}
	last_sequence = sequence;
	received_frames++;
	error_frames += result ? 1 : 0;
	received_bytes += size;
	if (UNLIKELY(!(received_frames % 500))) {
		fprintf(stderr, "frames=%6u,err=%6u,skipped=%6u,bytes=%12lu,rate=%8.1fkB/s\n",
			received_frames, error_frames, skipped_frames, received_bytes,
			received_bytes / ((float)(systemTime() - start_time) / 1000000000.0f) / 1024 );
	}

	RETURN(result ,int);
}

/*protected*/
int UVCSensor::handle_frame_data_mjpeg(const uint32_t &width, const uint32_t &height,
	const size_t &size, const uint8_t *data, const double &presentation_time_s) {

	ENTER();

	int result = 0;

	if (UNLIKELY(h264)) {
		SAFE_DELETE(h264);
	}

	// FIXME not implemented, just write to file now.
	if (UNLIKELY(!ofs.is_open())) {
		ofs.open(uuid() + "dump.mjpeg", std::ios::binary | std::ios::out | std::ios::trunc);
	}
	ofs.write((const char *)data, size);

	// XXX if you need to save images as mp4, you need to decode mjpeg frames,
	// encode them into h.264 frames and pass them to Mp4Writer

	RETURN(result, int);
}

/*protected*/
int UVCSensor::handle_frame_data_h264(const uint32_t &width, const uint32_t &height,
	const size_t &size, const uint8_t *data, const double &presentation_time_s) {

	ENTER();

	int result = 0;

	int64_t presentation_time_us = (int64_t)(presentation_time_s * 1000000L);

	if (UNLIKELY((h264_width != width) || (h264_height != height))) {
		LOGI("video size changed, re-create decoder");
		SAFE_DELETE(h264);
		h264_width = width;
		h264_height = height;
	}
	if (UNLIKELY(!h264)) {
		h264 = new media::H264Decoder();
		need_wait_iframe = true;
	}

	if (UNLIKELY(!ofs.is_open())) {
		ofs.open(uuid() + "dump.h264", std::ios::binary | std::ios::out | std::ios::trunc);
	}
	ofs.write((const char *)data, size);

	const bool is_iframe = get_vop_type_annexb(data, size) == 0;
	if (need_wait_iframe) {
		LOGD("waiting I-frame");
		if (is_iframe /*is_iframe(data, size)*/) {
			LOGI("I-frame found");
			need_wait_iframe = false;
		} else {
			LOGI("frame dropped %s", bin2hex(data, 128).c_str());
			RETURN(0, int);
		}
	}
	if (LIKELY(h264 && h264->is_initialized())) {
		result = h264->set_input_buffer((uint8_t *)data, size, presentation_time_us);
		if (!result) {
			if (h264->is_frame_ready()) {
				writer_lock.lock();
				{
					if (mp4_writer) {
						if (UNLIKELY(!mp4_writer->isRunning() && is_iframe)) {
							video_stream_index = mp4_writer->add(
								new media::VideoStream(h264->get_context(), width, height, 15));
							mp4_writer->start();
						}
						if (video_stream_index >= 0) {
							mp4_writer->set_input_buffer(video_stream_index, (uint8_t *)data, size, frame_index);
							frame_index += 33333;	// you can pass any index value/intervals as you want
						}
					}
				}
				writer_lock.unlock();
				const size_t output_bytes = h264->get_output_bytes();
				if (UNLIKELY(output_bytes > h264_output.size())) {
					LOGI("resize %lu => %lu", h264_output.size(), output_bytes);
					h264_output.resize(output_bytes, 0);
				}
				if (LIKELY(output_bytes <= h264_output.size())) {
					int64_t result_pts;
					int bytes = h264->get_output_buffer(h264_output.data(), h264_output.size(), result_pts);
					if (LIKELY(bytes > 0)) {
						// success, do something
					} else {
						LOGE("H264Decoder::get_output_buffer failed");
					}
				} else {
					LOGE("failed to allocate output buffer,output_bytes=%lu,size=%lu", output_bytes, h264_output.size());
				}
			} else {
				LOGD("not ready");
			}
		} else {
			// when ffmpeg decoder failed
			LOGE("H264Decoder::set_input_buffer returned error %d\n%s", result, bin2hex(data, 128).c_str());
			need_wait_iframe = true;
		}
	}

	RETURN(result, int);
}

/*protected*/
int UVCSensor::handle_frame_data_vp8(const uint32_t &width, const uint32_t &height,
	const size_t &size, const uint8_t *data, const double &presentation_time_s) {

	ENTER();

	int result = 0;

	if (UNLIKELY(h264)) {
		SAFE_DELETE(h264);
	}

	// FIXME not implemented, just write to file now.
	if (UNLIKELY(!ofs.is_open())) {
		ofs.open(uuid() + "dump.vp8", std::ios::binary | std::ios::out | std::ios::trunc);
	}
	ofs.write((const char *)data, size);

	// XXX if you need to save images as mp4, you need to decode vp8 frames,
	// encode them into h.264 frames and pass them to Mp4Writer

	RETURN(result, int);
}

/*virtual*/
/*protected*/
int UVCSensor::internal_start_recording(const std::string &file_name) {

	ENTER();

	int result = 0;
	Mutex::Autolock lock(writer_lock);

	if (UNLIKELY(!mp4_writer)) {
		mp4_writer = new media::Mp4Writer(file_name);
		need_wait_iframe = true;
		video_stream_index = -1;
	}

	RETURN(result, int);
}

/*virtual*/
/*protected*/
void UVCSensor::internal_stop_recording() {

	ENTER();

	Mutex::Autolock lock(writer_lock);

	video_stream_index = -1;
	if (mp4_writer) {
		SAFE_DELETE(mp4_writer);
	}

	EXIT();
}

} /* namespace sensor */
} /* namespace serenegiant */
