// Thingspeak Debugging
//#define PRINT_DEBUG_MESSAGES
//#define PRINT_HTTP

#include <ThingSpeak.h>
#include "ThingSpeak_API_Keys.h"

struct ThingSpeakUpdateFrame{
  float temperature;
  float pressure;
  float humidity;
  float gas_resistance;
  float altitude;
  float co2;
  float tvoc;
  float temperature_estimate;
};

/**
 * Setup ThingSpeak API
 */
bool SetupThingSpeak(WiFiClient &wifiClient){
  ThingSpeak.begin(wifiClient);

  return true;
}

/**
 * Sends a field update to the ThingSpeak API
 * Returns an http status code. 200 is a success
 */
int WriteFieldsThingSpeak(ThingSpeakUpdateFrame* updateFrame){
  ThingSpeak.setField(1, updateFrame->temperature);
  ThingSpeak.setField(2, updateFrame->pressure);
  ThingSpeak.setField(3, updateFrame->humidity);
  ThingSpeak.setField(4, updateFrame->gas_resistance);
  ThingSpeak.setField(5, updateFrame->altitude);
  ThingSpeak.setField(6, updateFrame->co2);
  ThingSpeak.setField(7, updateFrame->tvoc);
  ThingSpeak.setField(8, updateFrame->temperature_estimate);
  
  int httpStatus = ThingSpeak.writeFields(THING_SPEAK_CHANNEL_ID, WRITE_API_KEY);

  return httpStatus;
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
