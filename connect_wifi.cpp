#include <Arduino.h>
#include <ESP32MultiWiFiProvision.h>        // device provisioning
#include <WiFi.h>
#include <WiFiClient.h>
#include "EPD.h"            // Include the EPD library to control the E-Paper Display
#include "EPD_GUI.h"        // Include the EPD_GUI library which provides GUI functionalities

#define MENU 2 // MENU button
#define EXIT 1 //exit button
int menu_button_pressed = 0;

ESP32MultiWiFiProvision wifiConfig;

int status = WL_IDLE_STATUS;
const char* ssid     = "";
const char* password = "";


void connect_wifi(){
  /* WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(50000);
    Serial.print("Can't Connect!\n");
  }

  Serial.println("Connected to WiFi.");*/
  Serial.println("Connecting...");
  wifiConfig.begin("Smart Device", NULL, false);

  wifiConfig.onConnected([](String ssid) {
      Serial.println("Connected to: " + ssid);
  });

  if (!wifiConfig.connect()) {       // Blocks for up to 40s
      wifiConfig.startPortal();      // Fallback to portal
  }

  Serial.println("Status: " + wifiConfig.getStatusMessage());
}

void maintain_wifi(){
  wifiConfig.run();
  if (digitalRead(EXIT) == LOW) {
    wifiConfig.resetSettings();
    connect_wifi();
  }
}

String wifi_status(){
  String status;
  status = wifiConfig.getStatusMessage();
  return status;
}

bool connected(){
  bool connected = wifiConfig.isConnected();
  return connected; 
}
