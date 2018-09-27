
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "Adafruit_CCS811.h"

//wifi
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

//time keeping
#include <NTPClient.h>

#define SEALEVELPRESSURE_HPA (1013.25)

// WiFi credentials. See external file
#include "WiFi_Credentials.h"

const int GMT_ZONE = -6;      //GMT timzeone number -7, 0, 5, 8, etc




//sensors
Adafruit_BME680 bme;    // I2C
Adafruit_CCS811 ccs;     //I2C


int statusLED = 13;     //used to indicate error


//BME 680 Calibration
const float TEMP_OFFSET = -2.1f;
const float PRESSURE_OFFSET = 0;
const float HUMIDITY_OFFSET = 0;
const float GAS_RESISTANCE_OFFSET = 0;
const float ALTITUDE_OFFSET = 0;

//ccs811
const float CO2_OFFSET = 0;
const float TVOC_OFFSET = 0; 
const float TEMP_811_OFFSET = 0;



//JSON buffers
StaticJsonBuffer<600> jsonWriteBuffer;
StaticJsonBuffer<600> jsonReadBuffer;



void setup() {
  Serial.begin(9600);
  pinMode(statusLED, OUTPUT);

  SetupWifi();

  SetupBME680();        //Temperature, Pressure, Humidity, Gas (TVOC), Approx Altitude
  SetupCCS811();        //Temperature, CO2 (PPM), TVOC (PPB)
  
}

void loop() {
  PrintValuesSerial();
  
  delay(2000);

}

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String GetCurrentTimeString(){

  //this function may take a while depending on the internet
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  return timeClient.getFormattedTime();
}



void SetupWifi(){
  // Giving it a little time because the serial monitor doesn't
  // immediately attach. Want the firmware that's running to
  // appear on each upload.
  delay(2000);

  Serial.println();
  Serial.println("Running Firmware.");

  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Connecting...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    // Check to see if connecting failed.
    // This is due to incorrect credentials
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WIFI. Please verify credentials: ");
      Serial.println();
      Serial.print("SSID: ");
      Serial.println(WIFI_SSID);
    }
    delay(5000);
  }


  //Connected! Yay

  //setup time
  timeClient.begin();       //begin time client
  timeClient.setTimeOffset(GMT_ZONE * 3600);


  //print debug information
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Local time:");
  Serial.println(GetCurrentTimeString());
  Serial.println("Hello World, I'm connected to the internets!!");
}


void SetupBME680(){
  while (!Serial);
  Serial.println("BME680 starting...");

  while (!bme.begin()) {
    Serial.println("Failed to start BME680 sensor! Please check your wiring.");
    delay(2000);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

}

void SetupCCS811(){
  Serial.println("CCS811 starting...");
  
  if(!ccs.begin()){
    Serial.println("Failed to start CCS811 sensor! Please check your wiring.");
    while(1);
  }

  //calibrate temperature sensor
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
}


unsigned int frameCount = 0;
//Print out the current sensor values and state to serial.
void PrintValuesSerial(){

  //print current frame
  Serial.print("--------------Frame #");
  Serial.print(frameCount);
  Serial.println("--------------");



  //wifi state
  Serial.println("=====WIFI State=====");
  WiFi.status();
  WiFi.localIP();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi disconnected :(");
  }

  Serial.println("Current time: " + GetCurrentTimeString());
  Serial.println();


  //BME680
  Serial.println("=====BME680=====");
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  Serial.print("Temperature = ");
  Serial.print(bme.temperature + TEMP_OFFSET);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print((bme.pressure / 100.0) + PRESSURE_OFFSET);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity + HUMIDITY_OFFSET);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print((bme.gas_resistance / 1000.0) + GAS_RESISTANCE_OFFSET);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA) + ALTITUDE_OFFSET);
  Serial.println(" m");

  Serial.println();


  //CCS811
  Serial.println("=====CCS811=====");
  if(ccs.available()){
    float temp = ccs.calculateTemperature();

    //add enviromental data to improve readings
    ccs.setEnvironmentalData(bme.humidity + HUMIDITY_OFFSET, bme.temperature + TEMP_OFFSET);
    
    if(!ccs.readData()){
      Serial.print("CO2: ");
      Serial.print(ccs.geteCO2() + CO2_OFFSET);
      Serial.print("ppm, TVOC: ");
      Serial.print(ccs.getTVOC() + TVOC_OFFSET);
      Serial.print("ppb   Temp:");
      Serial.println(temp + TEMP_811_OFFSET);
 
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }
  }

  Serial.println();
  Serial.println();


  frameCount++;
}
