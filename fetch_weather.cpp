#include <HTTPClient.h>
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <math.h>
#include "fetch_weather.h"
#include "owm_credentials.h"

bool DEBUG = true;
const int DATA_LEN = 4096;

void fetch_data(WiFiClient& client);
bool parse_data(JsonDocument json);
void connect_wifi();
unsigned char* fetch_icon(WiFiClient& client, String code);

char bytes[DATA_LEN];

extern struct W_DATA weather_data;


void fetch_data(WiFiClient& client)
{
  Serial.begin(115200);
  Serial.println("Fetching weather data...");

  //String uri = "/data/3.0/onecall?lat="+LAT+"&lon="+LON+"&appid="+apikey+"&units=imperial&exclude=minutely";
//   String uri = "/v1/forecast?latitude=42.3584&longitude=-71.0598&daily=temperature_2m_max,temperature_2m_min,\
// apparent_temperature_max,apparent_temperature_min,precipitation_hours&hourly=temperature_2m,\
// apparent_temperature,precipitation_probability,precipitation,rain,snowfall,showers,weather_code,\
// is_day&minutely_15=is_day,rain,snowfall,snowfall_height,temperature_2m,relative_humidity_2m,\
// apparent_temperature,precipitation,weather_code&timezone=America%2FNew_York\
// &forecast_days=3&wind_speed_unit=mph&temperature_unit=fahrenheit&precipitation_unit=inch\
// &forecast_hours=12&temporal_resolution=hourly_3&forecast_minutely_15=4";
 String uri = "/v1/forecast?latitude=42.3584&longitude=-71.0598&daily=temperature_2m_max,temperature_2m_min,\
apparent_temperature_max,apparent_temperature_min,weather_code,precipitation_probability_max,precipitation_hours\
&hourly=temperature_2m,apparent_temperature,precipitation_probability,precipitation,rain,snowfall,showers,\
weather_code,is_day&minutely_15=is_day,rain,snowfall,snowfall_height,temperature_2m,relative_humidity_2m,\
apparent_temperature,precipitation,weather_code&timezone=America%2FNew_York&forecast_days=3&wind_speed_unit=mph\
&temperature_unit=fahrenheit&precipitation_unit=inch&forecast_hours=12&temporal_resolution=hourly_3&forecast_minutely_15=4";
  client.connect(server, 80);
  Serial.println(server);
  if (!client.connected()){
    Serial.println("Connection failed.");
  }
  LoggingStream loggingClient(client, Serial);
  loggingClient.print("GET "+uri+" HTTP/1.0\r\n" +
                      "Host: "+String(server)+"\r\n" +
                      "Connection: close\r\n" +
                      "\r\n"
                      );
  while (client.connected() || client.available()) {
    if (client.available()) {
      client.readBytesUntil('\n', bytes, DATA_LEN);
    }
  }

  client.stop();
  Serial.println("disconnected");

  JsonDocument data;
  DeserializationError error = deserializeJson(data, bytes);
  //Serial.println(bytes);

  if (error) {
    Serial.print("deserialization failed: ");
    Serial.println(error.c_str());
  }
  else {parse_data(data);}
}

bool parse_data(JsonDocument payload) {

  JsonVariant data = payload.as<JsonVariant>();
  JsonVariant minutely = data["minutely_15"];

  float temp = minutely["temperature_2m"][0];
  weather_data.temp = int(round(temp));

  float feelsLike = minutely["apparent_temperature"][0];
  weather_data.feelsLike = int(round(feelsLike));

  weather_data.icon_now = int(minutely["weather_code"][0]);

  weather_data.is_day = int(minutely["is_day"][0]);

  float high = data["daily"]["temperature_2m_max"][0];
  weather_data.high = int(round(high));

  float low = data["daily"]["temperature_2m_min"][0];
  weather_data.low = int(round(low));

  weather_data.chance_precip = data["daily"]["precipitation_probability_max"][0];

  // int high;
  // int low;
  // int humidity;
  // int temp_morn;
  // int temp_day;
  // int temp_eve;
  // int temp_night;
  // int chance_precip;
  // int icon_now;
  // int icon_today;
  // int is_day;
  
  return true;

  
}

void connect_wifi(){
  WiFi.begin(ssid, password);
}