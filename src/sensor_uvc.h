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
	virtual int handle_notify_update(const std::string &identity, const std::string &payload);
	virtual int handle_frame_data(const std::string &identity, const publish_header_t &header, const size_t &size, const uint8_t *data);
public:
	UVCSensor(const char *uuid, const char *name);
	virtual ~UVCSensor();
};

} /* namespace sensor */
} /* namespace serenegiant */

#endif /* SENSOR_UVC_H_ */
