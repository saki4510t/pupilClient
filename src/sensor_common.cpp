
#include "utilbase.h"
#include "app_const.h"

#include "sensor_common.h"

//================================================================================
void json_error(rapidjson::Document &doc) {
	if (doc.HasParseError()) {
		size_t offset = doc.GetErrorOffset();
		ParseErrorCode code = doc.GetParseError();
		const char *err = GetParseError_En(code);
		LOGE("json error:%lu:%d(%s)", offset, code, err);
	}
}

// センサータイプ文字列をセンサータイプ列挙型に変換
// 未知ならSENSOR_UNKNWONを返す
sensor_type_t get_sensor_type(const std::string &sensor_type_str) {
	ENTER();

	sensor_type_t result = SENSOR_UNKNWON;
	if (sensor_type_str == "audio") {
		result = SENSOR_MIC;
	} else if (sensor_type_str == "imu") {
		result = SENSOR_IMU;
	} else if (sensor_type_str == "video") {
		result = SENSOR_UVC;
	}

	RETURN(result, sensor_type_t);
}

// サブジェクトタイプ文字列をサブジェクトタイプ列挙型に変換
// 未知ならSUBJECT_UNKNOWNを返す
subject_type_t get_subject_type(const std::string &subject_type_str) {
	ENTER();

	subject_type_t result = SUBJECT_UNKNOWN;
	if (subject_type_str == "attach") {
		result = SUBJECT_ATTACH;
	} else if (subject_type_str == "detach") {
		result = SUBJECT_DETACH;
	} else if (subject_type_str == "update") {
		result = SUBJECT_UPDATE;
	} else if (subject_type_str == "remove") {
		result = SUBJECT_REMOVE;
	} else if (subject_type_str == "error") {
		result = SUBJECT_ERROR;
	}

	RETURN(result, subject_type_t);
}

const std::string get_request_type_str(const request_type_t request) {
	ENTER();

	std::string result;
	switch (request) {
	case REQUEST_REFRESH_CONTROLS:
		result = "refresh_controls";
		break;
	case REQUEST_SET_CONTROL_VALUE:
		result = "set_control_value";
		break;
	}

	RET(result);
}

//================================================================================
/**
 *enum Type {
 *  kNullType = 0,      //!< null
 *  kFalseType = 1,     //!< false
 *  kTrueType = 2,      //!< true
 *  kObjectType = 3,    //!< object
 *  kArrayType = 4,     //!< array
 *  kStringType = 5,    //!< string
 *  kNumberType = 6     //!< number
 *};
 */
const enum rapidjson::Type json_get_type(rapidjson::Document &doc, const char *name) {

	ENTER();

	enum rapidjson::Type result = kNullType;
	if (LIKELY(!doc.HasParseError())) {
		Value::ConstMemberIterator itr = doc.FindMember(name);
		if (itr != doc.MemberEnd()) {
			const Value &v = itr->value;
			result = v.GetType();
		} else {
			LOGW("json does not have the filed");
		}
	} else {
		json_error(doc);
	}


	RET(result);
}

const char *json_get_string(rapidjson::Document &doc, const char *name) {

	ENTER();

	const char *result = NULL;
	if (LIKELY(!doc.HasParseError())) {
		Value::ConstMemberIterator itr = doc.FindMember(name);
		if (itr != doc.MemberEnd()) {
			const Value &v = itr->value;
			if (v.IsString()) {
				result = v.GetString();
			} else {
				LOGW("filed type of specific filed is not a string");
			}
		} else {
			LOGW("json does not have the filed");
		}
	} else {
		json_error(doc);
	}


	RET(result);
}

const bool json_get_bool(rapidjson::Document &doc, const char *name) {

	ENTER();

	bool result = false;
	if (LIKELY(!doc.HasParseError())) {
		Value::ConstMemberIterator itr = doc.FindMember(name);
		if (itr != doc.MemberEnd()) {
			const Value &v = itr->value;
			if (v.IsBool()) {
				result = v.GetBool();
			} else {
				LOGW("filed type of specific filed is not a bool");
			}
		} else {
			LOGW("json does not have the filed");
		}
	} else {
		json_error(doc);
	}


	RET(result);
}

const int json_get_int(rapidjson::Document &doc, const char *name) {

	ENTER();

	int result = 0;
	if (LIKELY(!doc.HasParseError())) {
		Value::ConstMemberIterator itr = doc.FindMember(name);
		if (itr != doc.MemberEnd()) {
			const Value &v = itr->value;
			if (v.IsInt()) {
				result = v.GetInt();
			} else {
				LOGW("filed type of specific filed is not a int");
			}
		} else {
			LOGW("json does not have the filed");
		}
	} else {
		json_error(doc);
	}


	RET(result);
}

const long json_get_long(rapidjson::Document &doc, const char *name) {

	ENTER();

	long result = 0;
	if (LIKELY(!doc.HasParseError())) {
		Value::ConstMemberIterator itr = doc.FindMember(name);
		if (itr != doc.MemberEnd()) {
			const Value &v = itr->value;
			if (v.IsInt64()) {
				result = v.GetInt64();
			} else if (v.IsInt()) {
				result = v.GetInt();
			} else {
				LOGW("filed type of specific filed is not a long");
			}
		} else {
			LOGW("json does not have the filed");
		}
	} else {
		json_error(doc);
	}


	RET(result);
}

