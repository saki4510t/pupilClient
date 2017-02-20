/*
 * sensor_audio.h
 *
 *  Created on: 2017/02/17
 *      Author: saki
 */

#ifndef SENSOR_AUDIO_H_
#define SENSOR_AUDIO_H_

#include "sensor.h"

namespace serenegiant {
namespace sensor {

class AudioSensor: public virtual Sensor {
protected:
	virtual int handle_notify_update(const std::string &identity, const std::string &payload);
	virtual int handle_frame_data(const std::string &identity, const publish_header_t &header, const size_t &size, const uint8_t *data);
public:
	AudioSensor(const char *uuid, const char *name);
	virtual ~AudioSensor();
};

}	// namespace sensor
}	// namespace serenegiant

#endif /* SENSOR_AUDIO_H_ */
