/*
 * sensor_uvc.h
 *
 *  Created on: 2017/02/17
 *      Author: saki
 */

#ifndef SENSOR_UVC_H_
#define SENSOR_UVC_H_

#include <vector>
#include <iostream>
#include <fstream>

#include "sensor.h"
#include "h264_utils.h"
#include "h264_decoder.h"
#include "mp4_writer.h"

namespace serenegiant {
namespace sensor {


class UVCSensor: public virtual Sensor {
private:
	media::H264Decoder *h264;
	std::vector<uint8_t> h264_output;
	uint32_t h264_width, h264_height;
	bool need_wait_iframe;
	uint32_t last_sequence;
	std::ofstream ofs;
	uint32_t received_frames;
	uint32_t error_frames;
	uint32_t skipped_frames;
	size_t received_bytes;
	nsecs_t start_time;

	mutable Mutex writer_lock;
	media::Mp4Writer *mp4_writer;
	int video_stream_index;
protected:
	virtual int handle_notify_update(const std::string &identity, const std::string &payload);
	virtual int handle_frame_data(const std::string &identity,
		const publish_header_t &header, const size_t &size, const uint8_t *data);
	int handle_frame_data_mjpeg(const uint32_t &width, const uint32_t &height,
		const size_t &size, const uint8_t *data, const double &presentation_time_s);
	int handle_frame_data_h264(const uint32_t &width, const uint32_t &height,
		const size_t &size, const uint8_t *data, const double &presentation_time_s);
	int handle_frame_data_vp8(const uint32_t &width, const uint32_t &height,
		const size_t &size, const uint8_t *data, const double &presentation_time_s);
	virtual int internal_start_recording(const std::string &file_name);
	virtual void internal_stop_recording();
public:
	UVCSensor(const char *uuid, const char *name);
	virtual ~UVCSensor();
	virtual int start(const char *command, const char *notify, const char *data);
};

} /* namespace sensor */
} /* namespace serenegiant */

#endif /* SENSOR_UVC_H_ */
