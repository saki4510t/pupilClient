/*
 * sensor_common.h
 *
 *  Created on: 2017/02/19
 *      Author: saki
 */

#ifndef SENSOR_COMMON_H_
#define SENSOR_COMMON_H_

#include "app_const.h"

typedef enum _sensor_type {
	SENSOR_UNKNWON = 0,
	SENSOR_MIC,
	SENSOR_IMU,
	SENSOR_UVC,
} sensor_type_t;

typedef enum _subject_type {
	SUBJECT_UNKNOWN = 0,
	SUBJECT_ATTACH,
	SUBJECT_DETACH,
	SUBJECT_UPDATE,
	SUBJECT_REMOVE,
	SUBJECT_ERROR,

} subject_type_t;

extern void json_error(rapidjson::Document &doc);
extern sensor_type_t get_sensor_type(const std::string &sensor_type_str);
extern subject_type_t get_subject_type(const std::string &subject_type_str);

extern const enum rapidjson::Type json_get_type(rapidjson::Document &doc, const char *name);
extern const char *json_get_string(rapidjson::Document &doc, const char *name);
extern const bool json_get_bool(rapidjson::Document &doc, const char *name);
extern const int json_get_int(rapidjson::Document &doc, const char *name);
extern const long json_get_long(rapidjson::Document &doc, const char *name);



#endif /* SENSOR_COMMON_H_ */
