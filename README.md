This project is an alternative firmware for a Mill 400W heater in Norway, based on ESPHome. The heater works with Home Assistant. There's still work to be done, particularly with the behavior of the settings button, and I'm unsure if the temperature measurements are accurate.

Here’s what’s known so far:

    GPIO 4 is used for heating (switch),
    GPIO 19 and 18 are used for the I2C bus,
    GPIO 34 is connected to the NTC temperature sensor,
    There’s something connected to GPIO 35, but it will be determined later,
    GPIO 16 and 17 are used for TTL communication.

Additionally, a custom driver for the CMS79FT738 display has been fully implemented, with complete support for all buttons and icons. Theoretically, this firmware could also be compatible with other heaters that use ESP32, as the communication protocol should be the same.

There are probably other things I’ve overlooked. I wrote this because I’m frustrated with the official Norwegian firmware for Mill heaters. Fuck closed-source software!
