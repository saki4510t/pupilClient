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

#include "sensor_uvc.h"

namespace serenegiant {
namespace sensor {

/*public*/
UVCSensor::UVCSensor(const char *uuid, const char *name)
:	Sensor(SENSOR_UVC, uuid, name) {

	ENTER();

	EXIT();
}

/*virtual*/
/*public*/
UVCSensor::~UVCSensor() {

	ENTER();

	EXIT();
}

/*virtual*/
/*protected*/
int UVCSensor::on_receive_notify(const std::string &identity, zmq_msg_t &msg) {
	ENTER();

	int result = -1;
	const size_t size = zmq_msg_size(&msg);
	if (LIKELY(size > 0)) {
		// FIXME 未実装
		result = 0;
	}

	RETURN(result ,int);
}

/*virtual*/
/*protected*/
int UVCSensor::on_receive_data(const std::string &identity, const publish_header_t &header, zmq_msg_t &msg) {
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
