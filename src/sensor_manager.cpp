/*
 * sensor_manager.cpp
 *
 *  Created on: 2017/02/17
 *      Author: saki
 */

#if 0	// set 0 if you need debug log, otherwise set 1
	#ifndef LOG_NDEBUG
		#define LOG_NDEBUG
	#endif
	#undef USE_LOGALL
#else
//	#define USE_LOGALL
//	#undef LOG_NDEBUG
	#undef NDEBUG
#endif

#include "utilbase.h"
#include "app_const.h"

#include "sensor_common.h"
#include "sensor_manager.h"

#include "sensor.h"
#include "sensor_audio.h"
#include "sensor_imu.h"
#include "sensor_uvc.h"

namespace serenegiant {
namespace sensor {

const char *target_group = "pupil-mobile";

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

//	if (zmq_context) {
//		int result = zmq_ctx_shutdown(zmq_context);
//		if (LIKELY(!result)) {
//			result = zmq_ctx_term(zmq_context);
//			if (result) {
//				LOGE("zmq_ctx_term failed, result=%d,errno=%d", result, errno);
//			}
//		} else {
//			LOGE("zmq_ctx_shutdown failed, result=%d,errno=%d", result, errno);
//		}
//
//		zmq_context = NULL;
//	}

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

// 指定したnode_uuidで指定したsensor_uuidを持つセンサーを取得する
/*protected*/
Sensor *SensorManager::get_sensor(
	const std::string &node_uuid, const std::string &sensor_uuid) {

	ENTER();

	Sensor *result = NULL;
	Mutex::Autolock lock(sensor_lock);

	for (auto itr = sensors.find(node_uuid); itr != sensors.end(); itr++) {
		Sensor *sensor = itr->second;
		if (sensor && (sensor->uuid() == sensor_uuid)) {
			result = sensor;
			break;
		}
	}

	RET(result);
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

	LOGD("join to %s", zyre_event_group(event));

	RETURN(0, int);
}

/*protected*/
int SensorManager::handle_attach(zyre_t *zyre, zyre_event_t *event,
	const char *node_uuid, const char *node_name, Document &doc) {

	ENTER();

	int result = -1;
	const char *sensor_name = NULL, *sensor_uuid = NULL, *sensor_type = NULL;
	const char *notify = NULL, *command = NULL, *data = NULL;
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
	itr = doc.FindMember("sensor_type");
	if (itr != doc.MemberEnd()) {
		const Value &v = itr->value;
		sensor_type = v.GetString();
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

	if (LIKELY(sensor_uuid && sensor_name && sensor_type && notify && command && data)) {
//		LOGD("uuid=%s\nname=%s\ntype=%s\nnotify=%s\ncommand=%s\ndata=%s",
//			sensor_uuid, sensor_name, sensor_type, notify, command, data);
		Sensor *sensor = get_sensor(node_uuid, sensor_uuid);
		if (!sensor) {
			switch (get_sensor_type(sensor_type)) {
			case SENSOR_MIC:
				sensor = new AudioSensor(sensor_uuid, sensor_name);
				break;
			case SENSOR_IMU:
				sensor = new IMUSensor(sensor_uuid, sensor_name);
				break;
			case SENSOR_UVC:
				sensor = new UVCSensor(sensor_uuid, sensor_name);
				break;
			default:
				sensor = NULL;
				break;
			}
			if (sensor) {
				add_sensor(node_uuid, sensor);
				sensor->start(/*zmq_context,*/ command, notify, data);
				result = 0;
			} else {
				LOGE("unknown sensor type:%s", sensor_type);
			}
		} else {
			LOGV("sensor is already attached");
			result = 0;
		}
	} else {
		LOGE("unexpected attach payload");
	}

	RETURN(result, int);
}

/*protected*/
int SensorManager::handle_detach(zyre_t *zyre, zyre_event_t *event,
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
//			LOGV("msg=%s", str);
			rapidjson::Document doc;
			doc.Parse(str);
			if (LIKELY(!doc.HasParseError())) {
				const char *subject = json_get_string(doc, "subject");
				if (subject) {
					switch (get_subject_type(subject)) {
					case SUBJECT_ATTACH:
						handle_attach(zyre, event, node_uuid, node_name, doc);
						break;
					case SUBJECT_DETACH:
						handle_detach(zyre, event, node_uuid, node_name, doc);
						break;
					default:
						LOGE("unknown subject type:%s", subject);
					}
				} else {
					LOGE("missing subject field");
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

	LOGD("shout to %s", zyre_event_group(event));

	zmsg_t *msg = zyre_event_get_msg(event);
	if (msg) {
		zmsg_print(msg);
		char *str = zmsg_popstr(msg);
		if (str) {
			LOGD("msg=%s", str);
			rapidjson::Document doc;
			doc.Parse(str);
			if (LIKELY(!doc.HasParseError())) {
				const char *subject = json_get_string(doc, "subject");
				if (subject) {
					switch (get_subject_type(subject)) {
					case SUBJECT_ATTACH:
						handle_attach(zyre, event, node_uuid, node_name, doc);
						break;
					case SUBJECT_DETACH:
						handle_detach(zyre, event, node_uuid, node_name, doc);
						break;
					default:
						LOGE("unknown subject type:%s", subject);
					}
				} else {
					LOGE("missing subject field");
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
int SensorManager::handle_leave(zyre_t *zyre, zyre_event_t *event,
	const char *node_uuid, const char *node_name) {

	ENTER();

	LOGD("leave from %s", zyre_event_group(event));

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
/*private*/
void *SensorManager::zyre_thread_func(void *vptr_args) {
	ENTER();

	SensorManager *manager = reinterpret_cast<SensorManager *>(vptr_args);
	if (LIKELY(manager)) {
		manager->zyre_run();
	}

	PRE_EXIT();
	pthread_exit(NULL);
}

/*private*/
void SensorManager::zyre_run() {
	ENTER();

	zyre_t *node = zyre_new(NULL);
	zyre_set_header(node, "X-Client", "ffmpegDecoder");
	zyre_join(node, target_group);
	int status = zyre_start(node);
#ifndef LOG_NDEBUG
	zyre_print(node);
#endif
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
						LOGD("-----------------");
						LOGD("NODE_EVENT_TYPE: %s", event_type);
						LOGD("NODE_UUID      : %s", node_uuid);
						LOGD("NODE_NAME      : %s", node_name);
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
		LOGE("could not start zyre node");
	}
	zyre_destroy(&node);

	is_running = false;
	EXIT();
}

}	// namespace sensor
}	// namespace serenegiant

