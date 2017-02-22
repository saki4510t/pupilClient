/*
 * sensor.cpp
 *
 *  Created on: 2017/02/16
 *      Author: saki
 */

#if 1	// set 0 if you need debug log, otherwise set 1
	#ifndef LOG_NDEBUG
		#define LOG_NDEBUG
	#endif
	#undef USE_LOGALL
#else
//	#define USE_LOGALL
	#undef LOG_NDEBUG
	#undef NDEBUG
#endif

#include "utilbase.h"
#include "app_const.h"

#include "sensor_common.h"
#include "sensor.h"

namespace serenegiant {
namespace sensor {

static std::string recv_str(void *socket) {
//	ENTER();

	std::string result;

	zmq_msg_t msg;
	int r = zmq_msg_init(&msg);
	if (LIKELY(!r)) {
		r = zmq_msg_recv(&msg, socket, 0);
		if (r > 0) {
			char data[r + 1];
			memcpy(data, zmq_msg_data(&msg), r);
			data[r] = '\0';
			result = data;
		}
		zmq_msg_close(&msg);
	}

	return result; //	RET(result);
}

//================================================================================

Sensor::Sensor(const sensor_type_t &_sensor_type, const char *uuid, const char *name)
:	sensor_type(_sensor_type),
	sensor_uuid(uuid),
	sensor_name(name),
	sensor_identity(sensor_uuid.c_str()),
	is_running(false),
	zmq_context(zmq_ctx_new()),
	command_socket(NULL),
	notify_socket(NULL),
	data_socket(NULL),
	zmq_thread() {

	ENTER();

	LOGI("Sensor#Constructor");
	if (UNLIKELY(!zmq_context)) {
		LOGE("zmq_ctx_new failed, errno=%d", errno);
	}

	EXIT();
}

Sensor::~Sensor() {
	ENTER();

	LOGI("Sensor#Destructor");
	stop();

	if (zmq_context) {
		int result = zmq_ctx_shutdown(zmq_context);
		if (LIKELY(!result)) {
			result = zmq_ctx_term(zmq_context);
			if (result) {
				LOGE("zmq_ctx_term failed, result=%d,errno=%d", result, errno);
			}
		} else {
			LOGE("zmq_ctx_shutdown failed, result=%d,errno=%d", result, errno);
		}

		zmq_context = NULL;
	}

	EXIT();
}

static const int LINGER = 100;
static const int CONNECT_TIMEOUT = 1000;
static const int RCV_TIMEOUT = 200;
static const int SND_TIMEOUT = 200;
static const int RCV_HWM = 3;
static const int SND_HWM = 3;

static int setup_socket(void *zmq_socket) {
	ENTER();

	int result = zmq_setsockopt(zmq_socket, ZMQ_LINGER, &LINGER, sizeof(int));
	if (UNLIKELY(result)) {
		LOGE("failed to set connect timeout:errno=%d", errno);
		goto ret;
	}
	result = zmq_setsockopt(zmq_socket, ZMQ_CONNECT_TIMEOUT, &CONNECT_TIMEOUT, sizeof(int));
	if (UNLIKELY(result)) {
		LOGE("failed to set connect timeout:errno=%d", errno);
		goto ret;
	}
	result = zmq_setsockopt(zmq_socket, ZMQ_RCVTIMEO, &RCV_TIMEOUT, sizeof(int));
	if (UNLIKELY(result)) {
		LOGE("failed to set rcv timeout:errno=%d", errno);
		goto ret;
	}
	result = zmq_setsockopt(zmq_socket, ZMQ_SNDTIMEO, &SND_TIMEOUT, sizeof(int));
	if (UNLIKELY(result)) {
		LOGE("failed to set send timeout:errno=%d", errno);
		goto ret;
	}
	result = zmq_setsockopt(zmq_socket, ZMQ_RCVHWM, &RCV_HWM, sizeof(int));
	if (UNLIKELY(result)) {
		LOGE("failed to set rcv high water mark:errno=%d", errno);
		goto ret;
	}
	result = zmq_setsockopt(zmq_socket, ZMQ_SNDHWM, &SND_HWM, sizeof(int));
	if (UNLIKELY(result)) {
		LOGE("failed to set send high water mark:errno=%d", errno);
		goto ret;
	}

ret:
	RETURN(result, int);
}

/*public*/
int Sensor::start(/*void *_zmq_context,*/
	const char *command, const char *notify, const char *data) {

	ENTER();

	MARK("start Sensor %s", sensor_name.c_str());

	int result = 0;
	if (!isRunning()) {
		if (LIKELY(zmq_context && command && notify && data)) {
			command_endpoint = command;
			notify_endpoint = notify;
			data_endpoint = data;

			result = zmq_start();
			if (UNLIKELY(result)) {
				goto err;
			}

		} else {
			LOGE("zmq context / command / notify / data should not be null");
			goto err;
		}

		is_running = true;
		result = pthread_create(&zmq_thread, NULL, zmq_thread_func, (void *) this);

		if (UNLIKELY(result)) {
			stop();
		}
	} else {
		LOGD("already running:uuid=%s", sensor_uuid.c_str());
	}

	RETURN(result, int);

err:
	zmq_stop();
	RETURN(result, int);
}

/*public*/
int Sensor::stop() {
	ENTER();

	MARK("stop Sensor %s", sensor_name.c_str());

	const bool b = isRunning();
	is_running = false;
	if (b) {
		if (pthread_join(zmq_thread, NULL) != 0) {
			LOGW("failed pthread_join:");
		}
		zmq_stop();
	}

	RETURN(0, int);
}

/*private*/
int Sensor::zmq_start() {
	ENTER();

	int result = -1;

	const char *subscription = sensor_uuid.c_str();
	const int subscription_size = strlen(subscription);

	command_socket = zmq_socket(zmq_context, ZMQ_PUSH);
	if (LIKELY(command_socket)) {
		result = setup_socket(command_socket);
		if (UNLIKELY(result)) {
			goto err;
		}
		LOGV("command: connect to %s", command_endpoint.c_str());
		result = zmq_connect(command_socket, command_endpoint.c_str());
		if (UNLIKELY(result)) {
			LOGE("failed to connect to command:errno=%d", errno);
			goto err;
		}
	} else {
		LOGE("failed to create command socket:errno=%d", errno);
		goto err;
	}

	notify_socket = zmq_socket(zmq_context, ZMQ_SUB);
	if (LIKELY(notify_socket)) {
		result = setup_socket(notify_socket);
		if (UNLIKELY(result)) {

			goto err;
		}
		LOGV("notify: connect to %s", notify_endpoint.c_str());
		result = zmq_connect(notify_socket, notify_endpoint.c_str());
		if (UNLIKELY(result)) {

			LOGE("failed to connect to notify:errno=%d", errno);
			goto err;
		}
		LOGV("notify: subscribe %s", subscription);
		result = zmq_setsockopt(notify_socket, ZMQ_SUBSCRIBE, subscription, subscription_size);
		if (UNLIKELY(result)) {
			LOGE("failed to set subscription:errno=%d, subscription=%s", errno, subscription);
//			goto err;
		}
	} else {
		LOGE("failed to create notify socket:errno=%d", errno);
		goto err;
	}

	data_socket = zmq_socket(zmq_context, ZMQ_SUB);
	if (LIKELY(data_socket)) {
		result = setup_socket(data_socket);

		if (UNLIKELY(result)) {
			zmq_close(data_socket);
			data_socket = NULL;
			goto err;
		}
		LOGV("data: connect to %s", data_endpoint.c_str());
		result = zmq_connect(data_socket, data_endpoint.c_str());
		if (UNLIKELY(result)) {
			zmq_close(data_socket);
			data_socket = NULL;
			LOGE("failed to connect to data:errno=%d", errno);
			goto err;
		}
		LOGV("data: subscribe %s", subscription);
		result = zmq_setsockopt(data_socket, ZMQ_SUBSCRIBE, subscription, subscription_size);
		if (UNLIKELY(result)) {
			LOGE("failed to set subscription:errno=%d, subscription=%s", errno, subscription);
//					goto err;
		}
	} else {
		LOGE("failed to create data socket:errno=%d", errno);
		goto err;
	}

	RETURN(result, int);

err:
	zmq_stop();
	RETURN(result, int);
}

/*private*/
void Sensor::zmq_stop() {
	ENTER();

	is_running = false;
	if (data_socket) {
		LOGV("release data socket");
		zmq_setsockopt(data_socket, ZMQ_UNSUBSCRIBE, sensor_uuid.c_str(), sensor_uuid.size());
		zmq_disconnect(data_socket, data_endpoint.c_str());
		zmq_close(data_socket);
		data_socket = NULL;
		if (notify_socket) {
			LOGV("release notify socket");
			zmq_setsockopt(notify_socket, ZMQ_UNSUBSCRIBE, sensor_uuid.c_str(), sensor_uuid.size());
			zmq_disconnect(notify_socket, notify_endpoint.c_str());
			zmq_close(notify_socket);
			notify_socket = NULL;
		}
		if (command_socket) {
			LOGV("release command socket");
			zmq_disconnect(command_socket, command_endpoint.c_str());
			zmq_close(command_socket);
			command_socket = NULL;
		}
	} else {
		if (notify_socket) {
			LOGV("release notify socket");
			zmq_close(notify_socket);
			notify_socket = NULL;
			if (command_socket) {
				LOGV("release command socket");
				zmq_disconnect(command_socket, notify_endpoint.c_str());
				zmq_close(command_socket);
				command_socket = NULL;
			}
		} else {
			if (command_socket) {
				LOGV("release command socket");
				zmq_close(command_socket);
				command_socket = NULL;
			}
		}
	}

	EXIT();
}

/*static*/
/*private*/
void *Sensor::zmq_thread_func(void *vptr_args) {
	ENTER();

	Sensor *sensor = reinterpret_cast<Sensor *>(vptr_args);
	if (LIKELY(sensor)) {
		sensor->zmq_run();
	}

	PRE_EXIT();
	pthread_exit(NULL);
}

/*private*/
void Sensor::zmq_run() {
	ENTER();

//	zmq_start();
	zmq_pollitem_t items[2];
	items[0].socket = notify_socket;
	items[0].events = ZMQ_POLLIN;
	items[0].fd = 0;
	items[1].socket = data_socket;
	items[1].events = ZMQ_POLLIN;
	items[1].fd = 0;

	int cnt = 0;
	bool need_refresh_controls = true;
	bool first_time = true;
	need_refresh_controls = request_refresh_controls();
	for ( ; isRunning() ; ) {
		int ix = zmq_poll(items, 2, 100);
		if (LIKELY(isRunning())) {
			if (ix > 0) {
	//			LOGV("zmq_poll:result=%d", ix);
				if ((items[0].revents & ZMQ_POLLIN) == ZMQ_POLLIN) {
					// notify_socket ready to receive
					receive_notify();
				}
				if ((items[1].revents & ZMQ_POLLIN) == ZMQ_POLLIN) {
					// data socket ready to receive
					receive_data();
				}
			}

			if (((++cnt % 30) == 0) && need_refresh_controls) {
				need_refresh_controls = request_refresh_controls();
			}
			if (first_time && (cnt % 50) == 0) {
				first_time = false;
				// force publishing turn on for testing after approx. 5 secs.
				set_control_value("streaming", true);
			}
		}
	}
	is_running = false;

	zmq_stop();
	EXIT();
}

int Sensor::handle_notify_remove(const std::string &identity, const std::string &payload) {
	ENTER();

	LOGE("remove:%s", payload.c_str());

	RETURN(0, int);
}

int Sensor::handle_notify_error(const std::string &identity, const std::string &payload) {
	ENTER();

	LOGE("error:%s", payload.c_str());

	RETURN(0, int);
}

int Sensor::receive_notify() {
	ENTER();

	int result = -1;

	std::string identity = recv_str(notify_socket);
	if (LIKELY(!identity.empty())) {
		if (identity != sensor_uuid) {
			LOGW("unexpected identity: expect=%s,actual=%s", sensor_uuid.c_str(), identity.c_str());
		}
		std::string payload = recv_str(notify_socket);
		if (LIKELY(!payload.empty())) {
			rapidjson::Document doc;
			doc.Parse(payload.c_str());
			const char *subject = json_get_string(doc, "subject");
			switch (get_subject_type(subject)) {
			case SUBJECT_UPDATE:
				result = handle_notify_update(identity, payload);
				break;
			case SUBJECT_REMOVE:
				result = handle_notify_remove(identity, payload);
				break;
			case SUBJECT_ERROR:
				result = handle_notify_error(identity, payload);
				break;
			default:
				LOGE("unexpected subject type, payload=%s", payload.c_str());
				break;
			}
		}
	}

	RETURN(result, int);
}

int Sensor::receive_data() {
//	ENTER();

	int result = -1;

	std::string identity = recv_str(data_socket);
	if (LIKELY(!identity.empty())) {
		if (identity != sensor_uuid) {
			LOGW("unexpected identity: expect=%s,actual=%s", sensor_uuid.c_str(), identity.c_str());
		}
		zmq_msg_t msg;
		result = zmq_msg_init(&msg);
		if (LIKELY(!result)) {
			// 2つ目のデータはpublish_header_t
			// result is number of received bytes or -1
			result = zmq_msg_recv(&msg, data_socket, 0);
			// reserved_leはセンサーによっては送ってこないかもしれないのでどちらでもいいように
			if ((result >= (int)(sizeof(publish_header_t) - sizeof(uint32_t)))
				&& (result <= (int)(sizeof(publish_header_t)))) {
				// receive publishing header
				publish_header_t header;
				memset(&header, 0, sizeof(publish_header_t));
				memcpy(&header, zmq_msg_data(&msg), result);
				zmq_msg_close(&msg);
				//
				result = zmq_msg_init(&msg);
				if (LIKELY(!result)) {
					// 最後に実際のフレームデータが来る
					result = zmq_msg_recv(&msg, data_socket, 0);
					if (LIKELY(result > 0)) {	// result is number of received bytes or -1
						result = handle_frame_data(identity, header, result, (const uint8_t *)zmq_msg_data(&msg));
					} else {
						LOGE("empty frame data bytes, result=%d", result);
					}
				} else {
					LOGE("zmq_msg_init failed, errno=%d", errno);
					goto ret;
				}
			} else if (result > 0) {
				LOGW("receive unexpected data:bytes=%d", result);
			}
			zmq_msg_close(&msg);
		} else {
			LOGE("zmq_msg_init failed, errno=%d", errno);
		}
	} else {
		LOGE("couldn't receive sensor identity");
	}

ret:
	return result; // RETURN(result, int);
}

//================================================================================
static void zmq_free_function(void *_data, void *hint) {
//	ENTER();

	uint8_t *data = (uint8_t *)_data;
	SAFE_DELETE_ARRAY(data);

//	EXIT();
}

/*protected*/
int Sensor::send(const std::string &msg_str, const int &flag) {

	const char *msg_chars = msg_str.c_str();
	const size_t size = strlen(msg_chars);
	LOGV("message=%s", msg_chars);
	return send((const uint8_t *)msg_chars, size, flag);
}

/*protected*/
int Sensor::send(const uint8_t *_msg_bytes, const size_t &size, const int &flag) {
	ENTER();

	int result = -1;
	if (LIKELY(_msg_bytes && size)) {
		if (LIKELY(command_socket)) {
			// copy message bytes
			uint8_t *msg_bytes = new uint8_t[size];
			if (LIKELY(msg_bytes)) {
				memcpy(msg_bytes, _msg_bytes, size);
				zmq_msg_t msg;
				result = zmq_msg_init_data(&msg, msg_bytes, size, zmq_free_function, NULL);
				if (LIKELY(!result)) {
					int bytes = zmq_msg_send(&msg, command_socket, flag);
					result = bytes != (int)size;
					if (UNLIKELY(result)) {
						zmq_msg_close(&msg);
						LOGE("zmq_msg_send failed, errno=%d", errno);
					}
				} else {
					SAFE_DELETE_ARRAY(msg_bytes);
					LOGE("zmq_msg_init_data failed, errno=%d", errno);
				}
			} else {
				LOGE("failed to copy message bytes, errno=%d", errno);
			}
		} else {
			LOGE("command socket is already destroyed or not start");
		}
	} else {
		LOGV("empty message, do nothing.");
		result = 0;
	}

	RETURN(result, int);
}

//================================================================================
/*protected*/
int Sensor::create_payload(Writer<StringBuffer> &writer) {
	ENTER();

	// do nothing

	RETURN(0, int);
}

/*protected*/
int Sensor::create_payload(Writer<StringBuffer> &writer, const request_type_t &request) {
	ENTER();

	int result = -1;
	std::string request_str = get_request_type_str(request);
	if (!request_str.empty()) {
		writer.String("action");
		writer.String(request_str.c_str());
		result = 0;
	}

	RETURN(result, int);
}

//--------------------------------------------------------------------------------
int Sensor::request_refresh_controls() {
	ENTER();

	int result = -1;
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);

	// create payload
	writer.StartObject();
	{
		result = create_payload(writer, REQUEST_REFRESH_CONTROLS);
	}
	writer.EndObject();

	if (LIKELY(!result)) {
		// send sensor identity as first part with more flag
		result = send((const uint8_t *)sensor_identity, strlen(sensor_identity), ZMQ_SNDMORE);
		if (LIKELY(!result)) {
			// send actual payload
			result = send(buffer.GetString());
		}
	}

	RETURN(result, int);
}

/*
set_control_value = {
    "action"          : "set_control_value",
    "control_id"      : <String>,
    "value"           : <value>
}
*/

/*protected*/
int Sensor::set_control_value(const std::string &control_id, const bool &value) {
	ENTER();

	int result = -1;
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);

	// create payload
	writer.StartObject();
	{
		result = create_payload(writer, REQUEST_SET_CONTROL_VALUE);
		if (LIKELY(!result)) {
			writer.String("control_id");
			writer.String(control_id.c_str());
			writer.String("value");
			writer.Bool(value);
		}
	}
	writer.EndObject();

	if (LIKELY(!result)) {
		// send sensor identity as first part with more flag
		result = send((const uint8_t *)sensor_identity, strlen(sensor_identity), ZMQ_SNDMORE);
		if (LIKELY(!result)) {
			// send actual payload
			result = send(buffer.GetString());
		}
	}

	RETURN(result, int);
}

/*protected*/
int Sensor::set_control_value(const std::string &control_id, const int &value) {
	ENTER();

	int result = -1;
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);

	// create payload
	writer.StartObject();
	{
		result = create_payload(writer, REQUEST_SET_CONTROL_VALUE);
		if (LIKELY(!result)) {
			writer.String("control_id");
			writer.String(control_id.c_str());
			writer.String("value");
			writer.Int(value);
		}
	}
	writer.EndObject();

	if (LIKELY(!result)) {
		// send sensor identity as first part with more flag
		result = send((const uint8_t *)sensor_identity, strlen(sensor_identity), ZMQ_SNDMORE);
		if (LIKELY(!result)) {
			// send actual payload
			result = send(buffer.GetString());
		}
	}

	RETURN(result, int);
}


}	// namespace sensor
}	// namespace serenegiant
