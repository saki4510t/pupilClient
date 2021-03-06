/*
 * sensor_imu.h
 *
 *  Created on: 2017/02/17
 *      Author: saki
 */

#ifndef SENSOR_IMU_H_
#define SENSOR_IMU_H_

#include "sensor.h"

namespace serenegiant {
namespace sensor {

class IMUSensor: public virtual Sensor {
protected:
	virtual int handle_notify_update(const std::string &identity, const std::string &payload);
	virtual int handle_frame_data(const std::string &identity, const publish_header_t &header, const size_t &size, const uint8_t *data);
	virtual int internal_start_recording(const std::string &file_name);
	virtual void internal_stop_recording();
public:
	IMUSensor(const char *uuid, const char *name);
	virtual ~IMUSensor();
};

} /* namespace sensor */
} /* namespace serenegiant */

#endif /* SENSOR_IMU_H_ */
