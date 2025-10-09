#pragma once

#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/globals/globals_component.h"
#include "esphome/components/button/button.h" 

#include "nvs_flash.h"
#include <bitset>
#include "cms79ft738.h"
#include "cms79ft738_led.h"
#include "cms79ft738_key.h"

#define MIN_TEMPERATURE 5
#define MAX_TEMPERATURE 35

#define I2C_ADDRESS 0x50
//#define POLLING_PERIOD 1000
#define POLLING_PERIOD 50

#define EXTENDED_WIFI_LOGIC

namespace esphome {
namespace mill {

class MillClimate : public climate::Climate, public PollingComponent, public i2c::I2CDevice {
 public:
	bool wifi_enabled;
	bool new_data;
	bool update_lcd;
	bool fahrenheit_enabled;

	uint32_t bright_deadline = 0;
	bool     bright_boost    = false;

	float pending_current_ = NAN;
	float pending_target_ = NAN;

#ifdef EXTENDED_WIFI_LOGIC
	uint8_t wifi_color;
#endif

        Cms79ft738 cms79ft738;
        Cms79ft738_Led cms79ft738_led;
        Cms79ft738_Key cms79ft738_key;

        uint32_t last_button_press_time = 0;
        int last_button_state = -1;

        uint32_t last_settings_press_time = 0;
        bool settings_button_was_pressed = false;

        MillClimate() : PollingComponent(POLLING_PERIOD) {}

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

	template <typename V> void BinarySensor_publish(binary_sensor::BinarySensor *sensor, V value) {
		internal_publish_state(sensor, value);
	}

	template <typename V> void TextSensor_publish(text_sensor::TextSensor *sensor, V value) {
		internal_publish_state(sensor, value);
	}

	template <typename V> void Sensor_publish(sensor::Sensor *sensor, V value) {
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

	static inline float c_to_f(float c) { return c * 1.8f + 32.0f; }
	static inline float f_to_c(float f) { return (f - 32.0f) / 1.8f; }

  void set_temperature_sensor(sensor::Sensor *s) { this->temperature_sensor_ = s; }
  void set_heat_switch(switch_::Switch *s) { this->heat_switch_ = s; }
  void set_status_text(text_sensor::TextSensor *t) { this->status_text_ = t; }
  void set_target_temperature_sensor(sensor::Sensor *s) { this->target_temp_pub_ = s; }
  void set_current_temperature_sensor(sensor::Sensor *s) { this->current_temp_pub_ = s; }

  void set_target_temp_global(globals::GlobalsComponent<float>* g)             { target_temp_ptr_  = g ? &g->value() : nullptr; }
  void set_target_temp_global(globals::RestoringGlobalsComponent<float>* g)    { target_temp_ptr_  = g ? &g->value() : nullptr; }
  void set_current_temp_global(globals::GlobalsComponent<float>* g)            { current_temp_ptr_ = g ? &g->value() : nullptr; }
  void set_current_temp_global(globals::RestoringGlobalsComponent<float>* g)   { current_temp_ptr_ = g ? &g->value() : nullptr; }

  void set_status_value_global(globals::GlobalsComponent<bool>* g)            { status_value_ptr_ = g ? &g->value() : nullptr; }
  void set_status_value_global(globals::RestoringGlobalsComponent<bool>* g)   { status_value_ptr_ = g ? &g->value() : nullptr; }

  void set_reset_button(button::Button *b)   { this->reset_btn_ = b; }
  void set_factory_button(button::Button *b) { this->factory_btn_ = b; }

	void blit();
	void init_nvs();
	void save_wifi_state(bool enabled);
	void save_degrees_state(bool enabled);
	bool load_degrees_state();
	bool load_wifi_state();

	void setup() override;
	void loop() override;
	void update() override;

	climate::ClimateTraits traits() override;
	void control(const climate::ClimateCall &call) override;

 protected:
  sensor::Sensor *temperature_sensor_{nullptr};
  switch_::Switch *heat_switch_{nullptr};
  text_sensor::TextSensor *status_text_{nullptr};
  sensor::Sensor *target_temp_pub_{nullptr};
  sensor::Sensor *current_temp_pub_{nullptr};

  float* target_temp_ptr_{nullptr};
  float* current_temp_ptr_{nullptr};

  button::Button *reset_btn_{nullptr};
  button::Button *factory_btn_{nullptr};

  bool* status_value_ptr_{nullptr};
};

}  // namespace mill
}  // namespace esphome
