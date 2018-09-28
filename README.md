# air-quality-monitor

Intended for use with the Adafruit HUZZAH32 – ESP32 Feather Board.

## WiFi Setup

1. Make a copy of *Wifi_Credentials_sample.h* and rename it to *Wifi_Credentials.h*
2. Enter the desired WiFi information
3. Compile and you should recieve no missing file errors or missing variables

## ThingSpeak

1. Make a copy of *ThingSpeak_API_Keys_sample.h* and rename it to *ThingSpeak_API_Keys.h*
2. Enter the desired ThingSpeak API key information
3. Compile and you should recieve no missing file errors or missing variables

## Libraries used:

* Adafruit Unified Sensor Library v1.0.2
* Adafruit BME680 Library v1.0.7
* Adafruit CCS811 Library v1.0.1
* ThingSpeak v1.3.3