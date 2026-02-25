#include <HTTPClient.h>
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include "fetch_weather.h"
#include "owm_credentials.h"

bool DEBUG = pdTRUE;

bool fetch_data(WiFiClient& client);
bool parse_data(WiFiClient& json);
int connect_wifi();

bool fetch_data(WiFiClient& client)
{
  Serial.println("Fetching weather data...");

  client.stop();
  HTTPClient http;

  String uri = "/data/3.0/onecall?lat={"+LAT+"}&lon={"+LON+"}&exclude={part}&appid={"+apikey+"}&units={imperial}&exclude={minutely}";
  http.begin(client, server, 80, uri);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK){
    if (!parse_data(http.getStream())) return false;
    client.stop();
    http.end();
    return true;
  }
  else
  {
    Serial.printf("connection failed, error %s", http.errorToString(httpCode).c_str());
    client.stop();
    http.end();
    return false;
  }
  http.end();
  return true;
}

bool parse_data(WiFiClient& json)
{
  Serial.println(json);
  return true;
}

int connect_wifi(){
  WiFi.begin(ssid, password);
}