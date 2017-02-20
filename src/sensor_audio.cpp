/*
 * sensor_audio.cpp
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

#include "sensor_audio.h"

namespace serenegiant {
namespace sensor {

/*public*/
AudioSensor::AudioSensor(const char *uuid, const char *name)
:	Sensor(SENSOR_MIC, uuid, name) {

	ENTER();

	EXIT();
}

/*virtual*/
/*public*/
AudioSensor::~AudioSensor() {

	ENTER();

	EXIT();
}

/*virtual*/
/*protected*/
int AudioSensor::handle_notify_update(const std::string &identity, const std::string &payload) {
	ENTER();

	int result = -1;

	RETURN(result ,int);
}

/*virtual*/
/*protected*/
int AudioSensor::handle_frame_data(const std::string &identity, const publish_header_t &header, const size_t &size, const uint8_t *data) {
	ENTER();

	int result = -1;
	if (LIKELY(size > 0)) {
		// FIXME 未実装
		result = 0;
	}

	RETURN(result ,int);
}

}	// namespace sensor
}	// namespace serenegiant
