/*
 * Androusb
 * Copyright (c) 2014-2017 saki t_saki@serenegiant.com
 * Distributed under the terms of the GNU Lesser General Public License (LGPL v3.0) License.
 * License details are in the file license.txt, distributed as part of this software.
 */

#ifndef BINUTILS_H_
#define BINUTILS_H_

#include <vector>
#include <string>

/** intを10進文字列に変換 */
std::string i2d(int i);

/** 指定したバイト配列を16進文字列に置き換える */
std::string bin2hex(const uint8_t *data, const size_t data_bytes);

std::string bin2hex(std::vector<uint8_t> _data);

/** 16進文字列をバイト配列に変換する */
std::vector<uint8_t> hex2bin(const char *src);

/** 16進文字列をバイト配列に変換する */
std::vector<uint8_t> hex2bin(const std::string &_src);

/**
 * 文字列をバイト配列として取得する
 * @param _src
 * @param offset オフセット
 * @param max_len 最大取得バイト数, 0以下ならオフセット位置以降全て
 */
std::vector<uint8_t> str2bin(const std::string &_src, size_t offset, size_t max_len);

#endif /* BINUTILS_H_ */
