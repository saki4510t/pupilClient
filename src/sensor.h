/*
 * sensor.h
 *
 *  Created on: 2017/02/16
 *      Author: saki
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include "app_const.h"
#include "sensor_common.h"

namespace serenegiant {
namespace sensor {

typedef struct publish_header {
    uint32_t format_le; // MJPEG, H264, (YUYV, VP8)
    uint32_t width_le;
    uint32_t height_le;
    uint32_t sequence_le;
    int64_t presentation_time_us_le;
    uint32_t data_bytes_le;
    uint32_t reserved_le;
} __attribute__ ((packed)) publish_header_t;

class Sensor {
private:
	const sensor_type_t sensor_type;
	const std::string sensor_uuid;
	const std::string sensor_name;

	std::string command_endpoint;
	std::string notify_endpoint;
	std::string data_endpoint;

	volatile bool is_running;
	void *zmq_context;
	void *command_socket;
	void *notify_socket;
	void *data_socket;
	pthread_t zmq_thread;
	static void *zmq_thread_func(void *vptr_args);
	void zmq_run();
	void zmq_stop();
protected:
	int receive_notify();
	int receive_data();
	virtual int on_receive_notify(zmq_msg_t &msg) = 0;
	virtual int on_receive_data(const publish_header_t &header, zmq_msg_t &msg) = 0;
public:
	Sensor(const sensor_type_t &sensor_type, const char *uuid, const char *name);
	virtual ~Sensor();

	inline const sensor_type_t sensorType() const { return sensor_type; };
	inline const std::string uuid() const { return sensor_uuid; };
	inline const std::string name() const { return sensor_name; };

	int start(void *zmq_context, const char *command, const char *notify, const char *data);
	int stop();
	inline const bool isRunning() const { return is_running; };
};

}	// namespace sensor
}	// namespace serenegiant

#endif /* SENSOR_H_ */
