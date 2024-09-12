/*
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
#include "nvs_flash.h"
#include <bitset>
#include "esphome/core/log.h"
#include "cms79ft738.h"
#include "cms79ft738_led.h"
#include "cms79ft738_key.h"

#define MIN_TEMPERATURE 5
#define MAX_TEMPERATURE 35

#define I2C_ADDRESS 0x50
//#define POLLING_PERIOD 1000
#define POLLING_PERIOD 50

const char *const _log_tag = "millheat"; // prefix for log messages
                      /*   0     1     2     3     4     5     6     7     8     9    10    11    12 */
char global_array[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

bool _newData;
bool updateLcd;
bool updateSensor;
char _receivedChars[15];

class Cms79ft738LedMill : public PollingComponent, public Climate {
public:
	Cms79ft738 cms79ft738;
	Cms79ft738_Led cms79ft738_led;
	Cms79ft738_Key cms79ft738_key;

	Cms79ft738LedMill() : PollingComponent(POLLING_PERIOD) {}

	uint32_t last_button_press_time = 0;
	int last_button_state = -1;

	uint32_t last_settings_press_time = 0;
	bool settings_button_was_pressed = false;

	float get_setup_priority() const override { return esphome::setup_priority::BUS; } //Access I2C bus

	bool get_bit(uint8_t data, int bit) {
		return data & (1 << bit);
	}

	float getFloat24(byte msbh, byte msbl, byte lsb) {
		// Calculate float value from four bytes
		uint32_t _msbh = msbh;
		uint32_t _msbl = msbl;
		int32_t dword = (int32_t)(_msbh << 16 | _msbl << 8 | lsb);
		return ((float)dword) / 10000.0;
	}

	float getFloat32(byte msbh, byte msbl, byte lsbh, byte lsbl) {
		// Calculate float value from four bytes
		uint32_t _msbh = msbh;
		uint32_t _msbl = msbl;
		uint32_t _lsbh = lsbh;
		int32_t dword = (int32_t)(_msbh << 24 | _msbl << 16 | _lsbh << 8 | lsbl);
		return ((float)dword) / 10000.0;
	}

	float getFloat(byte msb, byte lsb) {
		// Calculate float value from two bytes
		uint16_t _msb = msb;
		int16_t word = (int16_t)(_msb << 8 | lsb);
		return ((float)word) / 100.0;
	}

	int16_t getInt(byte msb, byte lsb) {
		// Calculate float value from two bytes
		uint16_t _msb = msb;
		int16_t word = (int16_t)(_msb << 8 | lsb);
		return word;
	}

	int32_t getInt24(byte msbh, byte msbl, byte lsb) {
		// Calculate float value from two bytes
		uint32_t _msbh = msbh;
		uint32_t _msbl = msbl;
		int32_t word = (int32_t)(_msbh << 16 | _msbl << 8 | lsb);
		return word;
	}

	bool is_equal(const float &value1, const float &value2) {
		// For floating point also check if both values are NAN
		return ((value1 == value2) || (std::isnan(value1) && std::isnan(value2)));
	}
	bool is_equal(const char* &value1, const char* &value2) {
		return !strcmp(value1, value2);
	}
	bool is_equal(const std::string &value1, const std::string &value2) {
		return !value1.compare(value2);
	}

	template <typename V> void BinarySensor_publish(BinarySensor *sensor, V value) {
		internal_publish_state(sensor, value);
	}
	template <typename V> void TextSensor_publish(TextSensor *sensor, V value) {
		internal_publish_state(sensor, value);
        }
	template <typename V> void Sensor_publish(Sensor *sensor, V value) {
		internal_publish_state(sensor, value);
	}

	// Send the value to Home-assistant, but only do that if the value
	// really has changed. A cache is used to determine changes. This lowers
	// the network bandwidth required, but also enables a smaller database
	// on the home assistant side while keeping a fast response to changing
	// values.
	template <typename S, typename V> void internal_publish_state(S *sensor, V value) {
		static std::map<S*, V> cache = {};
		bool do_send_value = true;

		auto iter = cache.find(sensor);
		if (iter != cache.end()) {
			// Cached value exists for this sensor, check it the value has changed.
			if (is_equal(iter->second, value)) {
				do_send_value = false;
			}
		}
		if (do_send_value) {
			sensor->publish_state(value);
			cache[sensor] = value;
		}
	}


	void init_nvs() {
		esp_err_t ret = nvs_flash_init();
		if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
			nvs_flash_erase();
			nvs_flash_init();
		}
	}

	void save_wifi_state(bool enabled) {
		nvs_handle_t nvs_handle;
		esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
		if (err == ESP_OK) {
			nvs_set_u8(nvs_handle, "wifi_state", enabled ? 1 : 0);
			nvs_commit(nvs_handle);
			nvs_close(nvs_handle);
		}
	}

	bool load_wifi_state() {
		nvs_handle_t nvs_handle;
		uint8_t wifi_state = 0;
		esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
		if (err == ESP_OK) {
			nvs_get_u8(nvs_handle, "wifi_state", &wifi_state);
			nvs_close(nvs_handle);
		}
		return wifi_state == 1;
	}

	void setup() override {
		cms79ft738.begin();
		cms79ft738_led.begin(0x50, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
		cms79ft738_key.begin(0x50, 0x00, 0x00, 0x00, 0x00);

		cms79ft738_led.setIconWifi(true);
		cms79ft738_led.setIconSettings(true);
		cms79ft738_led.setIconMinus(true);
		cms79ft738_led.setIconPlus(true);
		cms79ft738_led.setIconHeating(false);
		cms79ft738_led.setIconPower(false);
		cms79ft738_led.setIconWifiGreen(false);
		cms79ft738_led.setIconX(false);

		cms79ft738_led.updateLed();

		init_nvs();
		wifi_enabled = load_wifi_state();
		if (wifi_enabled) {
			wifi::global_wifi_component->enable();
		} else {
			wifi::global_wifi_component->disable();
		}
	};

	void blit() {
		_newData = true;
	}

	void loop() override {
		if (_newData == true) {
			this->current_temperature = id(current_value);
			this->target_temperature = id(target_value);
			if (this->target_temperature < MIN_TEMPERATURE)
				this->target_temperature = MIN_TEMPERATURE;
			if (this->target_temperature > MAX_TEMPERATURE)
				this->target_temperature = MAX_TEMPERATURE;

			ESP_LOGI(_log_tag, "new data recived c %f t %f", this->current_temperature, this->target_temperature);

			Sensor_publish(wanted_temperature, this->target_temperature);
			Sensor_publish(now_temperature, this->current_temperature);
			cms79ft738_led.setTemperature(this->target_temperature);

#if 0
			if (id(status_value)) {
				if (id(current_value) < id(target_value)) {
					this->action = climate::CLIMATE_ACTION_HEATING;
					TextSensor_publish(heater_status, "HEAT");
					id(heating_value) = true;
				} else {
					this->action = climate::CLIMATE_ACTION_IDLE;
					TextSensor_publish(heater_status, "IDLE");
					id(heating_value) = false;
				}
			}

			if (id(status_value)) {
				this->mode = climate::CLIMATE_MODE_HEAT;
				//id(heating_value) = true;
			} else {
				this->mode = climate::CLIMATE_MODE_OFF;
				id(heating_value) = false;
			}
#endif
			this->publish_state();
			_newData = false;
		}
	}

	void update() override {
		int button = cms79ft738_key.readKey();

		uint32_t current_time = millis();

		if (button != last_button_state && current_time - last_button_press_time > 50) {
			last_button_press_time = current_time;
			last_button_state = button;

			if (button == KEY_PLUS || button == KEY_MINUS) {
				float wanted_temp = this->target_temperature;

				if (button == KEY_PLUS) {
					wanted_temp += 1.0;
				} else if (button == KEY_MINUS) {
					wanted_temp -= 1.0;
				}

				if (wanted_temp < MIN_TEMPERATURE)
					wanted_temp = MIN_TEMPERATURE;
				if (wanted_temp > MAX_TEMPERATURE)
					wanted_temp = MAX_TEMPERATURE;

				cms79ft738_led.setTemperature(wanted_temp);
				this->target_temperature = wanted_temp;
				id(target_value) = wanted_temp;
				Sensor_publish(wanted_temperature, wanted_temp);
				this->publish_state();
			}

			wifi_enabled = load_wifi_state();

			//ESP_LOGD(_log_tag, "wifi state %s", wifi_enabled ? "true" : "false");

			if (button == KEY_WIFI && wifi_enabled) {
				wifi::global_wifi_component->disable();
				save_wifi_state(false);
			} else if (button == KEY_WIFI && !wifi_enabled) {
				wifi::global_wifi_component->enable();
				save_wifi_state(true);
			}

			if (button == KEY_SETTINGS) {
				if (!settings_button_was_pressed) {
					last_settings_press_time = current_time;
					settings_button_was_pressed = true;
				}
				if (current_time - last_settings_press_time > 3000) {
					cms79ft738_led.setIconSettings(false);
					id(global_factory_settings).press();
					delay(500);
					cms79ft738_led.setIconSettings(true);
					settings_button_was_pressed = false;
					id(global_reset_device).press();
				}
			}  else if (settings_button_was_pressed) {
				if (current_time - last_button_press_time < 3000) {
					//toggleTemperatureUnit();
				}
				settings_button_was_pressed = false;
			}

			ESP_LOGD(_log_tag, "read key %s", cms79ft738_key.key2string(button));
			updateLcd = true;
		}

		if (this->mode != climate::CLIMATE_MODE_OFF) {
			if (id(current_value) < id(target_value)) {
				this->action = climate::CLIMATE_ACTION_HEATING;
				TextSensor_publish(heater_status, "HEAT");
			} else {
				this->action = climate::CLIMATE_ACTION_IDLE;
				TextSensor_publish(heater_status, "IDLE");
			}
		}

		if (id(status_value)) {
			this->mode = climate::CLIMATE_MODE_HEAT;
		} else {
			this->mode = climate::CLIMATE_MODE_OFF;
		}

		if (WiFi.isConnected()) {
			cms79ft738_led.setIconWifiGreen(true);
			updateLcd = true;
		} else {
			//wifi_ap_record_t apInfo;
    			//if (esp_wifi_sta_get_ap_info(&apInfo) == ESP_OK) {
			//} else {
			//}
			cms79ft738_led.setIconWifiGreen(false);
			updateLcd = true;
		}

		if (this->action == climate::CLIMATE_ACTION_HEATING) {
			cms79ft738_led.setIconHeating(true);
			if (!id(global_mill_heat).state) {
				id(global_mill_heat).turn_on();
			}
			updateLcd = true;
		} else {
			cms79ft738_led.setIconHeating(false);
			if (id(global_mill_heat).state) {
				id(global_mill_heat).turn_off();
			}
			updateLcd = true;
		}

		if (updateLcd) {
			cms79ft738_led.updateLed();
			updateLcd = false;
		}
	}

	void control(const ClimateCall &call) override {
		if (call.get_mode().has_value()) {

			switch (call.get_mode().value()) {
				case CLIMATE_MODE_OFF:
					ESP_LOGV(_log_tag, "Turning off the mill heater");
					TextSensor_publish(heater_status, "OFF");
					cms79ft738_led.setBrightness(1);
					cms79ft738_led.setOffDisplay();
					break;
				case CLIMATE_MODE_HEAT:
					ESP_LOGV(_log_tag, "Turning on the mill heater");
					TextSensor_publish(heater_status, "HEAT");
					cms79ft738_led.setBrightness(15);
					Sensor_publish(wanted_temperature, this->target_temperature);
					break;
				default:
					ESP_LOGD(_log_tag, "Unsupported heater mode: %d", (uint8_t)(*call.get_mode()));
					/* other modes are not supported */
					break;
			}

			ClimateMode mode = *call.get_mode();

			this->mode = mode;
			this->publish_state();
		}

		if (call.get_mode().value() != CLIMATE_MODE_OFF) {
			if (call.get_target_temperature().has_value()) {
				// User requested target temperature change
				float target_temp = *call.get_target_temperature();
				cms79ft738_led.setTemperature(target_temp);
				this->target_temperature = target_temp;
				id(target_value) = target_temp;
				Sensor_publish(wanted_temperature, target_temp);
				this->publish_state();
			}
		}

		updateLcd = true;
	}

	ClimateTraits traits() override {
		// The capabilities of the climate device
		auto traits = climate::ClimateTraits();
		traits.set_supports_current_temperature(true);
		traits.set_supports_action(true);
		traits.set_supported_modes({climate::CLIMATE_MODE_OFF,climate::CLIMATE_MODE_HEAT});
		traits.set_supports_current_temperature(true);
		traits.set_visual_min_temperature(MIN_TEMPERATURE);
		traits.set_visual_max_temperature(MAX_TEMPERATURE);
		return traits;
	}

		// 0 to check 
		// 1 position of array, line
		// 2 position of array, line
		// 3 position of array, line
		// 4 position of array, line
		// 5 position of array, line
		// 6 position of array, line
		// 7 position of array, digitt 1
		// digit value = 128, wifi icon
		// digit value > 128, digit and wifi icon
		// digit value < 128,  digit
		// digit 0x06 = 1
		// digit 0x07 = 7
		// digit 0x08 = _
		// digit 0x39 = C
		// digit 0x3e = U
		// digit 0x3f = 0
		// digit 0x40 = -
		// digit 0x4d = 3
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
		// ---- 0xa7 = 7
		// 8 position of array, digit 2
		// 9 position of array, digit 3 small
		// 10 position of array, icons, leds
		// ----------------------------------------------------------------------
		// 0xff value 255, green wifi, power, lightning, +, -, wifi, settings, 5 lines
		// 0xfe value 254, green wifi, power, lightning, +, -, wifi, settings, 4 lines
		// 0xfd value 253, green wifi, lightning, +, -, wifi, settings, 5 lines
		// 0xfc value 252, green wifi, lightning, +, -, wifi, settings, 4 lines
		// 0xfb value 251, green wifi, power, +, -, wifi, settings, 4 lines
		// 0xfa value 250, green wifi, power, +, -, wifi, settings, 3 lines
		// 0xf9 value 249, green wifi, +, -, wifi, settings, 5 lines
		// 0xf8 value 248, green wifi, +, -, wifi, settings, 4 lines
		//
		// 0x7f value 127 power, lightning, +, -, wifi, settings, 5 lines
		// 0x7e value 126 power, lightning, +, -, wifi, settings, 4 lines
		// 0x7d value 125 lightning, +, -, wifi, settings, 5 lines
		// 0x7c value 124 lightning, +, -, wifi, settings, 4 lines
		// 0x7b value 123 power, +, -, wifi, settings, 4 lines
		// 0x7a value 122 power, +, -, wifi, settings, 3 lines
		// 0x79 value 121 +, -, wifi, settings, 5 lines
		// 0x78 value 120 +, -, wifi, settings, 4 lines

		//
		// value 247, green wifi, power, lightning, +, wifi, settings, 4 lines
		// value 246, green wifi, power, lightning, +, wifi, settings, 3 lines
		// value 245, green wifi, lightning, +, wifi, settings, 4 lines
		// value 244, green wifi, lightning, +, wifi, settings, 3 lines
		// value 242, green wifi, power, +, wifi, settings, 3 lines
		// value 241, green wifi, +, wifi, settings, 5 lines
		// value 240, green wifi, +, wifi, settings, 4 lines
		//
		// value 239, green wifi, power, lightning, -, wifi, settings, 4 lines
		// value 238, green wifi, power, lightning, -, wifi, settings, 3 lines
		// value 237, green wifi, lightning, -, wifi, settings, 4 lines
		// value 236, green wifi, lightning, -, wifi, settings, 3 lines
		// value 235, green wifi, power, -, wifi, settings, 3 lines
		// value 234, green wifi, power, -, wifi, settings, 3 lines
		// value 233, green wifi, -, wifi, settings, 4 lines
		// value 232, green wifi, -, wifi, settings, 3 lines
		//
		// value 231, green wifi, power, lightning, wifi, settings, 3 lines
		// value 230, green wifi, power, lightning, wifi, settings, 2 lines
		// value 229, green wifi, lightning, wifi, settings, 3 lines
		// value 228, green wifi, lightning, wifi, settings, 2 lines
		// value 227, green wifi, power, wifi, settings, 2 lines
		// value 226, green wifi, power, wifi, settings, 2 lines
		// value 225, green wifi, wifi, settings, 5 lines
		// value 224, green wifi, wifi, settings, 4 lines
		//
		// value 223, green wifi, power, lightning, +, -, settings, 4 lines
		// value 222, green wifi, power, lightning, +, -, settings, 3 lines
		// value 221, green wifi, lightning, +, -, settings, 4 lines
		// value 220, green wifi, lightning, +, -, settings, 4 lines
		// value 219, green wifi, power, +, -, settings, 3 lines
		// value 218, green wifi, power, +, -, settings, 2 lines
		// value 217, green wifi, +, -, settings, 4 lines
		// value 216, green wifi, +, -, settings, 3 lines
		//
		// ...
		//
		
		// 11 position of array, line and dot
		// 12 position of array, brightness
		// 13 position of array, ??
		// brightness value <= 76 display off
		// brightness value 77 .. 92 display off
		// brightness value 93 .. 108 display on brightness 1
		// brightness value 109 .. 124 display on brightness 2
		// brightness value 125 .. 140 display on brightness 3
		// brightness value 141 .. 156 display on brightness 4
		// brightness value 157 .. 172 display on brightness 5
		// brightness value 173 .. 188 display on brightness 6
		// brightness value 189 .. 204 display on brightness 7
		// brightness value 205 .. 220 display on brightness 8
		// brightness value 221 .. 255 display on brightness 9
		// 14 position of array, brightness

};
