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

#ifndef CMS79FT738_KEY_H
#define CMS79FT738_KEY_H

#define KEY_PLUS 0x22
#define KEY_MINUS 0x11
#define KEY_WIFI 0x88
#define KEY_SETTINGS 0x44
#define KEY_NONE 0x00

using esphome::esp_log_printf_;

class Cms79ft738_Key {
public:
        Cms79ft738 cms79ft738;
	Cms79ft738_Key();
	Cms79ft738_ErrorCode begin(char address, char key_plus, char key_minus, char key_wifi, char key_settings);
	char readData(char address, char *data, int len);
	char updateKey();
	const char *key2string(int key);
	int readKey();
private:
	char _address;
	char _key_plus;
	char _key_minus;
	char _key_wifi;
	char _key_settings;
};

#endif
