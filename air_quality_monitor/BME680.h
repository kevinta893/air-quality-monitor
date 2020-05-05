#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

// BME 680 Calibration
#define BME680_SEALEVELPRESSURE_HPA 1013.25
#define BME680_TEMP_OFFSET -2.1f
#define BME680_PRESSURE_OFFSET 0
#define BME680_HUMIDITY_OFFSET 0
#define BME680_GAS_RESISTANCE_OFFSET 0
#define BME680_ALTITUDE_OFFSET 0
#define BME680_MAX_RETRY_SETUP_LIMIT 20

struct BME680DataFrame{
  float temperature;
  float pressure;
  float humidity;
  float gas_resistance;
  float altitude;
};

Adafruit_BME680 bme680;     //I2C

/**
 * Setup the BME680 sensor
 * Has retry loop to reconnect if setup fails
 * Temperature, Pressure, Humidity, Gas (TVOC), Approx Altitude
 */
bool SetupBME680(){
  while (!Serial);
  Serial.println("BME680 starting...");

  int bme680SetupRetry = 0;
  while (!bme680.begin()) {
    Serial.println("Failed to start BME680 sensor! Please check your wiring. Retrying...");
    bme680SetupRetry++;
    if (bme680SetupRetry > BME680_MAX_RETRY_SETUP_LIMIT){
      return false;
    }
    delay(2000);
  }

  Serial.println("BME680 started.");

  // Set up oversampling and filter initialization
  bme680.setTemperatureOversampling(BME680_OS_8X);
  bme680.setHumidityOversampling(BME680_OS_2X);
  bme680.setPressureOversampling(BME680_OS_4X);
  bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme680.setGasHeater(320, 150); // 320*C for 150 ms

  return true;
}

/**
 * Gets the current sensor readings from the BME680
 * Sensor Data returned in out variable
 * Returns true if read is successful, false otherwise
 */
bool ReadBME680(BME680DataFrame* out){
  if (!bme680.performReading()) {
    return false;
  }
  
  out->temperature = bme680.temperature + BME680_TEMP_OFFSET;
  out->pressure = (bme680.pressure / 100.0) + BME680_PRESSURE_OFFSET;
  out->humidity = bme680.humidity + BME680_HUMIDITY_OFFSET;
  out->gas_resistance = (bme680.gas_resistance / 1000.0f) + BME680_GAS_RESISTANCE_OFFSET;
  out->altitude = bme680.readAltitude(BME680_SEALEVELPRESSURE_HPA) + BME680_ALTITUDE_OFFSET;
  
  return true;
}
