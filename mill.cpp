#include "mill.h"

namespace esphome {
namespace mill {

static const char *TAG = "mill.climate";

void MillClimate::blit() {
	new_data = true;
}

void MillClimate::init_nvs() {
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		nvs_flash_erase();
		nvs_flash_init();
	}
}

void MillClimate::save_wifi_state(bool enabled) {
	nvs_handle_t nvs_handle;
	esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
	if (err == ESP_OK) {
		nvs_set_u8(nvs_handle, "wifi_state", enabled ? 1 : 0);
		nvs_commit(nvs_handle);
		nvs_close(nvs_handle);
	}
}

bool MillClimate::load_degrees_state() {
	nvs_handle_t nvs_handle;
	uint8_t degrees_state = 0;
	esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
	if (err == ESP_OK) {
		nvs_get_u8(nvs_handle, "degrees_state", &degrees_state);
		nvs_close(nvs_handle);
	}
	return degrees_state == 1;
}

void MillClimate::save_degrees_state(bool enabled) {
	nvs_handle_t nvs_handle;
	esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
	if (err == ESP_OK) {
		nvs_set_u8(nvs_handle, "degrees_state", enabled ? 1 : 0);
		nvs_commit(nvs_handle);
		nvs_close(nvs_handle);
	}
}

bool MillClimate::load_wifi_state() {
	nvs_handle_t nvs_handle;
	uint8_t wifi_state = 0;
	esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
	if (err == ESP_OK) {
		nvs_get_u8(nvs_handle, "wifi_state", &wifi_state);
		nvs_close(nvs_handle);
	}
	return wifi_state == 1;
}

void MillClimate::setup() {
	ESP_LOGD("mill", "MillClimate initialized");
	cms79ft738.begin();
	cms79ft738_led.begin(0x50, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
	cms79ft738_key.begin(0x50, 0x00, 0x00, 0x00, 0x00);

	cms79ft738_led.setIconWifi(true);
	cms79ft738_led.setIconSettings(true);
	cms79ft738_led.setIconMinus(true);
	cms79ft738_led.setIconPlus(true);
	cms79ft738_led.setIconHeating(false);
	cms79ft738_led.setIconPower(true);
	cms79ft738_led.setIconWifiGreen(false);
	cms79ft738_led.setIconX(false);

	cms79ft738_led.updateLed();

	nvs_flash_init();
	wifi_enabled = load_wifi_state();
	if (wifi_enabled) {
		wifi::global_wifi_component->enable();
	} else {
		wifi::global_wifi_component->disable();
	}

#ifdef EXTENDED_WIFI_LOGIC
	wifi_color = 0xFF;
#endif

	fahrenheit_enabled = load_degrees_state();
}

void MillClimate::loop() {
	if (new_data == true) {
		if (this->current_temp_ptr_)
			this->current_temperature = *this->current_temp_ptr_;
		if (this->target_temp_ptr_)
			this->target_temperature = *this->target_temp_ptr_;

		if (this->target_temperature < MIN_TEMPERATURE)
			this->target_temperature = MIN_TEMPERATURE;
		if (this->target_temperature > MAX_TEMPERATURE)
			this->target_temperature = MAX_TEMPERATURE;

		ESP_LOGV(TAG, "new data recived c %f t %f", this->current_temperature, this->target_temperature);

		if (this->target_temp_pub_)
			Sensor_publish(this->target_temp_pub_, this->target_temperature);

		if (this->current_temp_pub_)
			Sensor_publish(this->current_temp_pub_, this->current_temperature);

		cms79ft738_led.setTemperature(this->target_temperature, fahrenheit_enabled);

		this->publish_state();
		new_data = false;
	}
}

void MillClimate::update() {
	int button = cms79ft738_key.readKey();
	uint32_t current_time = millis();

	if (button != last_button_state && current_time - last_button_press_time > 50) {
		last_button_press_time = current_time;
		last_button_state = button;

		cms79ft738_led.setBrightness(15);
		bright_deadline = current_time + 30000UL;
		bright_boost    = true;

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

			cms79ft738_led.setTemperature(wanted_temp, fahrenheit_enabled);
			this->target_temperature = wanted_temp;
			if (this->target_temp_ptr_)
				*this->target_temp_ptr_ = wanted_temp;

			if (this->target_temp_pub_)
				Sensor_publish(this->target_temp_pub_, wanted_temp);
			this->publish_state();
		}

		wifi_enabled = load_wifi_state();

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

				if (this->factory_btn_)
					this->factory_btn_->press();
				delay(500);
				cms79ft738_led.setIconSettings(true);
				settings_button_was_pressed = false;

				if (this->reset_btn_)
					this->reset_btn_->press();
			}
		}  else if (settings_button_was_pressed) {
			if (current_time - last_button_press_time < 3000) {
#if 0
				bool save_settings_degrees = false;
				fahrenheit_enabled = load_degrees_state();
				if (fahrenheit_enabled)
					cms79ft738_led.setFDisplay();
				else
					cms79ft738_led.setCDisplay();
				if (button == KEY_PLUS && fahrenheit_enabled) {
					cms79ft738_led.setCDisplay();
					fahrenheit_enabled = false;
				} else if (button == KEY_PLUS && !fahrenheit_enabled) {
					cms79ft738_led.setFDisplay();
					fahrenheit_enabled = true;
				} else if (button == KEY_MINUS && fahrenheit_enabled) {
					cms79ft738_led.setCDisplay();
					fahrenheit_enabled = false;
				} else if (button == KEY_MINUS && !fahrenheit_enabled) {
					cms79ft738_led.setFDisplay();
					fahrenheit_enabled = true;
				} else if (button == KEY_SETTINGS) {
					cms79ft738_led.setIconSettings(false);
					delay(500);
					cms79ft738_led.setIconSettings(true);
					save_degrees_state(fahrenheit_enabled);
					settings_button_was_pressed = false;
					if (this->reset_btn_)
						this->reset_btn_->press();
				}
#endif
			}

			settings_button_was_pressed = false;
		}

		ESP_LOGD(TAG, "read key %s", cms79ft738_key.key2string(button));
		update_lcd = true;
	}

	if (this->mode != climate::CLIMATE_MODE_OFF) {
		//if (this->current_temp_ptr_ && this->target_temp_ptr_) {
			if (*this->current_temp_ptr_ < *this->target_temp_ptr_) {
				this->action = climate::CLIMATE_ACTION_HEATING;
				if (this->status_text_)
					TextSensor_publish(this->status_text_, "HEAT");
			} else {
				this->action = climate::CLIMATE_ACTION_IDLE;
				if (this->status_text_)
					TextSensor_publish(this->status_text_, "IDLE");
			}
		//}
	}

	if (this->status_value_ptr_ && *this->status_value_ptr_ == true) {
		this->mode = climate::CLIMATE_MODE_HEAT;
	} else {
		this->mode = climate::CLIMATE_MODE_OFF;
	}

#ifdef EXTENDED_WIFI_LOGIC
	auto mode = WiFi.getMode();
	auto st   = WiFi.status();

	uint8_t color = 0;

	if (mode == WIFI_MODE_NULL) {
		color = 0;
	} else if (mode == WIFI_MODE_AP) {
		int n = WiFi.softAPgetStationNum();
		color = (n > 0) ? 1 : 2;
	} else if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) {
		switch (st) {
			case WL_CONNECTED:
				color = 1;
				break;
			case WL_IDLE_STATUS:
				color = 2;
				break;
			case WL_CONNECTION_LOST:
				color = 2;
				break;
			case WL_NO_SSID_AVAIL:
				color = 3;
				break;
			case WL_CONNECT_FAILED:
				color = 3;
				break;
			case WL_DISCONNECTED:
			default:
				color = 3;
				break;
		}
		if (mode == WIFI_MODE_APSTA && color != 1) {
			int n = WiFi.softAPgetStationNum();
			if (n > 0 && color == 3)
				color = 2;
		}
	}

	if (color != wifi_color) {
		if (wifi_color == 1)
			cms79ft738_led.setIconWifiGreen(false);
		else if (wifi_color == 2)
			cms79ft738_led.setIconWifiYellow(false);
		else if (wifi_color == 3)
			cms79ft738_led.setIconWifiRed(false);

		if (color == 1)
			cms79ft738_led.setIconWifiGreen(true);
		else if (color == 2)
			cms79ft738_led.setIconWifiYellow(true);
		else if (color == 3)
			cms79ft738_led.setIconWifiRed(true);

		wifi_color = color;
		update_lcd = true;
	}
#else
	if (WiFi.isConnected()) {
		cms79ft738_led.setIconWifiGreen(true);
		update_lcd = true;
	} else {
		cms79ft738_led.setIconWifiGreen(false);
		update_lcd = true;
	}
#endif

	if (this->action == climate::CLIMATE_ACTION_HEATING) {
		cms79ft738_led.setIconHeating(true);
		if (this->heat_switch_ && !this->heat_switch_->state) {
			this->heat_switch_->turn_on();
		}
		update_lcd = true;
	} else {
		cms79ft738_led.setIconHeating(false);
		if (this->heat_switch_ && this->heat_switch_->state) {
			this->heat_switch_->turn_off();
		}
		update_lcd = true;
	}

	if (bright_boost && (int32_t)(current_time - bright_deadline) >= 0) {
		cms79ft738_led.setBrightness(2);
		bright_boost = false;
		update_lcd = true;
	}

	if (update_lcd) {
		cms79ft738_led.updateLed();
		update_lcd = false;
	}
}

climate::ClimateTraits MillClimate::traits() {
	auto traits = climate::ClimateTraits();
	traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT});
	traits.set_supports_current_temperature(true);
	traits.set_supports_action(true);
	traits.set_visual_min_temperature(5);
	traits.set_visual_max_temperature(35);
	traits.set_visual_temperature_step(1.0f);
	return traits;
}

void MillClimate::control(const climate::ClimateCall &call) {
	if (call.get_mode().has_value()) {
		switch (call.get_mode().value()) {
			case climate::CLIMATE_MODE_OFF:
				ESP_LOGV(TAG, "Turning off the mill heater");
				if (this->status_text_)
					TextSensor_publish(this->status_text_, "OFF");
				cms79ft738_led.setBrightness(1);
				cms79ft738_led.setOffDisplay();
				break;
			case climate::CLIMATE_MODE_HEAT:
				ESP_LOGV(TAG, "Turning on the mill heater");
				if (this->status_text_)
					TextSensor_publish(this->status_text_, "HEAT");
				cms79ft738_led.setBrightness(15);
				cms79ft738_led.setTemperature(this->target_temperature, fahrenheit_enabled);
				if (this->target_temp_pub_)
					Sensor_publish(this->target_temp_pub_, this->target_temperature);

				bright_deadline = millis() + 30000UL;
				bright_boost    = true;

				break;
			default:
				ESP_LOGD(TAG, "Unsupported heater mode: %d", (uint8_t)(*call.get_mode()));
				/* other modes are not supported */
				break;
		}

		climate::ClimateMode mode = *call.get_mode();

		this->mode = mode;
		this->publish_state();
	}

	if (call.get_mode().value() != climate::CLIMATE_MODE_OFF) {
		if (call.get_target_temperature().has_value()) {
			// User requested target temperature change
			float target_temp = *call.get_target_temperature();
			cms79ft738_led.setTemperature(target_temp, fahrenheit_enabled);
			this->target_temperature = target_temp;
			if (this->target_temp_ptr_)
				*this->target_temp_ptr_ = target_temp;
			if (this->target_temp_pub_)
				Sensor_publish(this->target_temp_pub_, target_temp);
			this->publish_state();
		}
	}

	update_lcd = true;
}

}  // namespace cms79ft738
}  // namespace esphome
