#include <HTTPClient.h>
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "fetch_weather.h"
#include "owm_credentials.h"


bool DEBUG = true;
const int DATA_LEN = 512;

JsonDocument fetch_data(WiFiClient& client);
bool parse_data(JsonDocument json);
void connect_wifi();
unsigned char* fetch_icon(WiFiClient& client, String code);

String bytes;
JsonDocument data;

struct {
  float temp;
  float feelsLike;
  float high;
  float low;
  float humidity;
  float temp_morn;
  float temp_day;
  float temp_eve;
  float temp_night;
  float chance_precip;
  String desc;
  String icon;
} weather_data;

JsonDocument fetch_data(WiFiClient& client)
{
  Serial.begin(115200);
  Serial.println("Fetching weather data...");

  String uri = "/data/3.0/onecall?lat="+LAT+"&lon="+LON+"&appid="+apikey+"&units=imperial&exclude=minutely";
  //api.openweathermap.org/data/3.0/onecall?lat=42.29519506567491&lon=-71.11523920701491&appid=c0674e8af5cb95fdc247aab4eff3c996&units=imperial&exclude=minutely
  client.connect(server, 80);
  Serial.println(server);
  if (!client.connected()){
    Serial.println("Connection failed.");
    return data;
  }
  LoggingStream loggingClient(client, Serial);
  loggingClient.print("GET "+uri+" HTTP/1.1\r\n" +
                      "Host: "+String(server)+"\r\n" +
                      "Connection: close\r\n" +
                      "\r\n"
                      );
  while (client.connected() || client.available()) {
    if (client.available()) {
      bytes = client.readStringUntil('\n');
      Serial.println(bytes);
    }
  }

  client.stop();
  Serial.println("disconnected");

  deserializeJson(data, bytes);

  //parse_data(data);

  return data;
}

bool parse_data(JsonDocument json)
{
  String data;
  serializeJson(json, data);
  Serial.println(data);
  return true;
}

void connect_wifi(){
  WiFi.begin(ssid, password);
}