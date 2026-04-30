#ifndef fetch_weather_h_
#define fetch_weather_h_

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
void connect_wifi();
unsigned char* fetch_icon(WiFiClient& client, String code);


#endif