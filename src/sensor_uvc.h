/*
 * sensor_uvc.h
 *
 *  Created on: 2017/02/17
 *      Author: saki
 */

#ifndef SENSOR_UVC_H_
#define SENSOR_UVC_H_

#include "sensor.h"

namespace serenegiant {
namespace sensor {

class UVCSensor: public virtual Sensor {
protected:
	virtual int on_receive_notify(zmq_msg_t &msg);
	virtual int on_receive_data(const publish_header_t &header, zmq_msg_t &msg);
public:
	UVCSensor(const char *uuid, const char *name);
	virtual ~UVCSensor();
};

} /* namespace sensor */
} /* namespace serenegiant */

#endif /* SENSOR_UVC_H_ */
