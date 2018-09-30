
// Sensors
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "Adafruit_CCS811.h"
#define SEALEVELPRESSURE_HPA (1013.25)

// WiFi
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>


// WiFi credentials. See external file
#include "WiFi_Credentials.h"
const char* WIFI_HOSTNAME = "Arduino Air Monitor";

// ThingSpeak API
#include <ThingSpeak.h>
#include "ThingSpeak_API_Keys.h"
const int MIN_UPDATE_INTERVAL = 15 * 1000;
const int UPDATE_INTERVAL_SECONDS = 60 * 1000;
const int WARMUP_PERIOD = (30 * 60 * 1000) + MIN_UPDATE_INTERVAL + (5*1000);    //at 30 minutes, do an update to notify sensors have been warmed up. additional 15-20 seconds to avoid colliding with other messages


// Task scheduler
#include <TaskScheduler.h>

//sensors
Adafruit_BME680 bme;     //I2C
Adafruit_CCS811 ccs;     //I2C



int errorLED = 13;     //used to indicate error


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

//retry limit
const int MAX_RETRY_LIMIT = 20;
const int MAX_WIFI_RETRY_LIMIT = 15;



WiFiClient wifiClient;
Scheduler runner;
Task updateMonitoring(UPDATE_INTERVAL_SECONDS, TASK_FOREVER, &UpdateMonitoring);
Task warmupPeriodDone(WARMUP_PERIOD, 1, &WarmupNotify);

void setup() {
  Serial.begin(9600);
  pinMode(errorLED, OUTPUT);

  // Giving it a little time because the serial monitor doesn't
  // immediately attach. Want the firmware that's running to
  // appear on each upload.
  delay(2000);

  Serial.println();
  Serial.println("Running Firmware.");

  SetupWifi();

  SetupBME680();        //Temperature, Pressure, Humidity, Gas (TVOC), Approx Altitude
  SetupCCS811();        //Temperature, CO2 (PPM), TVOC (PPB)

  //begin thingspeak
  ThingSpeak.begin(wifiClient);

  Serial.println("Air monitor started and online.");
  PostStatusMessage("Air monitor started and online.");

  //execute the monitoring thread
  Serial.println("Starting monitoring...");
  runner.init();
  runner.addTask(updateMonitoring);
  runner.addTask(warmupPeriodDone);
  updateMonitoring.enable();
  warmupPeriodDone.enable();
}


void loop() {
  runner.execute();

}


//setup the wifi, can be called to reconnect if disconnected
void SetupWifi(){

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
  WiFi.setHostname(WIFI_HOSTNAME);
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


  //print debug information
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Hello World, I'm connected to the internets!!");
}



int bme680SetupRetry = 0;
void SetupBME680(){
  while (!Serial);
  Serial.println("BME680 starting...");

  bme680SetupRetry = 0;
  while (!bme.begin()) {
    Serial.println("Failed to start BME680 sensor! Please check your wiring. Retrying...");
    bme680SetupRetry++;
    if (bme680SetupRetry > MAX_RETRY_LIMIT){
      ErrorLoop("Cannot start BME680 sensor!");
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

}

int ccs811SetupRetry = 0;
void SetupCCS811(){
  Serial.println("CCS811 starting...");

  ccs811SetupRetry = 0;
  while(!ccs.begin()){
    Serial.println("Failed to start CCS811 sensor! Please check your wiring. Retrying...");
    ccs811SetupRetry++;
    if (ccs811SetupRetry > MAX_RETRY_LIMIT){
      ErrorLoop("Cannot start CCS811 sensor!");
    }
    delay(2000);
  }
  
  Serial.println("CCS811 started.");

  //calibrate temperature sensor
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
}



unsigned int frameCount = 0;
void UpdateMonitoring(){

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
    SetupWifi();        //attempt reconnect
  }

  Serial.println();


  //BME680
  Serial.println("=====BME680=====");
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading from BME680 :(");
    PostStatusMessage("Failed to perform reading from BME680 :(");
    return;
  }

  float temperature = bme.temperature + TEMP_OFFSET;
  float pressure = (bme.pressure / 100.0) + PRESSURE_OFFSET;
  float humidity = bme.humidity + HUMIDITY_OFFSET;
  float gas_resistance = (bme.gas_resistance / 1000.0f) + GAS_RESISTANCE_OFFSET;
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA) + ALTITUDE_OFFSET;
  

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(gas_resistance);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(altitude);
  Serial.println(" m");

  Serial.println();


  //CCS811
  Serial.println("=====CCS811=====");
  if(!ccs.available()){
    Serial.println("Error! CCS811 not available.");
    PostStatusMessage("Error! CCS811 not available.");
    return;
  }
  if(ccs.readData()){
    Serial.println("Failed to perform reading from CCS811 :(");
    PostStatusMessage("Failed to perform reading from CCS811 :(");
    return;
  }
  
  //add enviromental data to improve readings
  ccs.setEnvironmentalData(bme.humidity + HUMIDITY_OFFSET, bme.temperature + TEMP_OFFSET);

  float co2 = ccs.geteCO2() + CO2_OFFSET;
  float tvoc = ccs.getTVOC() + TVOC_OFFSET;
  float temperature_estimate = ccs.calculateTemperature() + TEMP_811_OFFSET;

  Serial.print("CO2: ");
  Serial.print(co2);
  Serial.print("ppm, TVOC: ");
  Serial.print(tvoc);
  Serial.print("ppb   Temp:");
  Serial.println(temperature_estimate);
 

  Serial.println();
  Serial.println();


  frameCount++;


  //now send information to ThingSpeak
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, pressure);
  ThingSpeak.setField(3, humidity);
  ThingSpeak.setField(4, gas_resistance);
  ThingSpeak.setField(5, altitude);
  ThingSpeak.setField(6, co2);
  ThingSpeak.setField(7, tvoc);
  ThingSpeak.setField(8, temperature_estimate);
  
  ThingSpeak.writeFields(THING_SPEAK_CHANNEL_ID, WRITE_API_KEY);
}


void WarmupNotify(){
  Serial.println("Sensors have sufficiently warmed up for 30 minutes");
  PostStatusMessage("Sensors have sufficiently warmed up for 30 minutes");
}



//Note: max bytes for a status message is 255 bytes
void ErrorLoop(String errorMessage){
  Serial.println("An error has occured! Please restart and check connections. Message:");
  Serial.println(errorMessage);

  String statusMessage = "Error! Needs restart. ";
  statusMessage += errorMessage;
  PostStatusMessage(statusMessage);

  //loop forever in error land
  while(1){
    digitalWrite(errorLED, HIGH);
    delay(1000);
    digitalWrite(errorLED, LOW);
    delay(1000);

    Serial.println("An error has occured! Please restart and check connections. Message:");
    Serial.println(errorMessage);
  }
}

void PostStatusMessage(String statusMessage){

  if (WiFi.status() == WL_CONNECTED){
    ThingSpeak.setStatus(statusMessage);
    ThingSpeak.writeFields(THING_SPEAK_CHANNEL_ID, WRITE_API_KEY);
  }
}
