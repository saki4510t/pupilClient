/*
 * sensor_ime.cpp
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

#include "sensor_imu.h"

namespace serenegiant {
namespace sensor {

/*public*/
IMUSensor::IMUSensor(const char *uuid, const char *name)
:	Sensor(SENSOR_IMU, uuid, name) {

	ENTER();

	EXIT();
}

/*virtual*/
/*public*/
IMUSensor::~IMUSensor() {

	ENTER();

	EXIT();
}

/*virtual*/
/*protected*/
int IMUSensor::handle_notify_update(const std::string &identity, const std::string &payload) {
	ENTER();

	int result = -1;

	RETURN(result ,int);
}

/*virtual*/
/*protected*/
int IMUSensor::handle_frame_data(const std::string &identity, const publish_header_t &header, const size_t &size, const uint8_t *data) {
	ENTER();

	int result = -1;
	const uint32_t sequence = letoh32_unaligned(&header.imu.sequence_le);
	const uint32_t data_bytes = letoh32_unaligned(&header.imu.data_bytes_le);
	if (LIKELY((size > 0) && (size == data_bytes))) {
		LOGD("actual bytes=%lu,sequence=%u", size, sequence);
		// FIXME 未実装
		result = 0;
	} else {
		LOGW("data_bytes=%u, received=%lu", data_bytes, size);
	}

	RETURN(result ,int);
}

} /* namespace sensor */
} /* namespace serenegiant */
