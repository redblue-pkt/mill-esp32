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
#include "esphome/core/log.h"
#include "cms79ft738.h"

const char *const log_tag = "cms79ft738"; // prefix for log messages

Cms79ft738::Cms79ft738()
{
}

unsigned char Cms79ft738::checksum(char *buf, int len)
{
	unsigned char chk = 0;
	for ( ; len != 0; len--) {
		chk += *buf++;
		//ESP_LOGD(log_tag, "setup checksum: %u", chk);
	}
	return chk;
}

Cms79ft738_ErrorCode Cms79ft738::begin()
{
	Cms79ft738_ErrorCode error = NO_ERROR;
	Wire.begin();

	return NO_ERROR;
}

Cms79ft738_ErrorCode Cms79ft738::writeData(char address, char *data, int len) {
    busy = true;

    char size[15 + 3];

    size[0] = 0x5A;

    memcpy(&size[1], data, len);

    char crc = checksum(data, len + 1);

    size[len + 1] = crc;
    size[len + 2] = 0x5B;

    Wire.beginTransmission(address);

    Wire.write(reinterpret_cast<const uint8_t*>(size), len + 3);

    Cms79ft738_ErrorCode errorCode = (Cms79ft738_ErrorCode)(-10 * Wire.endTransmission(true));

	    delay(5);

    busy = false;

    return errorCode;
}

Cms79ft738_ErrorCode Cms79ft738::readData(char address, char *data, int len)
{
	busy = true;
	char buf[len];

	Wire.requestFrom(address, len);

	int counter = 0;
	while (Wire.available() < len)
	{
		counter++;
		delay(10);
		if (counter > 250)
			return TIMEOUT_ERROR;
	}

#if 1
	Wire.readBytes(reinterpret_cast<uint8_t*>(data), len);
#else
	Wire.readBytes(buf, len);

	for (int j = 0; j < len; j++) {
		data[j] = buf[j];
		ESP_LOGD(log_tag, "read i2c data #%d 0x%x at address 0x%x", j, data[j], address);
	}
#endif
	busy = false;
	return NO_ERROR;
}
