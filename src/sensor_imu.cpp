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

IMUSensor::IMUSensor(const char *uuid, const char *name)
:	Sensor(SENSOR_IMU, uuid, name) {

	ENTER();

	EXIT();
}

IMUSensor::~IMUSensor() {

	ENTER();

	EXIT();
}

int IMUSensor::on_receive_data(const publish_header_t &header, zmq_msg_t &msg) {
	ENTER();

	int result = -1;
	const size_t size = zmq_msg_size(&msg);
	if (LIKELY(size > 0)) {
		// FIXME 未実装
		result = 0;
	}

	RETURN(result ,int);
}

} /* namespace sensor */
} /* namespace serenegiant */