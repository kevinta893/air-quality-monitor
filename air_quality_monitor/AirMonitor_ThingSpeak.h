#include <ThingSpeak.h>
#include "ThingSpeak_API_Keys.h"

/**
 * Setup ThingSpeak API
 */
bool SetupThingSpeak(WiFiClient &wifiClient){
  ThingSpeak.begin(wifiClient);

  return true;
}
