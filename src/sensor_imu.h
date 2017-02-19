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
	virtual int on_receive_data(const publish_header_t &header, zmq_msg_t &msg);
public:
	IMUSensor(const char *uuid, const char *name);
	virtual ~IMUSensor();
};

} /* namespace sensor */
} /* namespace serenegiant */

#endif /* SENSOR_IMU_H_ */
