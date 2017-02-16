/*
 * sensor.h
 *
 *  Created on: 2017/02/16
 *      Author: saki
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include "app_const.h"

namespace serenegiant {
namespace sensor {

class Sensor {
private:
	const std::string sensor_uuid;
	const std::string sensor_name;
public:
	Sensor(const char *uuid, const char *name, const char *command, const char *notify, const char *data);
	virtual ~Sensor();

	inline const std::string uuid() const { return sensor_uuid; };
	inline const std::string name() const { return sensor_name; };
};

}	// namespace sensor
}	// namespace serenegiant

#endif /* SENSOR_H_ */
