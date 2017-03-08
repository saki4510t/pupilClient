/*
 * sensor_common.h
 *
 *  Created on: 2017/02/19
 *      Author: saki
 */

#ifndef SENSOR_COMMON_H_
#define SENSOR_COMMON_H_

#include "app_const.h"

typedef enum sensor_type {
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

typedef enum request_type {
	REQUEST_REFRESH_CONTROLS = 0,
	REQUEST_SET_CONTROL_VALUE = 1,
} request_type_t;

typedef enum uvc_frame_format {
	VIDEO_FRAME_FORMAT_UNKNOWN     = 0,     // supported, unknown
	VIDEO_FRAME_FORMAT_YUYV        = 0x01,  // supported, YUYV
//	(VIDEO_FRAME_FORMAT_UYVY        = 0x02)
//	(VIDEO_FRAME_FORMAT_GRAY8       = 0x03)
//	(VIDEO_FRAME_FORMAT_BY8         = 0x04)
//	(VIDEO_FRAME_FORMAT_NV21        = 0x05)
//	(VIDEO_FRAME_FORMAT_YV12        = 0x06)
//	(VIDEO_FRAME_FORMAT_I420        = 0x07)
//	(VIDEO_FRAME_FORMAT_Y16         = 0x08)
//	(VIDEO_FRAME_FORMAT_RGBP        = 0x09)
//	(VIDEO_FRAME_FORMAT_M420        = 0x0a)
//	(VIDEO_FRAME_FORMAT_NV12        = 0x0b)
//	(VIDEO_FRAME_FORMAT_RGB565      = 0x0c)
//	(VIDEO_FRAME_FORMAT_RGB         = 0x0d)
//	(VIDEO_FRAME_FORMAT_BGR         = 0x0e)
//	(VIDEO_FRAME_FORMAT_RGBX        = 0x0f)
	 VIDEO_FRAME_FORMAT_MJPEG       = 0x10,  // supported, MJPEG
//	(VIDEO_FRAME_FORMAT_MPEG2TS     = 0x11)
	 VIDEO_FRAME_FORMAT_H264        = 0x12,  // supported, H264
	 VIDEO_FRAME_FORMAT_VP8         = 0x13,  // supported, VP8
//	(VIDEO_FRAME_FORMAT_YCbCr       = 0x14)
//	(VIDEO_FRAME_FORMAT_FRAME_H264  = 0x15)
//	(VIDEO_FRAME_FORMAT_FRAME_VP8   = 0x16)
//	(VIDEO_FRAME_FORMAT_DV          = 0x17)

} uvc_frame_format_t;

// 28バイト
typedef struct mic_publish_header {
    uint32_t format_le;		// 0:PCM8, 1:PCM16
    uint32_t channel_le;
    uint32_t sequence_le;
    int64_t presentation_time_us_le;
    uint32_t data_bytes_le;
    uint32_t reserved_le;
} __attribute__ ((packed)) mic_publish_header_t;

// 28バイト
typedef struct imu_publish_header {
    uint32_t format_le;		// always 0
    uint32_t channel_le;	// always 3
    uint32_t sequence_le;
    int64_t presentation_time_us_le;
    uint32_t data_bytes_le;
    uint32_t reserved_le;
} __attribute__ ((packed)) imu_publish_header_t;

// 32バイト
typedef struct uvc_publish_header {
    uint32_t format_le; // MJPEG, H264, (YUYV, VP8)
    uint32_t width_le;
    uint32_t height_le;
    uint32_t sequence_le;
    int64_t presentation_time_us_le;
    uint32_t data_bytes_le;
    uint32_t reserved_le;
} __attribute__ ((packed)) uvc_publish_header_t;


typedef struct publish_header {
	union {
		mic_publish_header_t mic;
		imu_publish_header_t imu;
		uvc_publish_header_t uvc;
	};
} publish_header_t;

typedef enum value_type {
	VALUE_TYPE_UNKNOWN = 0,
	VALUE_TYPE_BOOL,
	VALUE_TYPE_INT,
	VALUE_TYPE_STRING,
	VALUE_TYPE_MAP,
} value_type_t;

typedef struct control_value {
	value_type_t value_type;
	std::string current;	// json string
	union {
		bool bool_value;
		int int_value;
		std::string string_value;
	};
} control_value_t;

extern void json_error(rapidjson::Document &doc);
extern sensor_type_t get_sensor_type(const std::string &sensor_type_str);
extern subject_type_t get_subject_type(const std::string &subject_type_str);
extern const std::string get_request_type_str(const request_type_t request);

extern const enum rapidjson::Type json_get_type(rapidjson::Document &doc, const char *name);
extern const char *json_get_string(rapidjson::Document &doc, const char *name);
extern const bool json_get_bool(rapidjson::Document &doc, const char *name);
extern const int json_get_int(rapidjson::Document &doc, const char *name);
extern const long json_get_long(rapidjson::Document &doc, const char *name);



#endif /* SENSOR_COMMON_H_ */
