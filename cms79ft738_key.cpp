/*
 * Copyright (C) 2024 RedBlue
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "esphome.h"
#include <bitset>
#include "cms79ft738.h"
#include "cms79ft738_key.h"

static const char *const log_tag = "cms79ft738_key"; // prefix for log messages

//static char baseArray[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static char baseArray[] = { 0x00, 0x00, 0x00, 0x00 };

Cms79ft738_Key::Cms79ft738_Key()
{
}

Cms79ft738_ErrorCode Cms79ft738_Key::begin(char address, char key_plus, char key_minus, char key_wifi, char key_settings)
{
	Cms79ft738_ErrorCode error = NO_ERROR;
	_address = address;
	_key_plus = key_plus;
	_key_minus = key_minus;
	_key_wifi = key_wifi;
	_key_settings = key_settings;

	return NO_ERROR;
}

char Cms79ft738_Key::readData(char address, char *data, int len)
{
	return cms79ft738.readData(_address, data, len);
}

char Cms79ft738_Key::updateKey()
{
	return readData(_address, baseArray, sizeof(baseArray));
}

const char *Cms79ft738_Key::key2string(int key)
{
	const char *result;

	switch (key) {
		case KEY_MINUS:
			result = "minus";
			break;
		case KEY_PLUS:
			result = "plus";
			break;
		case KEY_WIFI:
			result = "wifi";
			break;
		case KEY_SETTINGS:
			result = "settings";
			break;
		default:
			result = "none";
			break;
	}

	return result;
}

int Cms79ft738_Key::readKey() {
	int ret;

	updateKey();

	switch (baseArray[2]) {
		case KEY_MINUS:
			ret = KEY_MINUS;
			break;
		case KEY_PLUS:
			ret = KEY_PLUS;
			break;
		case KEY_WIFI:
			ret = KEY_WIFI;
			break;
		case KEY_SETTINGS:
			ret = KEY_SETTINGS;
			break;
		default:
			ret = KEY_NONE;
		break;
	}

	return ret;
}
