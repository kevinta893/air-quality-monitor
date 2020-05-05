#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_CCS811.h"

// CCS811 Calibration
#define CCS811_CO2_OFFSET 0
#define CCS811_TVOC_OFFSET 0
#define CCS811_TEMP_OFFSET 0
#define CCS811_MAX_RETRY_SETUP_LIMIT 20

struct CCS811DataFrame{
  float co2;
  float tvoc;
  float temperature_estimate;
};

Adafruit_CCS811 ccs811;     //I2C
CCS811DataFrame ccs811LastReading;    //Data last read from the BME680 after ReadBME680()


/**
 * Setup the CCS811 sensor
 * Has retry loop to reconnect if setup fails
 * Temperature, CO2 (PPM), TVOC (PPB)
 */
bool SetupCCS811(){
  Serial.println("CCS811 starting...");

  int ccs811SetupRetry = 0;
  while(!ccs811.begin()){
    Serial.println("Failed to start CCS811 sensor! Please check your wiring. Retrying...");
    ccs811SetupRetry++;
    if (ccs811SetupRetry > CCS811_MAX_RETRY_SETUP_LIMIT){
      return false;
    }
    delay(2000);
  }
  
  Serial.println("CCS811 started.");

  //calibrate temperature sensor
  while(!ccs811.available());
  float temp = ccs811.calculateTemperature();
  ccs811.setTempOffset(temp - 25.0);

  return true;
}

/**
 * Gets the current sensor readings from the BME680
 * Sensor Data returned in out variable
 * Returns true if read is successful, false otherwise
 */
bool ReadCCS811(CCS811DataFrame* out){
  if(!ccs811.available()){
    return false;
  }

  uint8_t errorCode = ccs811.readData();
  if(errorCode){
    Serial.print("Failed to read from CCS811. Error Code=");
    Serial.println(errorCode);
    return false;
  }
  
  out->co2 = ccs811.geteCO2() + CCS811_CO2_OFFSET;
  out->tvoc = ccs811.getTVOC() + CCS811_TVOC_OFFSET;
  out->temperature_estimate = ccs811.calculateTemperature() + CCS811_TEMP_OFFSET;
  
  return true;
}

/**
 * Gets the current sensor readings from the BME680 using the
 * provided enviromental data to enhance the readings
 * Sensor Data returned in out variable
 * Returns true if read is successful, false otherwise
 */
bool ReadCCS811(float humidity, float temperature, CCS811DataFrame* out){
  ccs811.setEnvironmentalData(humidity, temperature);
  return ReadCCS811(out);
}
