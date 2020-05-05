#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_CCS811.h"

// CCS811 Calibration
#define CO2_OFFSET 0
#define TVOC_OFFSET 0
#define TEMP_811_OFFSET 0
#define MAX_RETRY_SETUP_LIMIT 20

Adafruit_CCS811 ccs;     //I2C


int ccs811SetupRetry = 0;
/**
 * Setup the CCS811 sensor
 * Has retry loop to reconnect if setup fails
 * Temperature, CO2 (PPM), TVOC (PPB)
 */
bool SetupCCS811(){
  Serial.println("CCS811 starting...");

  ccs811SetupRetry = 0;
  while(!ccs.begin()){
    Serial.println("Failed to start CCS811 sensor! Please check your wiring. Retrying...");
    ccs811SetupRetry++;
    if (ccs811SetupRetry > MAX_RETRY_SETUP_LIMIT){
      return false;
    }
    delay(2000);
  }
  
  Serial.println("CCS811 started.");

  //calibrate temperature sensor
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);

  return true;
}
