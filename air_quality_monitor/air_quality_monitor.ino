// Sensors
#include "BME680.h"
#include "CCS811.h"

// WiFi
#include "AirMonitor_WiFi.h"

// ThingSpeak API
#include "AirMonitor_ThingSpeak.h"

// Task scheduler
#include <TaskScheduler.h>

// Task scheduling constants
#define MIN_UPDATE_INTERVAL (15 * 1000)
#define UPDATE_INTERVAL (3 * 60 * 1000)
#define WARMUP_PERIOD ((30 * 60 * 1000) + MIN_UPDATE_INTERVAL + (5*1000))    //at 30 minutes, do an update to notify sensors have been warmed up. additional 15-20 seconds to avoid colliding with other tasks
#define RESTART_INVERVAL ((1 * 24 * 60 * 60 * 1000))          // for resetting every few days

// Retry limit
#define MAX_WIFI_RETRY_LIMIT 15

// System
#define ERROR_LED_PIN 13     //used to indicate error


// Scheduling
Scheduler runner;
Task updateMonitoring(UPDATE_INTERVAL, TASK_FOREVER, &UpdateMonitoring);
Task warmupPeriodDone(WARMUP_PERIOD, 1, &WarmupNotify);
Task restartSystem(RESTART_INVERVAL, TASK_FOREVER, &RestartSystemRegular);

/**
 * Setup system
 */
void setup() {
  Serial.begin(9600);
  pinMode(ERROR_LED_PIN, OUTPUT);

  // Giving it a little time because the serial monitor doesn't
  // immediately attach. Want the firmware that's running to
  // appear on each upload.
  delay(2000);

  Serial.println();
  Serial.println("Running Firmware.");

  //Setup WiFi
  if (!SetupWifi()){
    ErrorLoop("Cannot start WiFi!");
  }

  //Setup sensors
  if (!SetupBME680()){
    ErrorLoop("Cannot start BME680 sensor!");
  }
  if (!SetupCCS811()){
    ErrorLoop("Cannot start CCS811 sensor!");
  }

  //Setup ThingSpeak
  if (!SetupThingSpeak(wifiClient)){
    ErrorLoop("Cannot start ThingSpeak!");
  }


  Serial.println("Monitoring configuration:");
  Serial.print("Update interval (ms)=");
  Serial.println(UPDATE_INTERVAL);
  Serial.print("Warmup time (ms)=");
  Serial.println(WARMUP_PERIOD);
  Serial.print("Restart interval (ms)=");
  Serial.println(RESTART_INVERVAL);
  
  //Execute the monitoring thread
  Serial.println("Starting monitoring...");
  runner.init();
  runner.addTask(updateMonitoring);
  runner.addTask(warmupPeriodDone);
  runner.addTask(restartSystem);
  updateMonitoring.enable();
  warmupPeriodDone.enableDelayed();
  restartSystem.enableDelayed();

  Serial.println("Air monitor started and online.");
  PostStatusMessage("Air monitor started and online.");
}

/**
 * Run the scheduling loop
 */
void loop() {
  runner.execute();
}

unsigned int frameCount = 0;
/**
 * Updates ThingSpeak with new monitoring values
 */
void UpdateMonitoring(){

  //Current frame header
  Serial.print("--------------Frame #");
  Serial.print(frameCount);
  Serial.println("--------------");



  //Wifi state
  Serial.println("# WIFI State");
  WiFi.status();
  WiFi.localIP();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi is Connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi disconnected :(");
    Serial.println("Reconnecting...");
    SetupWifi();        //attempt reconnect
  }

  Serial.println();


  //BME680
  Serial.println("# BME680");

  BME680DataFrame bme680Reading;
  if (!ReadBME680(&bme680Reading)){
    String errorMessage = "Failed to perform reading from BME680 :(";
    Serial.println(errorMessage);
    PostStatusMessage(errorMessage);
    return;
  }
  

  Serial.print("Temperature = ");
  Serial.print(bme680Reading.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme680Reading.pressure);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme680Reading.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme680Reading.gas_resistance);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme680Reading.altitude);
  Serial.println(" m");

  Serial.println();


  //CCS811
  Serial.println("# CCS811");

  
  //Read CCS811
  CCS811DataFrame ccs811Reading;
  if (!ReadCCS811(bme680Reading.humidity, bme680Reading.temperature, &ccs811Reading)){
    String errorMessage = "Failed to perform reading from CCS811 :(";
    Serial.println(errorMessage);
    PostStatusMessage(errorMessage);
    return;
  }

  Serial.print("CO2: ");
  Serial.print(ccs811Reading.co2);
  Serial.print("ppm, TVOC: ");
  Serial.print(ccs811Reading.tvoc);
  Serial.print("ppb,   Temp:");
  Serial.println(ccs811Reading.temperature_estimate);
 

  Serial.println();
  Serial.println("# ThingSpeak");



  //Send sensor readings to ThingSpeak
  ThingSpeakUpdateFrame updateFrame;
  updateFrame.temperature = bme680Reading.temperature;
  updateFrame.pressure = bme680Reading.pressure;
  updateFrame.humidity = bme680Reading.humidity;
  updateFrame.gas_resistance = bme680Reading.gas_resistance;
  updateFrame.altitude = bme680Reading.altitude;
  updateFrame.co2 = ccs811Reading.co2;
  updateFrame.tvoc = ccs811Reading.tvoc;
  updateFrame.temperature_estimate = ccs811Reading.temperature_estimate;

  
  int httpStatus = WriteFieldsThingSpeak(&updateFrame);
  if (httpStatus != 200){
    digitalWrite(ERROR_LED_PIN, HIGH);
    Serial.print("Error writing to Thingspeak Channel. HTTP Status code= ");
    Serial.println(httpStatus);
  }
  else{
    digitalWrite(ERROR_LED_PIN, LOW);
    Serial.print("ThingSpeak update successful. HTTP Status code=");
    Serial.println(httpStatus);
  }

  frameCount++;
}

/**
 * Callback for warmup period complete message
 */
void WarmupNotify(){
  Serial.println("Sensors have sufficiently warmed up for 30 minutes");
  PostStatusMessage("Sensors have sufficiently warmed up for 30 minutes");
}

/** 
 * Restarts the system at a specified time for regular maintenance
 */
void RestartSystemRegular(){
  PostStatusMessage("Executing regularly scheduled reset. Restarting system...");
  Serial.println("Executing regularly scheduled system restart.");
  ResetSystem();
}

/**
 * Restarts the system
 */
void ResetSystem(){
  Serial.println("Restarting system...");
  esp_restart();
}


/**
 * Puts the system into a perpetual error stage with the
 * Error LED blinking
 */
void ErrorLoop(String errorMessage){
  Serial.println("An error has occured! Please restart and check connections. Message:");
  Serial.println(errorMessage);

  String statusMessage = "Error! Needs restart. ";
  statusMessage += errorMessage;
  PostStatusMessage(statusMessage);

  //loop forever in error land blinking
  while(1){
    digitalWrite(ERROR_LED_PIN, HIGH);
    delay(1000);
    digitalWrite(ERROR_LED_PIN, LOW);
    delay(1000);

    Serial.println("An error has occured! Please restart and check connections. Message:");
    Serial.println(errorMessage);
  }
}
