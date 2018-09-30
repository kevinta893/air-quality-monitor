# air-quality-monitor

Intended for use with the Adafruit HUZZAH32 – ESP32 Feather Board using the BME680, and CCS811 enviromental sensors. It publishes data onto ThingSpeak at a rate one update per minute. 

See the ThingSpeak channel for this repository: https://thingspeak.com/channels/585230

Visualizations on ThingSpeak are not included in this respository.

You will need to setup WiFi and a ThingSpeak channel for this project to work.

## WiFi Setup

1. Make a copy of *Wifi_Credentials_sample.h* and rename it to *Wifi_Credentials.h*
2. Enter the desired WiFi information
3. Compile and you should recieve no missing file errors or missing variables

## ThingSpeak

1. Make a copy of *ThingSpeak_API_Keys_sample.h* and rename it to *ThingSpeak_API_Keys.h*
2. Enter the desired ThingSpeak API key information (the channel ID and write API key)
3. Compile and you should recieve no missing file errors or missing variables

## Wiring and Hardware

The hardware setup uses an Adafruit Huzzah32 (ESP32) WiFi microcontroller and two enviromental sensors BME680 and CCS811. The sensors are connected via the I2C protocol. A 4.7k pull-up resistor setup is used for both the SDA and SCK lines. See included wiring diagram.

Finally a 3D printable enclosure for the breadboard was made to help keep the sensors safe during transport. Made in OpenSCAD.

## Libraries used

Install the following libraries to compile and upload the project.

* Adafruit Unified Sensor Library v1.0.2
* Adafruit BME680 Library v1.0.7
* Adafruit CCS811 Library v1.0.1
* ThingSpeak v1.3.3
