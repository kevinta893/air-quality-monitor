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
#define UPDATE_INTERVAL_SECONDS (3 * 60 * 1000)
#define WARMUP_PERIOD ((30 * 60 * 1000) + MIN_UPDATE_INTERVAL + (5*1000))    //at 30 minutes, do an update to notify sensors have been warmed up. additional 15-20 seconds to avoid colliding with other tasks
#define RESTART_INVERVAL ((1 * 24 * 60 * 60 * 1000))          // for resetting every few days

// Retry limit
#define MAX_WIFI_RETRY_LIMIT 15

// System
#define ERROR_LED_PIN 13     //used to indicate error


// Scheduling
Scheduler runner;
Task updateMonitoring(UPDATE_INTERVAL_SECONDS, TASK_FOREVER, &UpdateMonitoring);
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

  Serial.println("Air monitor started and online.");
  PostStatusMessage("Air monitor started and online.");

  //Execute the monitoring thread
  Serial.println("Starting monitoring...");
  runner.init();
  runner.addTask(updateMonitoring);
  runner.addTask(warmupPeriodDone);
  runner.addTask(restartSystem);
  updateMonitoring.enable();
  warmupPeriodDone.enableDelayed();
  restartSystem.enableDelayed();
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
  
  int httpStatus = ThingSpeak.writeFields(THING_SPEAK_CHANNEL_ID, WRITE_API_KEY);
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

  //loop forever in error land
  while(1){
    digitalWrite(ERROR_LED_PIN, HIGH);
    delay(1000);
    digitalWrite(ERROR_LED_PIN, LOW);
    delay(1000);

    Serial.println("An error has occured! Please restart and check connections. Message:");
    Serial.println(errorMessage);
  }
}

/**
 * Sends a status message to the thingspeak channel
 */
void PostStatusMessage(String statusMessage){

  //Note: max bytes for a status message is 255 bytes
  if (WiFi.status() == WL_CONNECTED){
    ThingSpeak.setStatus(statusMessage);
    ThingSpeak.writeFields(THING_SPEAK_CHANNEL_ID, WRITE_API_KEY);
  }
}
