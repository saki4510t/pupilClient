/*
 * main.cpp
 *
 *  Created on: 2017/02/04
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

#include <stdio.h>

#include "utilbase.h"
#include "app_const.h"

#include "sensor_manager.h"
#include "h264_decoder.h"

//================================================================================

int main(int argc, const char* argv[]) {

	ENTER();

	av_register_all();

	// zyre経由で送られてくるセンサー情報をハンドリングする

	serenegiant::sensor::SensorManager manager;

	system("stty -echo -icanon min 1 time 0");
	manager.start();
	for ( ; manager.isRunning() ; ) {
		// 何か入力するまで実行する
		const char c = getchar();
		if ((c == 'r') || (c == 'R')) {
			// r|Rなら録画開始/停止
			if (manager.isRecording()) {
				manager.stop_recording();
			} else {
				manager.start_recording();
			}
			continue;
		}
		if (c != EOF) break;
	}
	manager.stop();
	system("stty echo -icanon min 1 time 0");

	RETURN(0, int);
}
