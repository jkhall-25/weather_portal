#ifndef fetch_weather_h_
#define fetch_weather_h_

struct W_DATA {
  String time;
  int temp;
  int feelsLike;
  int high;
  int low;
  int humidity;
  int temp_morn;
  int temp_day;
  int temp_eve;
  int temp_night;
  int chance_precip;
  int icon_now;
  int icon_today;
  int is_day;
};

void fetch_data(WiFiClient& client);
bool parse_data(JsonDocument json);
void connect_wifi();
unsigned char* fetch_icon(WiFiClient& client, String code);


#endif