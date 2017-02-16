/*
 * sensor_manager.cpp
 *
 *  Created on: 2017/02/17
 *      Author: saki
 */

#include "utilbase.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/en.h"

#include "sensor_manager.h"
#include "sensor.h"

namespace serenegiant {
namespace sensor {

using namespace rapidjson;

const char *target_group = "pupil-mobile";

//================================================================================
static void json_error(Document &doc) {
	if (doc.HasParseError()) {
		size_t offset = doc.GetErrorOffset();
		ParseErrorCode code = doc.GetParseError();
		const char *err = GetParseError_En(code);
		printf("json error:%lu:%d(%s)\n", offset, code, err);
	}
}

//================================================================================
/*public*/
SensorManager::SensorManager()
:	is_running(false),
	zyre_thread() {

	ENTER();

	EXIT();
}

/*public*/
SensorManager::~SensorManager() {
	ENTER();

	stop();
	remove_sensor_all();

	EXIT();
}

/*public*/
int SensorManager::stop() {
	ENTER();

	const bool b = isRunning();
	is_running = false;
	if (b) {
		if (pthread_join(zyre_thread, NULL) != 0) {
			LOGW("failed pthread_join:");
		}
	}

	RETURN(0, int);
}

/*public*/
int SensorManager::start() {
	ENTER();

	int result = 0;
	if (!isRunning()) {
		is_running = true;
		result = pthread_create(&zyre_thread, NULL,
			zyre_thread_func, (void *) this);

		if (UNLIKELY(result)) {
			stop();
		}
	}

	RETURN(result, int);
}

//================================================================================
// 保持しているすべてのセンサーを削除する
void SensorManager::remove_sensor_all() {

	ENTER();

	Mutex::Autolock lock(sensor_lock);

	for (auto itr: sensors) {
		Sensor *sensor = itr.second;
		SAFE_DELETE(sensor);
	}
	sensors.clear();

	EXIT();
}

// 同じnode_uuidを持つセンサーをすべて削除する
void SensorManager::remove_sensors(const std::string &node_uuid) {
	ENTER();

	Mutex::Autolock lock(sensor_lock);

	for (auto itr = sensors.find(node_uuid); itr != sensors.end(); itr++) {
		Sensor *sensor = itr->second;
		if (sensor) {
			SAFE_DELETE(sensor);
			sensors.erase(itr);
		}
	}

	EXIT();
}

// 指定したnode_uuidで指定したsensor_uuidを持つセンサーを削除する
// こっちは排他制御した状態で呼ぶ。
// 排他制御をしてないないときは#remove_sensorを呼ぶ
/*protected*/
void SensorManager::remove_sensor_locked(
	const std::string &node_uuid, const std::string &sensor_uuid) {

	ENTER();

	for (auto itr = sensors.find(node_uuid); itr != sensors.end(); itr++) {
		Sensor *sensor = itr->second;
		if (sensor && (sensor->uuid() == sensor_uuid)) {
			SAFE_DELETE(sensor);
			sensors.erase(itr);
		}
	}

	EXIT();
}

// 指定したnode_uuidで指定したsensor_uuidを持つセンサーを削除する
/*protected*/
void SensorManager::remove_sensor(
	const std::string &node_uuid, const std::string &sensor_uuid) {

	Mutex::Autolock lock(sensor_lock);

	remove_sensor_locked(node_uuid, sensor_uuid);
}

// センサーを追加する。
// 同じnode_uuidで同じsensor_uuidのセンサーがあれば削除してから追加する
/*protected*/
void SensorManager::add_sensor(const std::string &node_uuid, Sensor *sensor) {

	ENTER();

	Mutex::Autolock lock(sensor_lock);

	const std::string &sensor_uuid = sensor ? sensor->uuid() : "";
	// 同じuuidのノードで同じセンサー名のものがあれば削除する
	remove_sensor_locked(node_uuid, sensor_uuid);
	//
	if (sensor) {
		sensors.insert(std::make_pair(node_uuid, sensor));
	}

	EXIT();
}

//================================================================================
/*protected*/
int SensorManager::handle_enter(zyre_t *zyre, zyre_event_t *event,
	const char *node_uuid, const char *node_name) {

	ENTER();

	RETURN(0, int);
}

/*protected*/
int SensorManager::handle_join(zyre_t *zyre, zyre_event_t *event,
	const char *node_uuid, const char *node_name) {
	ENTER();

	const char *group = zyre_event_group(event);
	printf("join to %s\n", group);

	RETURN(0, int);
}

/*protected*/
int SensorManager::handle_whisper_attach(zyre_t *zyre, zyre_event_t *event,
	const char *node_uuid, const char *node_name, Document &doc) {

	ENTER();

	const char *sensor_name = NULL, *sensor_uuid, *notify = NULL, *command = NULL, *data = NULL;
	Value::ConstMemberIterator itr = doc.FindMember("sensor_uuid");
	if (itr != doc.MemberEnd()) {
		const Value &v = itr->value;
		sensor_uuid = v.GetString();
	}
	itr = doc.FindMember("sensor_name");
	if (itr != doc.MemberEnd()) {
		const Value &v = itr->value;
		sensor_name = v.GetString();
	}
	itr = doc.FindMember("notify_endpoint");
	if (itr != doc.MemberEnd()) {
		const Value &v = itr->value;
		notify = v.GetString();
	}
	itr = doc.FindMember("command_endpoint");
	if (itr != doc.MemberEnd()) {
		const Value &v = itr->value;
		command = v.GetString();
	}
	itr = doc.FindMember("data_endpoint");
	if (itr != doc.MemberEnd()) {
		const Value &v = itr->value;
		data = v.GetString();
	}

	if (LIKELY(sensor_name && sensor_uuid && notify && command && data)) {
		printf("uuid=%s\nname=%s\nnnotify=%s\ncommand=%s\ndata=%s\n",
			sensor_uuid, sensor_name, notify, command, data);
		Sensor *sensor = new Sensor(sensor_uuid, sensor_name, notify, command, data);
		add_sensor(node_uuid, sensor);
	}

	RETURN(0, int);
}

/*protected*/
int SensorManager::handle_whisper_detach(zyre_t *zyre, zyre_event_t *event,
	const char *node_uuid, const char *node_name, Document &doc) {

	ENTER();

	const char *sensor_uuid = NULL;
	Value::ConstMemberIterator itr = doc.FindMember("sensor_uuid");
	if (itr != doc.MemberEnd()) {
		const Value &v = itr->value;
		sensor_uuid = v.GetString();
	}
	if (LIKELY(sensor_uuid)) {
		remove_sensor(node_uuid, sensor_uuid);
	}

	RETURN(0, int);
}

/*protected*/
int SensorManager::handle_whisper(zyre_t *zyre, zyre_event_t *event,
	const char *node_uuid, const char *node_name) {

	ENTER();

	zmsg_t *msg = zyre_event_get_msg(event);
	if (msg) {
		zmsg_print(msg);
		char *str = zmsg_popstr(msg);
		if (str) {
			printf("msg=%s\n", str);
			Document doc;
			doc.Parse(str);
			if (LIKELY(!doc.HasParseError())) {
				Value::ConstMemberIterator itr = doc.FindMember("subject");
				if (itr != doc.MemberEnd()) {
					const Value &v = itr->value;
					const char *subject = v.GetString();
					if (subject) {
						if (streq(subject, "attach")) {
							handle_whisper_attach(zyre, event, node_uuid, node_name, doc);
						} else if (streq(subject, "detach")) {
							handle_whisper_detach(zyre, event, node_uuid, node_name, doc);
						}
					}
				}
			} else {
				json_error(doc);
			}
			zstr_free(&str);
		}
		zmsg_destroy(&msg);
	}

	RETURN(0, int);
}

/*protected*/
int SensorManager::handle_shout(zyre_t *zyre, zyre_event_t *event,
	const char *node_uuid, const char *node_name) {

	ENTER();

	const char *group = zyre_event_group(event);
	printf("shout to %s\n", group);

	zmsg_t *msg = zyre_event_get_msg(event);
	if (msg) {
		zmsg_print(msg);
		char *str = zmsg_popstr(msg);
		if (str) {
			printf("msg=%s", str);
			zstr_free(&str);
		}
		zmsg_destroy(&msg);
	}

	RETURN(0, int);
}

/*protected*/
int SensorManager::handle_leave(zyre_t *zyre, zyre_event_t *event,
	const char *node_uuid, const char *node_name) {

	ENTER();

	const char *group = zyre_event_group(event);
	printf("leave from %s\n", group);

	remove_sensors(node_uuid);

	RETURN(0, int);
}

/*protected*/
int SensorManager::handle_exit(zyre_t *zyre, zyre_event_t *event,
	const char *node_uuid, const char *node_name) {

	ENTER();

	remove_sensors(node_uuid);

	RETURN(0, int);
}

/*static*/
/*protected*/
void *SensorManager::zyre_thread_func(void *vptr_args) {
	ENTER();

	SensorManager *manager = reinterpret_cast<SensorManager *>(vptr_args);
	if (LIKELY(manager)) {
		manager->zyre_run();
	}

	PRE_EXIT();
	pthread_exit(NULL);
}

void SensorManager::zyre_run() {
	zyre_t *node = zyre_new(NULL);
	zyre_set_header(node, "X-Client", "ffmpegDecoder");
	zyre_join(node, target_group);
	int status = zyre_start(node);
	zyre_print(node);
	if (status == 0) {
		zsock_t *socket = zyre_socket(node);
		zpoller_t *poller = zpoller_new(socket);
		if (LIKELY(poller)) {
			for ( ; isRunning() ; ) {
				zsock_t *which = (zsock_t *) zpoller_wait(poller, 100);
				if (which == socket) {
					zyre_event_t *event = zyre_event_new(node);
					if (LIKELY(event)) {
						const char *event_type = zyre_event_type(event);
						const char *node_uuid = zyre_event_peer_uuid(event);
						const char *node_name = zyre_event_peer_name(event);
						printf("-----------------\n");
						printf("NODE_EVENT_TYPE: %s\n", event_type);
						printf("NODE_UUID      : %s\n", node_uuid);
						printf("NODE_NAME      : %s\n", node_name);
						if (streq(event_type, "ENTER")) {
							handle_enter(node, event, node_uuid, node_name);
						} else if (streq(event_type, "JOIN")) {
							handle_join(node, event, node_uuid, node_name);
						} else if (streq(event_type, "WHISPER")) {
							handle_whisper(node, event, node_uuid, node_name);
						} else if (streq(event_type, "SHOUT")) {
							handle_shout(node, event, node_uuid, node_name);
						} else if (streq(event_type, "LEAVE")) {
							handle_leave(node, event, node_uuid, node_name);
						} else if (streq(event_type, "EXIT")) {
							handle_exit(node, event, node_uuid, node_name);
						} else {
							LOGW("unexpected event type:%s", event_type);
						}
						zyre_event_destroy(&event);
					}
				} else {
				}
			}
			zpoller_destroy(&poller);
		}
		zyre_leave(node, target_group);
		zyre_stop(node);
	} else {
		LOGE("could not start zyre node\n");
	}
	zyre_destroy(&node);
}

}	// namespace sensor
}	// namespace serenegiant

