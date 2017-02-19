/*
 * sensor_manager.h
 *
 *  Created on: 2017/02/17
 *      Author: saki
 */

#ifndef SENSOR_MANAGER_H_
#define SENSOR_MANAGER_H_

#include "app_const.h"
#include "sensor_common.h"

namespace serenegiant {
namespace sensor {

using namespace rapidjson;

class Sensor;

class SensorManager {
private:
	volatile bool is_running;
	mutable Mutex sensor_lock;
	std::multimap<std::string, Sensor*> sensors;
	pthread_t zyre_thread;
	static void *zyre_thread_func(void *vptr_args);
	void *zmq_context;
	void zyre_run();
protected:
	void remove_sensor_all();
	void remove_sensors(const std::string &node_uuid);
	void remove_sensor(const std::string &node_uuid, const std::string &sensor_uuid);
	void remove_sensor_locked(const std::string &node_uuid, const std::string &sensor_uuid);
	Sensor *get_sensor(const std::string &node_uuid, const std::string &sensor_uuid);
	void add_sensor(const std::string &node_uuid, Sensor *sensor);

	int handle_enter(zyre_t *zyre, zyre_event_t *event,
		const char *node_uuid, const char *node_name);
	int handle_join(zyre_t *zyre, zyre_event_t *event,
		const char *node_uuid, const char *node_name);
	int handle_attach(zyre_t *zyre, zyre_event_t *event,
		const char *node_uuid, const char *node_name, Document &doc);
	int handle_detach(zyre_t *zyre, zyre_event_t *event,
		const char *node_uuid, const char *node_name, Document &doc);
	int handle_whisper(zyre_t *zyre, zyre_event_t *event,
		const char *node_uuid, const char *node_name);
	int handle_shout(zyre_t *zyre, zyre_event_t *event,
		const char *node_uuid, const char *node_name);
	int handle_leave(zyre_t *zyre, zyre_event_t *event,
		const char *node_uuid, const char *node_name);
	int handle_exit(zyre_t *zyre, zyre_event_t *event,
		const char *node_uuid, const char *node_name);
public:
	SensorManager();
	virtual ~SensorManager();
	int start();
	int stop();
	inline const bool isRunning() const { return is_running; };
};

}	// namespace sensor
}	// namespace serenegiant

#endif /* SENSOR_MANAGER_H_ */
