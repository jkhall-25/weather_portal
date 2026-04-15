#ifndef fetch_weather_h_
#define fetch_weather_h_

JsonDocument fetch_data(WiFiClient& client);
bool parse_data(JsonDocument json);
void connect_wifi();

#endif