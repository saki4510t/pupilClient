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

class Sensor {
private:
	const sensor_type_t sensor_type;
	const std::string sensor_uuid;
	const std::string sensor_name;
	const char *sensor_identity;	// = sensor_uuid.c_str()

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
	int zmq_start();
	void zmq_stop();
protected:
	int receive_notify();
	int receive_data();
	virtual int handle_notify_update(const std::string &identity, const std::string &payload) = 0;
	virtual int handle_notify_remove(const std::string &identity, const std::string &payload);
	virtual int handle_notify_error(const std::string &identity, const std::string &payload);
	virtual int handle_frame_data(const std::string &identity, const publish_header_t &header, const size_t &size, const uint8_t *data) = 0;
	int create_payload(Writer<StringBuffer> &writer);
	int create_payload(Writer<StringBuffer> &writer, const request_type_t &request);
	int send(const std::string &msg_str, const int &flag = 0);
	int send(const uint8_t *msg_bytes, const size_t &size, const int &flag = 0);
	int request_refresh_controls();
	int set_control_value(const std::string &control_id, const bool &value);
	int set_control_value(const std::string &control_id, const int &value);
public:
	Sensor(const sensor_type_t &sensor_type, const char *uuid, const char *name);
	virtual ~Sensor();

	inline const sensor_type_t sensorType() const { return sensor_type; };
	inline const std::string uuid() const { return sensor_uuid; };
	inline const std::string name() const { return sensor_name; };

	int start(const char *command, const char *notify, const char *data);
	int stop();
	inline const bool isRunning() const { return is_running; };
};

}	// namespace sensor
}	// namespace serenegiant

#endif /* SENSOR_H_ */
