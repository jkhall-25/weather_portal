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

extern struct W_DATA today;
extern struct W_DATA tomorrow;
extern struct W_DATA hr3;
extern struct W_DATA hr6;
extern struct W_DATA hr9;
extern struct W_DATA hr12;


void fetch_data(WiFiClient& client)
{
  Serial.begin(115200);
  Serial.println("Fetching weather data...");

String uri = "/v1/forecast?latitude=42.3584&longitude=-71.0598&daily=temperature_2m_max,temperature_2m_min,weather_code,\
precipitation_probability_max,precipitation_hours,wind_speed_10m_max,apparent_temperature_max,apparent_temperature_min,\
temperature_2m_mean,apparent_temperature_mean,uv_index_max&hourly=temperature_2m,apparent_temperature,precipitation_probability,\
precipitation,rain,snowfall,showers,weather_code,is_day&minutely_15=is_day,rain,snowfall,snowfall_height,temperature_2m,\
relative_humidity_2m,apparent_temperature,precipitation,weather_code&timezone=America%2FNew_York&past_days=0&forecast_days=3\
&wind_speed_unit=mph&temperature_unit=fahrenheit&precipitation_unit=inch&forecast_minutely_15=4&forecast_hours=15&temporal_resolution=hourly_3";

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

  if (error) {
    Serial.print("deserialization failed: ");
    Serial.println(error.c_str());
  }
  else {parse_data(data);}
}

bool parse_data(JsonDocument payload) {

  JsonVariant data = payload.as<JsonVariant>();
  JsonVariant minutely = data["minutely_15"];
  JsonVariant daily = data["daily"];
  JsonVariant hourly = data["hourly"];

  today.time = String(minutely["time"][0]).substring(11);

  float temp = minutely["temperature_2m"][0];
  today.temp = int(round(temp));
  float feelsLike = minutely["apparent_temperature"][0];
  today.feelsLike = int(round(feelsLike));
  today.icon = int(minutely["weather_code"][0]);
  today.is_day = int(minutely["is_day"][0]);
  float high = daily["temperature_2m_max"][0];
  today.high = int(round(high));
  float low = daily["temperature_2m_min"][0];
  today.low = int(round(low));
  float f_high = daily["apparent_temperature_max"][0];
  today.feel_high = int(round(f_high));
  float f_low = daily["apparent_temperature_min"][0];
  today.feel_low = int(round(f_low));
  today.chance_precip = daily["precipitation_probability_max"][0];
  today.hours_precip = daily["precipitation_hours"][0];
  today.UV = daily["uv_index_max"][0];

  hr3.time = String(hourly["time"][0]).substring(11) + "-" + String(hourly["time"][1]).substring(11);
  temp = hourly["temperature_2m"][0];
  hr3.temp = int(round(temp));
  feelsLike = hourly["apparent_temperature"][0];
  hr3.feelsLike = int(round(feelsLike));
  hr3.chance_precip = hourly["precipitation_probability"][0];
  hr3.icon = int(hourly["weather_code"][0]);
  hr3.is_day = int(hourly["is_day"][0]);

  hr6.time = String(hourly["time"][1]).substring(11) + "-" + String(hourly["time"][2]).substring(11);
  temp = hourly["temperature_2m"][1];
  hr6.temp = int(round(temp));
  feelsLike = hourly["apparent_temperature"][1];
  hr6.feelsLike = int(round(feelsLike));
  hr6.chance_precip = hourly["precipitation_probability"][1];
  hr6.icon = int(hourly["weather_code"][1]);
  hr6.is_day = int(hourly["is_day"][1]);

  hr9.time = String(hourly["time"][2]).substring(11) + "-" + String(hourly["time"][3]).substring(11);
  temp = hourly["temperature_2m"][2];
  hr9.temp = int(round(temp));
  feelsLike = hourly["apparent_temperature"][2];
  hr9.feelsLike = int(round(feelsLike));
  hr9.chance_precip = hourly["precipitation_probability"][2];
  hr9.icon = int(hourly["weather_code"][2]);
  hr9.is_day = int(hourly["is_day"][2]);

  hr12.time = String(hourly["time"][3]).substring(11) + "-" + String(hourly["time"][4]).substring(11);
  temp = hourly["temperature_2m"][3];
  hr12.temp = int(round(temp));
  feelsLike = hourly["apparent_temperature"][3];
  hr12.feelsLike = int(round(feelsLike));
  hr12.chance_precip = hourly["precipitation_probability"][3];
  hr12.icon = int(hourly["weather_code"][3]);
  hr12.is_day = int(hourly["is_day"][3]);

  temp = daily["temperature_2m_mean"][1];
  tomorrow.temp = int(round(temp));
  feelsLike = daily["apparent_temperature_mean"][1];
  tomorrow.feelsLike = int(round(feelsLike));
  tomorrow.icon = int(daily["weather_code"][1]);
  tomorrow.is_day = int(daily["is_day"][1]);
  high = daily["temperature_2m_max"][1];
  tomorrow.high = int(round(high));
  low = daily["temperature_2m_min"][1];
  tomorrow.low = int(round(low));
  f_high = daily["apparent_temperature_max"][1];
  tomorrow.feel_high = int(round(f_high));
  f_low = daily["apparent_temperature_min"][1];
  tomorrow.feel_low = int(round(f_low));
  tomorrow.chance_precip = daily["precipitation_probability_max"][1];
  tomorrow.hours_precip = daily["precipitation_hours"][1];
  tomorrow.UV = daily["uv_index_max"][1];
  
  return true;
}

void connect_wifi(){
  WiFi.begin(ssid, password);
}