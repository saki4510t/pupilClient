/*
 * main.cpp
 *
 *  Created on: 2017/02/04
 *      Author: saki
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <map>
#include <string>

#include <zyre.h>
#include <czmq.h>

#include "utilbase.h"
#include "app_const.h"

#include "sensor_manager.h"
#include "h264_decoder.h"

using namespace serenegiant;
using namespace serenegiant::sensor;

//================================================================================

int main(int argc, const char* argv[]) {

	av_register_all();
	SensorManager manager;

	manager.start();
	for ( ; ; ) {
		char c = getchar();
		if (c != EOF) break;
	}
	manager.stop();
}
