#ifndef fetch_weather_h_
#define fetch_weather_h_

bool fetch_data(WiFiClient& client);
bool parse_data(WiFiClient& json);
int connect_wifi();

#endif