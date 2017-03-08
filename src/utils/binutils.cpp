/*
 * Androusb
 * Copyright (c) 2014-2017 saki t_saki@serenegiant.com
 * Distributed under the terms of the GNU Lesser General Public License (LGPL v3.0) License.
 * License details are in the file license.txt, distributed as part of this software.
 */

#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "utilbase.h"
#include "binutils.h"

#define H2B(a) (a <= '9' ? a - '0' : (a <= 'Z' ? a - 'A' + 0x0a : a <= 'z' ? a - 'a' + 0x0a : 0))

static const char HEX_CHARS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

/** intを10進文字列に変換 */
std::string i2d(int i) {
	std::stringstream ss;
	if (i < 0) {
		ss << '-';
		i = -i;
	}
	do {
		ss << (char)((i % 10) + '0');
		i /= 10;
	} while (i > 0);
	return ss.str();
}

/**
 * 指定したバイト配列を16進文字列に置き換える
 */
std::string bin2hex(const uint8_t *data, const size_t data_bytes) {
//	std::string result;
	std::stringstream ss;

	for (uint32_t i = 0; i < data_bytes; i++) {
		ss << HEX_CHARS[(data[i] & 0xf0) >> 4]; // result += HEX_CHARS[(data[i] & 0xf0) >> 4];
		ss << HEX_CHARS[data[i] & 0xf]; 		// result += HEX_CHARS[data[i] & 0xf];
	}

	return ss.str(); // result;
}

std::string bin2hex(std::vector<uint8_t> _data) {
	const size_t data_bytes = _data.size();
	const uint8_t *data = &_data[0];
	return bin2hex(data, data_bytes);
}

/**
 * 16進文字列をバイト配列に変換する
 */
std::vector<uint8_t> hex2bin(const char *src) {
	const int n = src ? strlen(src) : 0;
	std::vector<uint8_t> result = std::vector<uint8_t>(n / 2);
	int ix = 0;
	for (int i = 0; i < n; i += 2) {
		result[ix++] = (uint8_t)(H2B(src[i]) * 16 + H2B(src[i + 1]));
	}
	return result;
}

/**
 * 16進文字列をバイト配列に変換する
 */
std::vector<uint8_t> hex2bin(const std::string &_src) {
	const int n = _src.size();
	const char *src = _src.c_str();

	std::vector<uint8_t> result;
	if (LIKELY(n >= 2)) {
		result = std::vector<uint8_t>(n / 2);
		int ix = 0;
		for (int i = 0; i < n; i += 2) {
			result[ix++] = (uint8_t)(H2B(src[i]) * 16 + H2B(src[i + 1]));
		}
	}
	return result;
}

/**
 * 文字列をバイト配列として取得する
 * @param _src
 * @param offset オフセット
 * @param max_len 最大取得バイト数, 0以下ならオフセット位置以降全て
 */
std::vector<uint8_t> str2bin(const std::string &_src, size_t offset, size_t max_len) {
	const size_t size = _src.size();
	if (max_len <= 0)
		max_len = size;
	if (offset + max_len > size)
		max_len = size - offset;
	std::vector<uint8_t> result;
	if (LIKELY(max_len)) {
		result = std::vector<uint8_t>(max_len);
		memcpy(&result[0], &_src.data()[offset], max_len);
	}
	return result;
}
