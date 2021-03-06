/*
 * app_const.h
 *
 *  Created on: 2017/02/17
 *      Author: saki
 */

#ifndef APP_CONST_H_
#define APP_CONST_H_

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <map>
#include <string>
#include <typeinfo>
#include <iostream>

#include <zmq.h>
#include <czmq.h>
#include <zyre.h>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

#include "mutex.h"
#include "condition.h"
#include "endian_unaligned.h"
#include "timers.h"

#include "binutils.h"
#include "ffmpeg_utils.h"
#include "h264_utils.h"

using namespace android;
using namespace rapidjson;

#endif /* APP_CONST_H_ */
