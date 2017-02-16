/*
 * sensor.cpp
 *
 *  Created on: 2017/02/16
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

#include "sensor.h"

namespace serenegiant {
namespace sensor {

Sensor::Sensor(const char *uuid, const char *name,
	const char *command, const char *notify, const char *data)
:	sensor_uuid(uuid),
	sensor_name(name) {

	ENTER();

	printf("Sensor#Constructor\n");

	EXIT();
}

Sensor::~Sensor() {
	ENTER();

	printf("Sensor#Destructor\n");

	EXIT();
}

}	// namespace sensor
}	// namespace serenegiant
