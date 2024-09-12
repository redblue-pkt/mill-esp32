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

#ifndef CMS79FT738_H
#define CMS79FT738_H

typedef enum {
	NO_ERROR = 0,
	TIMEOUT_ERROR = -100,

	// Wire I2C translated error codes
	WIRE_I2C_DATA_TOO_LOG = -10,
	WIRE_I2C_RECEIVED_NACK_ON_ADDRESS = -20,
	WIRE_I2C_RECEIVED_NACK_ON_DATA = -30,
	WIRE_I2C_UNKNOW_ERROR = -40
} Cms79ft738_ErrorCode;

using esphome::esp_log_printf_;

class Cms79ft738 {
public:
	Cms79ft738();

	Cms79ft738_ErrorCode begin();
	Cms79ft738_ErrorCode writeData(char address, char *data, int len);
	Cms79ft738_ErrorCode readData(char address, char *data, int len);
private:
	bool busy;
	unsigned char checksum(char *buf, int len);
};

#endif
