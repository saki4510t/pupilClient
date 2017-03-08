/*
 * ffmpeg_utils.h
 *
 *  Created on: 2017/03/08
 *      Author: saki
 */

#ifndef UTILS_FFMPEG_UTILS_H_
#define UTILS_FFMPEG_UTILS_H_

#include <string>

namespace serenegiant {
namespace media {

std::string av_error(const int errnum);

}	// namespace media
}	// namespace serenegiant

#endif /* UTILS_FFMPEG_UTILS_H_ */
