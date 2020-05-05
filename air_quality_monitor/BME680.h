#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

// BME 680 Calibration
#define SEALEVELPRESSURE_HPA 1013.25
#define TEMP_OFFSET -2.1f
#define PRESSURE_OFFSET 0
#define HUMIDITY_OFFSET 0
#define GAS_RESISTANCE_OFFSET 0
#define ALTITUDE_OFFSET 0
#define MAX_RETRY_SETUP_LIMIT 20

Adafruit_BME680 bme;     //I2C

int bme680SetupRetry = 0;

/**
 * Setup the BME680 sensor
 * Has retry loop to reconnect if setup fails
 * Temperature, Pressure, Humidity, Gas (TVOC), Approx Altitude
 */
bool SetupBME680(){
  while (!Serial);
  Serial.println("BME680 starting...");

  bme680SetupRetry = 0;
  while (!bme.begin()) {
    Serial.println("Failed to start BME680 sensor! Please check your wiring. Retrying...");
    bme680SetupRetry++;
    if (bme680SetupRetry > MAX_RETRY_SETUP_LIMIT){
      return false;
    }
    delay(2000);
  }

  Serial.println("BME680 started.");

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  return true;
}
