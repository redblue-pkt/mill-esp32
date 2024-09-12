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

#ifndef CMS79FT738_LED_H
#define CMS79FT738_LED_H

// digit 0x06 = 1
// digit 0x07 = 7
// digit 0x08 = _
// digit 0x39 = C
// digit 0x3e = U
// digit 0x3f = 0
// digit 0x40 = -
// digit 0x4f = 3
// digit 0x5b = 2
// digit 0x63 stopnie c/f
// digit 0x66 = 4
// digit 0x6d = 5
// digit 0x6f = 9
// digit 0x71 = F
// digit 0x73 = P
// digit 0x76 = H
// digit 0x77 = A
// digit 0x79 = E
// digit 0x7d = 6
// digit 0x7f = 8
// digit 0x80 = red wifi

// 0xff value 255, green wifi, power, lightning, +, -, wifi, settings, 5 lines X
// 0xfe value 254, green wifi, power, lightning, +, -, wifi, settings, 4 lines
// 0xfd value 253, green wifi, lightning, +, -, wifi, settings, 5 lines X
// 0xfc value 252, green wifi, lightning, +, -, wifi, settings, 4 lines
// 0xfb value 251, green wifi, power, +, -, wifi, settings, 4 lines
// 0xfa value 250, green wifi, power, +, -, wifi, settings, 3 lines
// 0xf9 value 249, green wifi, +, -, wifi, settings, 5 lines X
// 0xf8 value 248, green wifi, +, -, wifi, settings, 4 lines

// 0x7f value 127 power, lightning, +, -, wifi, settings, 5 lines X
// 0x7e value 126 power, lightning, +, -, wifi, settings, 4 lines
// 0x7d value 125 lightning, +, -, wifi, settings, 5 lines X
// 0x7c value 124 lightning, +, -, wifi, settings, 4 lines
// 0x7b value 123 power, +, -, wifi, settings, 4 lines
// 0x7a value 122 power, +, -, wifi, settings, 3 lines
// 0x79 value 121 +, -, wifi, settings, 5 lines X
// 0x78 value 120 +, -, wifi, settings, 4 lines

// 0 1 2 3 4 5 6 7
// X X X X X X X X
// 3 S W P M 2 1 4
//  S - icon settings
//  W - icon wifi
//  P - plus icon
//  M - minus icon
//  1 - power icon
//  2 - heat icon
//  3 - wifi green icon
//  4 - nothing ??

#define ICON_GREEN_WIFI_ON        (1 << 7)
#define ICON_SETTINGS_ON          (1 << 6)
#define ICON_WIFI_INDICATOR_ON    (1 << 5)
#define ICON_PLUS_ON              (1 << 4)
#define ICON_MINUS_ON             (1 << 3)
#define ICON_HEATING_ON           (1 << 2)
#define ICON_POWER_ON             (1 << 1)
#define ICON_5_LINES_ON           (1 << 0)

#define DIGIT_0    0x3F
#define DIGIT_1    0x06
#define DIGIT_2    0x5B
#define DIGIT_3    0x4F
#define DIGIT_4    0x66
#define DIGIT_5    0x6D
#define DIGIT_6    0x7D
#define DIGIT_7    0x07
#define DIGIT_8    0x7F
#define DIGIT_9    0x6F

#define CHAR_A     0x77
#define CHAR_C     0x39
#define CHAR_E     0x79
#define CHAR_F     0x71
#define CHAR_H     0x76
#define CHAR_P     0x73
#define CHAR_U     0x3E
#define CHAR_SPACE 0x08
#define CHAR_MINUS 0x40
#define CHAR_FC    0x63

#define _WIFI_RED_ 0x80

#define _WIFI_YELLOW_
#define _WIFI_GREEN_

using esphome::esp_log_printf_;

class Cms79ft738_Led {
public:
	Cms79ft738 cms79ft738;

        Cms79ft738_Led();
	Cms79ft738_ErrorCode begin(char address, char digit1, char  digit2, char digit3, char icon, char lines, char brightness);
	Cms79ft738_ErrorCode updateLed();
	void setDigit1(char digit1);
	void setDigit2(char digit2);
	void setDigit3(char digit3);
	void setChar1(char char1);
	void setChar2(char char2);
	void setChar3(char char3);
	char setIconTranslationLayer();
	void setIcon();
	void setIconWifi(bool wifi);
	void setIconSettings(bool settings);
	void setIconMinus(bool minus);
	void setIconPlus(bool plus);
	void setIconHeating(bool heating);
	void setIconPower(bool power);
	void setIconWifiGreen(bool wifi_green);
	void setIconX(bool x);
	bool getIconSettings();
	bool getIconMinus();
	bool getIconPlus();
	bool getIconHeating();
	bool getIconPower();
	bool getIconWifiGreen();
	bool getIconWifiRed();
	void setLines(char lines);
	char setBrightnessTranslationLayer(int brightness);
	void setBrightness(int brightness);
	char setDigitTranslationLayer(int digit);
	void setTemperature(float temperature);
	char setCharTranslationLayer(char character);
	void setOffDisplay();
	void setFDisplay();
	void setCDisplay();
private:
	bool heating_icon;
	bool power_icon;
	bool wifi_green_icon;
	bool wifi_yellow_icon;
	bool wifi_red_icon;
	bool wifi_icon;
	bool settings_icon;
	bool minus_icon;
	bool plus_icon;
	bool x_icon;

	char _address;
	char _digit1;
	char _digit2;
	char _digit3;
	char _icon;
	char _lines;
	char _brightness;

	Cms79ft738_ErrorCode writeData(char *data, int len);
};

#endif
