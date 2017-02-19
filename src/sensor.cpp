/*
 * sensor.cpp
 *
 *  Created on: 2017/02/16
 *      Author: saki
 */

#if 0	// set 0 if you need debug log, otherwise set 1
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

Sensor::Sensor(const sensor_type_t &_sensor_type, const char *uuid, const char *name)
:	sensor_type(_sensor_type),
	sensor_uuid(uuid),
	sensor_name(name),
	zmq_context(NULL),
	command_socket(NULL),
	notify_socket(NULL),
	data_socket(NULL),
	zmq_thread() {

	ENTER();

	LOGI("Sensor#Constructor");

	EXIT();
}

Sensor::~Sensor() {
	ENTER();

	LOGI("Sensor#Destructor");
	stop();

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

	int result = zmq_setsockopt(zmq_socket, ZMQ_LINGER, (const void *)&LINGER, sizeof(LINGER));
	if (UNLIKELY(result)) {
		LOGE("failed to set connect timeout:errno=%d", errno);
		goto ret;
	}
	result = zmq_setsockopt(zmq_socket, ZMQ_CONNECT_TIMEOUT, (const void *)&CONNECT_TIMEOUT, sizeof(CONNECT_TIMEOUT));
	if (UNLIKELY(result)) {
		LOGE("failed to set connect timeout:errno=%d", errno);
		goto ret;
	}
	result = zmq_setsockopt(zmq_socket, ZMQ_RCVTIMEO, (const void *)&RCV_TIMEOUT, sizeof(RCV_TIMEOUT));
	if (UNLIKELY(result)) {
		LOGE("failed to set rcv timeout:errno=%d", errno);
		goto ret;
	}
	result = zmq_setsockopt(zmq_socket, ZMQ_SNDTIMEO, (const void *)&SND_TIMEOUT, sizeof(SND_TIMEOUT));
	if (UNLIKELY(result)) {
		LOGE("failed to set send timeout:errno=%d", errno);
		goto ret;
	}
	result = zmq_setsockopt(zmq_socket, ZMQ_RCVHWM, (const void *)&RCV_HWM, sizeof(RCV_HWM));
	if (UNLIKELY(result)) {
		LOGE("failed to set rcv high water mark:errno=%d", errno);
		goto ret;
	}
	result = zmq_setsockopt(zmq_socket, ZMQ_SNDHWM, (const void *)&SND_HWM, sizeof(SND_HWM));
	if (UNLIKELY(result)) {
		LOGE("failed to set send high water mark:errno=%d", errno);
		goto ret;
	}

ret:
	RETURN(result, int);
}

/*public*/
int Sensor::start(void *_zmq_context,
	const char *command, const char *notify, const char *data) {

	ENTER();

	int result = 0;
	if (!isRunning()) {
		if (LIKELY(_zmq_context && command && notify && data)) {
			zmq_context = _zmq_context;
			command_endpoint = command;
			notify_endpoint = notify;
			data_endpoint = data;
			const char *subscription = sensor_uuid.c_str();
			const int subscription_size = strlen(subscription);

			command_socket = zmq_socket(_zmq_context, ZMQ_PUSH);
			if (LIKELY(command_socket)) {
				result = setup_socket(command_socket);
				if (UNLIKELY(result)) {
					goto err;
				}
				result = zmq_connect(command_socket, command);
				if (UNLIKELY(result)) {
					LOGE("failed to connect to command:errno=%d", errno);
					goto err;
				}
			} else {
				LOGE("failed to create command socket:errno=%d", errno);
				goto err;
			}

			notify_socket = zmq_socket(_zmq_context, ZMQ_SUB);
			if (LIKELY(notify_socket)) {
				result = setup_socket(notify_socket);
				if (UNLIKELY(result)) {
					goto err;
				}
				result = zmq_connect(notify_socket, notify);
				if (UNLIKELY(result)) {

					LOGE("failed to connect to notify:errno=%d", errno);
					goto err;
				}
				result = zmq_setsockopt(notify_socket, ZMQ_SUBSCRIBE, subscription, subscription_size);
				if (UNLIKELY(result)) {
					LOGE("failed to set subscription:errno=%d, subscription=%s", errno, subscription);
//					goto err;
				}
			} else {
				LOGE("failed to create notify socket:errno=%d", errno);
				goto err;
			}

			data_socket = zmq_socket(_zmq_context, ZMQ_SUB);
			if (LIKELY(data_socket)) {
				result = setup_socket(data_socket);
				if (UNLIKELY(result)) {
					zmq_close(data_socket);
					data_socket = NULL;
					goto err;
				}
				result = zmq_connect(data_socket, data);
				if (UNLIKELY(result)) {
					zmq_close(data_socket);
					data_socket = NULL;
					LOGE("failed to connect to data:errno=%d", errno);
					goto err;
				}
				result = zmq_setsockopt(data_socket, ZMQ_SUBSCRIBE, subscription, subscription_size);
				if (UNLIKELY(result)) {
					LOGE("failed to set subscription:errno=%d, subscription=%s", errno, subscription);
//					goto err;
				}
			} else {
				LOGE("failed to create data socket:errno=%d", errno);
				goto err;
			}


		} else {
			LOGE("zmq context / command / notify / data should not be null");
		}

		is_running = true;
		result = pthread_create(&zmq_thread, NULL, zmq_thread_func, (void *) this);

		if (UNLIKELY(result)) {
			stop();
		}
	}

	RETURN(result, int);
err:
	zmq_stop();
	RETURN(result, int);
}

/*public*/
int Sensor::stop() {
	ENTER();

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

/*protected*/
void Sensor::zmq_stop() {
	ENTER();

	if (zmq_context) {
		if (data_socket) {
			zmq_setsockopt(data_socket, ZMQ_UNSUBSCRIBE, sensor_uuid.c_str(), sensor_uuid.size());
			zmq_disconnect(data_socket, data_endpoint.c_str());
			zmq_close(data_socket);
			data_socket = NULL;
			if (notify_socket) {
				zmq_setsockopt(notify_socket, ZMQ_UNSUBSCRIBE, sensor_uuid.c_str(), sensor_uuid.size());
				zmq_disconnect(notify_socket, notify_endpoint.c_str());
				zmq_close(notify_socket);
				notify_socket = NULL;
			}
			if (command_socket) {
				zmq_disconnect(command_socket, command_endpoint.c_str());
				zmq_close(command_socket);
				command_socket = NULL;
			}
		} else {
			if (notify_socket) {
				zmq_close(notify_socket);
				notify_socket = NULL;
				if (command_socket) {
					zmq_disconnect(command_socket, notify_endpoint.c_str());
					zmq_close(command_socket);
					command_socket = NULL;
				}
			} else {
				if (command_socket) {
					zmq_close(command_socket);
					command_socket = NULL;
				}
			}
		}
	} else {
		command_socket = notify_socket = data_socket = NULL;
	}
	zmq_context = NULL;

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

int Sensor::receive_notify() {
	ENTER();

	zmq_msg_t msg;
	int result = zmq_msg_init(&msg);
	if (LIKELY(!result)) {
		result = zmq_msg_recv(&msg, notify_socket, 0);
		if (LIKELY(result > 0)) {	// result is number of received bytes or -1
			result = on_receive_notify(msg);
		}
		zmq_msg_close(&msg);
	}

	RETURN(result, int);
}

int Sensor::receive_data() {
	ENTER();

	zmq_msg_t msg;
	int result = zmq_msg_init(&msg);
	if (LIKELY(!result)) {
		result = zmq_msg_recv(&msg, data_socket, 0);
		if (result >= (int)sizeof(publish_header_t)) {	// result is number of received bytes or -1
			// receive publishing header
			publish_header_t header;
			memcpy(&header, zmq_msg_data(&msg), sizeof(publish_header_t));
			zmq_msg_close(&msg);
			//
			result = zmq_msg_init(&msg);
			if (LIKELY(!result)) {
				result = zmq_msg_recv(&msg, data_socket, 0);
				if (LIKELY(result > 0)) {	// result is number of received bytes or -1
					result = on_receive_data(header, msg);
				}
			}
		} else if (result > 0){
			LOGW("receive unexpected data:bytes=%d", result);
		}
		zmq_msg_close(&msg);
	}

	RETURN(result, int);
}

/*private*/
void Sensor::zmq_run() {
	ENTER();

	zmq_pollitem_t items[2];
	items[0].socket = notify_socket;
	items[0].events = ZMQ_POLLIN;
	items[1].socket = data_socket;
	items[1].events = ZMQ_POLLIN;

	for ( ; isRunning() ; ) {
		int ix = zmq_poll(items, 2, 100);
		if (ix > 0) {
			if ((items[0].revents & ZMQ_POLLIN) == ZMQ_POLLIN) {
				// notify_socket ready to receive
				receive_notify();
			}
			if ((items[1].revents & ZMQ_POLLIN) == ZMQ_POLLIN) {
				// data socket ready to receive
				receive_data();
			}
		}
	}
	is_running = false;

	EXIT();
}

}	// namespace sensor
}	// namespace serenegiant
