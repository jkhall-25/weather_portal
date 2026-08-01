#ifndef _fetch_weather_h_
#define _fetch_weather_h_

#include "connect_wifi.h"
#include <WiFiClient.h>

struct W_DATA {
  String time;
  int temp;
  int feelsLike;
  int high;
  int low;
  int feel_high;
  int feel_low;
  int chance_precip;
  int hours_precip;
  int icon;
  int is_day;
  float UV;
};

void fetch_data(WiFiClient& client);
bool parse_data(JsonDocument json);
unsigned char* fetch_icon(WiFiClient& client, String code);
void locate(WiFiClient& client);
void query(WiFiClient& client, char q_data[], const char server[], String uri);


#endif