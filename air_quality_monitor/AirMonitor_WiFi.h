#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// WiFi credentials. See external file
#include "WiFi_Credentials.h"

#define WIFI_HOSTNAME "Arduino Air Monitor"

// Network config
// If using ESP32, refer to https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi
IPAddress local_ip(0,0,0,0);      //Leave as 0.0.0.0 to leave as auto
IPAddress gateway(0,0,0,0);       //Leave as 0.0.0.0 to leave as auto
IPAddress subnet(0,0,0,0);        //Leave as 0.0.0.0 to leave as auto
IPAddress dns1(1,1,1,1);          //Optional
IPAddress dns2(1,0,0,1);          //Optional

// Access wifi using this client
WiFiClient wifiClient;


/**
 * Setup the wifi
 * Can call this function again to reconnect
 */
bool SetupWifi(){

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
  WiFi.config(local_ip, gateway, subnet, dns1, dns2);
  WiFi.setHostname(WIFI_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  delay(5000);
  
  int checkCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    // Check to see if connecting failed.
    // This is due to incorrect credentials
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WIFI. Please verify credentials: ");
      Serial.println();
      Serial.print("SSID: ");
      Serial.println(WIFI_SSID);
      return false;
    }

    delay(5000);
    Serial.print("#");
    Serial.print(checkCount++);
    Serial.println(" Waiting for WiFi...");
  }


  //Connected! Yay


  //print debug information
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Hello World, I'm connected to the internets!!");

  return true;
}
