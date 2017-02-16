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

//================================================================================

int main(int argc, const char* argv[]) {

	av_register_all();

	// zyre経由で送られてくるセンサー情報をハンドリングする

	serenegiant::sensor::SensorManager manager;

	manager.start();
	for ( ; ; ) {
		// 何か入力するまで実行する
		char c = getchar();
		if (c != EOF) break;
	}
	manager.stop();
}
