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
#include "cms79ft738_led.h"

static const char *const log_tag = "cms79ft738_led"; // prefix for log messages
                   /*   0     1     2     3     4     5     6     7     8     9    10    11    12 */
static char baseArray[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

Cms79ft738_Led::Cms79ft738_Led()
{
}

Cms79ft738_ErrorCode Cms79ft738_Led::begin(char address, char digit1, char  digit2, char digit3, char icon, char lines, char brightness)
{
	Cms79ft738_ErrorCode error = NO_ERROR;
	_address = address;
	_digit1 = digit1;
	_digit2 = digit2;
	_digit3 = digit3;
	_icon = icon;
	_lines = lines;
	_brightness = brightness;

	heating_icon = false;
	power_icon = false;
	wifi_green_icon = false;
	wifi_yellow_icon = false;
	wifi_red_icon = false;
	wifi_icon = false;
	settings_icon = false;
	minus_icon = false;
	plus_icon = false;

	x_icon = false;

        return NO_ERROR;
}

Cms79ft738_ErrorCode Cms79ft738_Led::writeData(char *data, int len)
{
	data[7] = _digit1;
	data[8] = _digit2;
	data[9] = _digit3;
	data[10] = _icon;
	data[11] = _lines;
	data[12] = _brightness;
	data[14] = _brightness;

	return cms79ft738.writeData(_address, data, len);
}

Cms79ft738_ErrorCode Cms79ft738_Led::updateLed()
{
	return writeData(baseArray, sizeof(baseArray));
}

void Cms79ft738_Led::setDigit1(char digit1)
{
	_digit1 = digit1;
}

void Cms79ft738_Led::setDigit2(char digit2)
{
	_digit2 = digit2;
}

void Cms79ft738_Led::setDigit3(char digit3)
{
	_digit3 = digit3;
}

void Cms79ft738_Led::setChar1(char char1)
{
	_digit1 = char1;
}

void Cms79ft738_Led::setChar2(char char2)
{
	_digit2 = char2;
}

void Cms79ft738_Led::setChar3(char char3)
{
	_digit3 = char3;
}

char Cms79ft738_Led::setIconTranslationLayer()
{
	char iconValue;

	if (wifi_green_icon) {
		iconValue |= ICON_GREEN_WIFI_ON;
	} else {
		iconValue &= ~ICON_GREEN_WIFI_ON;
	}

	if (power_icon) {
		iconValue |= ICON_POWER_ON;
	} else {
		iconValue &= ~ICON_POWER_ON;
	}

	if (heating_icon) {
		iconValue |= ICON_HEATING_ON;
	} else {
		iconValue &= ~ICON_HEATING_ON;
	}

	if (plus_icon) {
		iconValue |= ICON_PLUS_ON;
	} else {
		iconValue &= ~ICON_PLUS_ON;
	}

	if (minus_icon) {
		iconValue |= ICON_MINUS_ON;
	} else {
		iconValue &= ~ICON_MINUS_ON;
	}

	if (wifi_icon) {
		iconValue |= ICON_WIFI_INDICATOR_ON;
	} else {
		iconValue &= ~ICON_WIFI_INDICATOR_ON;
	}

	if (settings_icon) {
		iconValue |= ICON_SETTINGS_ON;
	} else {
		iconValue &= ~ICON_SETTINGS_ON;
	}

	if (x_icon) {
		iconValue |= ICON_5_LINES_ON;
	} else {
		iconValue &= ~ICON_5_LINES_ON;
	}

	return iconValue;
}

void Cms79ft738_Led::setIcon() {
    _icon = setIconTranslationLayer();
}

void Cms79ft738_Led::setIconWifi(bool wifi) {
	wifi_icon = wifi;
	setIcon();
}

void Cms79ft738_Led::setIconSettings(bool settings) {
	settings_icon = settings;
	setIcon();
}

void Cms79ft738_Led::setIconMinus(bool minus) {
	minus_icon = minus;
	setIcon();
}

void Cms79ft738_Led::setIconPlus(bool plus) {
	plus_icon = plus;
	setIcon();
}

void Cms79ft738_Led::setIconHeating(bool heating) {
	heating_icon = heating;
	setIcon();
}

void Cms79ft738_Led::setIconPower(bool power) {
	power_icon = power;
	setIcon();
}

void Cms79ft738_Led::setIconWifiGreen(bool wifi_green) {
	wifi_green_icon = wifi_green;
	setIcon();
}

void Cms79ft738_Led::setIconX(bool x) {
        x_icon = x;
        setIcon();
}

bool Cms79ft738_Led::getIconSettings() {
	return settings_icon;
}

bool Cms79ft738_Led::getIconMinus() {
	return minus_icon;
}

bool Cms79ft738_Led::getIconPlus() {
	return plus_icon;
}

bool Cms79ft738_Led::getIconHeating() {
	return heating_icon;
}

bool Cms79ft738_Led::getIconPower() {
	return power_icon;
}

bool Cms79ft738_Led::getIconWifiGreen() {
	return wifi_green_icon;
}

bool Cms79ft738_Led::getIconWifiRed() {
	return wifi_red_icon;
}

void Cms79ft738_Led::setLines(char lines)
{
	_lines = lines;
}

char Cms79ft738_Led::setBrightnessTranslationLayer(int brightness)
{
	char response;

	response = brightness * 17;

	return response;
}

void Cms79ft738_Led::setBrightness(int brightness)
{
	_brightness = setBrightnessTranslationLayer(brightness);
}

char Cms79ft738_Led::setDigitTranslationLayer(int digit) {
	char response = 0;

	switch (digit) {
		case 0:
			response = DIGIT_0;
			break;
		case 1:
			response = DIGIT_1;
			break;
		case 2:
			response = DIGIT_2;
			break;
		case 3:
			response = DIGIT_3;
			break;
		case 4:
			response = DIGIT_4;
			break;
		case 5:
			response = DIGIT_5;
			 break;
		case 6:
			response = DIGIT_6;
			break;
		case 7:
			response = DIGIT_7;
			break;
		case 8:
			response = DIGIT_8;
			break;
		case 9:
			response = DIGIT_9;
			break;
		default:
			break;
	}

	if (wifi_red_icon == true) {
		response |= 0x80;
	}

	return response;
}

void Cms79ft738_Led::setTemperature(float temperature)
{
	/* ab.c */
	int a, b, c, d, e;

	float temp = (float) temperature;

	a = temp * 10;
	b = a / 100 % 10;
	c = a / 10 % 10;
	d = a % 10;

	setDigit1(setDigitTranslationLayer(b));
	setDigit2(setDigitTranslationLayer(c));
	setDigit3(setDigitTranslationLayer(d));
}

char Cms79ft738_Led::setCharTranslationLayer(char character)
{
	char response = 0;

	switch (character) {
		case 'A':
			response = CHAR_A;
			break;
		case 'C':
			response = CHAR_C;
			break;
		case 'E':
			response = CHAR_E;
			break;
		case 'F':
			response = CHAR_F;
			break;
		case 'H':
			response = CHAR_H;
			break;
		case 'O':
			response = DIGIT_0;
			break;
		case 'P':
			response = CHAR_P;
			break;
		case 'U':
			response = CHAR_U;
			break;
		case '-':
			response = CHAR_MINUS;
			break;
		case ' ':
			response = CHAR_SPACE;
			break;
	}

	return response;
}

void Cms79ft738_Led::setOffDisplay()
{
	char firstChar = 'O';
	char secondChar = 'F';

	setDigit1(setCharTranslationLayer(firstChar));
	setDigit2(setCharTranslationLayer(secondChar));
}

void Cms79ft738_Led::setFDisplay()
{
	char firstChar = ' ';
	char secondChar = 'F';

	setDigit1(setCharTranslationLayer(firstChar));
	setDigit2(setCharTranslationLayer(secondChar));
}

void Cms79ft738_Led::setCDisplay()
{
	char firstChar = ' ';
	char secondChar = 'C';

	setDigit1(setCharTranslationLayer(firstChar));
	setDigit2(setCharTranslationLayer(secondChar));
}

