#include <Arduino.h>        // Include the core library for Arduino platform development
#include "EPD.h"            // Include the EPD library to control the E-Paper Display
#include "EPD_GUI.h"        // Include the EPD_GUI library which provides GUI functionalities
#include "esp_sleep.h"
#include <ArduinoJson.h>    // https://github.com/bblanchon/ArduinoJson
#include <WiFi.h>           //built in
#include "time.h"           //built in
#include <SPI.h>            //built in
#include <WiFiClient.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "epaper_fonts.h"
#include "fetch_weather.h"
#include "icons.h"

#define FULL_REFRESH_INTERVAL 80000000 // 24 hours
#define REFRESH_DATA_INTERVAL 750000 // 15 minutes

unsigned long lastFullRefresh = 0; // vairable to save the last executed time for code block 1
unsigned long lastAPICall = 0; // vairable to save the last executed time for code block 2

int status = WL_IDLE_STATUS;

uint8_t Image_BW[15000];    // Declare an array of 15000 bytes to store black and white image data

int fontSize = 24; // Default font size

int icon_width = 72;

WiFiClient client;

struct BLOCK {
  int startX, startY, endX, endY;
};
BLOCK fullscreen = {0, 0, 400, 300};
BLOCK status_bar = {0, 0, 400, 12};
BLOCK current_block = {0, 14, 200, 150};
BLOCK tomorrow_block = {200, 14, 400, 150};
BLOCK block_3h = {0, 150, 100, 300};
BLOCK block_6h = {100, 150, 200, 300};
BLOCK block_9h = {200, 150, 300, 300};
BLOCK block_12h = {300, 150, 400, 300};

      // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
      // X ____status______400 x 14____________________X
      // X                     X                       X
      // X        today        X        tomorrow       X
      // X      200 x 150      X       200 x 150       X
      // X                     X                       X
      // X                     X                       X
      // X                     X                       X
      // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
      // X    3h    X    6h    X    9h     X    12h    X
      // X          X          X           X           X
      // X   100    X   100    X    100    X   100     X
      // X    x     X    x     X     x     X    x      X
      // X   150    X   150    X    150    X   150     X
      // X          X          X           X           X
      // X          X          X           X           X
      // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

struct W_DATA today;
struct W_DATA tomorrow;
struct W_DATA hr3;
struct W_DATA hr6;
struct W_DATA hr9;
struct W_DATA hr12;

void setup() {
  // Initialization settings, executed only once when the program starts
  // Configure pin 7 as output mode and set it to high level to activate the screen power
  pinMode(7, OUTPUT);            // Set pin 7 as output mode
  digitalWrite(7, HIGH);         // Set pin 7 to high level, activating the screen power
  Serial.begin(115200);
  Serial.println("Booting up...");

  EPD_GPIOInit();                // Initialize the GPIO pin configuration for the EPD e-ink screen

  // The SPI initialization part is commented out
  // default baud is 115200
  // SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  // SPI.begin ();
  connect_wifi();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi.");

  clear_all();

  PrintData(fullscreen);
  
  EPD_Sleep();                // Set the screen to sleep mode to save power

}

void loop() {

  unsigned long currentMillis = millis();

  //if it's been longer than a day since the last full screen refresh, refresh the screen
  if (currentMillis - lastFullRefresh >= FULL_REFRESH_INTERVAL) {
    lastFullRefresh = currentMillis; // save the last executed time
    Serial.println("Full Refresh");
    EPD_Init();
    clear_all();

    lastAPICall = currentMillis;
    PrintData(fullscreen);
    EPD_Sleep();
  }

  if (currentMillis - lastAPICall >= REFRESH_DATA_INTERVAL) {
    lastAPICall = currentMillis; // save the last executed time
    Serial.println("updating data");
    EPD_Init();
    PrintData(fullscreen);
    EPD_Sleep();
  }

}

void PrintData(BLOCK block)
{
  const char *My_Text = "The quick brown fox jumped over the lazy dog";

  /*First refresh the screen*/
  //EPD_Clear();                   // Clear the screen content, restoring it to its default state
  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create a new image buffer, size EPD_W x EPD_H, background color white
  EPD_Full(WHITE);              // Fill the entire canvas with white
  EPD_Display_Part(0, 0, EPD_W, EPD_H, Image_BW); // Display the image stored in the Image_BW array
  //EPD_Init(); // initalize slowly
  EPD_Init_Fast(Fast_Seconds_1_5s); // Quickly initialize the EPD screen, setting it to 1.5 second fast mode

  fetch_data(client);

  display_status(status_bar);
  display_top(current_block, today, "Tod:");
  display_top(tomorrow_block, tomorrow, "Tom:");
  display_bottom(block_3h, hr3);
  display_bottom(block_6h, hr6);
  display_bottom(block_9h, hr9);
  display_bottom(block_12h, hr12);

  // draw the boxes 
  EPD_DrawLine(1, 12, 400, 12, BLACK); // top horizontal line
  EPD_DrawLine(1, 150, 400, 150, BLACK); // middle horizontal line
  EPD_DrawLine(200, 12, 200, 300, BLACK); // middle vertical line
  EPD_DrawLine(100, 150, 100, 300, BLACK); // bottom Q1
  EPD_DrawLine(300, 150, 300, 300, BLACK); // bottom Q3

  EPD_Display_Fast(Image_BW); // Quickly display the image stored in the Image_BW array
}

void display_status(BLOCK block) {
  int fontSize = 12;
  String status_text = String("Last updated: ") + today.time;
  Part_Text_Display(status_text.c_str(), block.startX, block.startY, fontSize, BLACK, block.endX, block.endY);
}

void display_top(BLOCK block, W_DATA weather_data, char* heading) {
  int startX = block.startX;
  int startY = block.startY;
  int endX = block.endX;
  int endY = block.endY;
  int font1 = 32;
  int font2 = 48;
  int indent = 5;
  int icon_offset = 70;

  //  Part_Text_Display(STRING, startX, startY, fontSize, color, endX, endY); 
  Part_Text_Display(heading, startX+indent, startY, font1, BLACK, endX, (startY+font1));  

  
  char current_temp[4]; // maximum 3 characters + null term
  int temp = weather_data.temp;
  int err = snprintf(current_temp, 4, "%02d", temp);
  if (err>= 0 && err<100){
    if (temp < 100 && temp >= 0){
      Part_Text_Display(current_temp, (startX+indent), (startY+font1), font2, BLACK, endX, endY);
    }
    else { // if the temp is 3 digits. if the temp is 4 digits, we have bigger problems.
      Part_Text_Display(current_temp, (startX), (startY+font1), font2, BLACK, endX, endY);
      icon_offset = 74;
    }
  }

  char feels_temp[6]; // maximum 3 characters + null term + 2 parens
  int f_temp = weather_data.feelsLike;
  err = snprintf(feels_temp, 6, "(%02d)", f_temp);
  if (err>= 0 && err<100){
    if (f_temp < 100 && f_temp >= 0){
      Part_Text_Display(feels_temp, (startX), (startY+font1+font2), font1, BLACK, endX, endY);
    }
    else {
      Part_Text_Display(feels_temp, (startX), (startY+font1+font2), font2, BLACK, endX, endY);
      icon_offset = 74;
    }
  }

  char UV[10]; 
  float uv = weather_data.UV;
  err = snprintf(UV, 10, " UV: %.1f", uv);
  if (err>= 0 && err<100) {
      Part_Text_Display(UV, (startX), (startY+font1+font2+font1), 16, BLACK, endX, endY);
  }

  char high_low[10];
  err = snprintf (high_low, 10, "%d/%d", weather_data.high, weather_data.low);
  if (err>=0 && err<100){
    Part_Text_Display(high_low, (startX+icon_offset+(font1/2)), (startY+icon_width), font1, BLACK, endX, endY);
  }

  char high_low_feel[10];
  err = snprintf(high_low_feel, 10, "(%d/%d)", weather_data.feel_high, weather_data.feel_low);
  if (err>=0 && err<100){
    Part_Text_Display(high_low_feel, (startX+icon_offset), (startY+icon_width+font1), font1, BLACK, endX, endY);
  }

  // display icon
  display_icon((startX+icon_offset), startY, weather_data.icon);

  // precipitation 
  char chance_rain[5];
  if (weather_data.chance_precip == 100) {
    weather_data.chance_precip = 99;
  }
  err = snprintf(chance_rain, 5, "%02d%%", weather_data.chance_precip);
  if (err>=0 && err<100){
    Part_Text_Display(chance_rain, (startX + icon_offset + icon_width + 5), (startY), font1, BLACK, endX, endY);
  }
  char hours_precip[4];
  err = snprintf(hours_precip, 4, "%02dh", weather_data.hours_precip);
  if (err>=0 && err<100){
    Part_Text_Display(hours_precip, (startX + icon_offset + icon_width + 5), (startY + font1), font1, BLACK, endX, endY);
  }
}

void display_bottom(BLOCK block, W_DATA weather_data){
  int font1 = 16;
  int font2 = 24;
  int time_indent = 5;
  int temp_indent = 14;
  int rain_indent = 35;
  display_icon((block.startX+14), block.startY, weather_data.icon);

  Part_Text_Display(weather_data.time.c_str(), (block.startX+time_indent), (block.startY+icon_width), font1, BLACK, block.endX, block.endY);

  char temps[10];
  int err = snprintf(temps, 10,  "%d(%d)", weather_data.temp, weather_data.feelsLike);
  if (err>=0 && err<100) {
    Part_Text_Display(temps, (block.startX+temp_indent), (block.startY + icon_width + font1), font2, BLACK, block.endX, block.endY);
  }

  char rain[5];
  if (weather_data.chance_precip == 100) {
    weather_data.chance_precip = 99;
  }
  err = snprintf(rain, 5,  "%02d%%", weather_data.chance_precip);
  if (err>=0 && err<100) {
    Part_Text_Display(rain, (block.startX+rain_indent), (block.startY + icon_width + font1 + font2), font2, BLACK, block.endX, block.endY);
  }
}

void display_icon(int x, int y, int code) {

  const unsigned char* icon;

  if (code >= 0 && code <=2) {
    if (today.is_day == 1) {
        if (code == 0) { icon = i_sunny; }
        if (code == 1) { icon = i_mostly_clear_day; }
        if (code == 2) { icon = i_partly_cloudy_day; }
    }
    if (today.is_day == 0) { 
      if (code == 0) { icon = i_moon; }
      if (code == 1) { icon = i_mostly_clear_night; }
      if (code == 2) { icon = i_partly_cloudy_night; }
    }
  }
  else if (code == 3) { icon = i_cloudy; } //cloudy
  else if (code == 45 || code == 48) { icon = i_fog; } // fog
  else if (code > 50 && code <= 55) { icon = i_drizzle; } // drizzle
  else if (code > 55 && code < 60) { icon = i_freezing_rain; } // freezing drizzle
  else if (code > 60 && code <= 65) { icon = i_rain; } // rain 
  else if (code > 65 && code < 70) { icon = i_freezing_rain; } // freezing rain 
  else if ((code > 70 && code < 80) || (code >= 85 && code <= 86)) { icon = i_snow; } // snow
  else if (code > 90) { icon = i_tstorms; } // thunderstorms
  else {
    icon = i_unknown;
  }

  EPD_ShowPicture(x, y, icon_width, icon_width, icon, WHITE);

}

void clear_all() {
  // Function to clear the screen content
  EPD_Clear();                 // Clear the screen content, restoring it to its default state
  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create a new image buffer, size EPD_W x EPD_H, background color white
  EPD_Full(WHITE);            // Fill the entire canvas with white
  EPD_Display_Part(0, 0, EPD_W, EPD_H, Image_BW); // Display the image stored in the Image_BW array
}

/*
*---------Function description: Display text content locally------------
*----Parameter introduction:
      content: Text content
      startX: Starting horizontal axis
      startY: Starting vertical axis
      fontSize: Font size
      color: Font color
      endX: End horizontal axis
      endY: End vertical axis
*/
void Part_Text_Display(const char* content, int startX, int startY, int fontSize, int color, int endX, int endY) {
    int length = strlen(content);
    int i = 0;
    char line[(endX - startX) / (fontSize/2) + 1]; // Calculate the maximum number of characters per line based on the width of the area
    int currentX = startX;
    int currentY = startY;
    int lineHeight = fontSize;

    while (i < length) {
        int lineLength = 0;
        memset(line, 0, sizeof(line));

        // Fill the line until it reaches the width of the region or the end of the string
        while (lineLength < (endX - startX) / (fontSize/2) && i < length) {
            line[lineLength++] = content[i++];
        }

        // If the current Y coordinate plus font size exceeds the area height, stop displaying
        if (currentY + lineHeight > endY) {
            break;
        }
        // Display this line
        EPD_ShowString(currentX, currentY, line, fontSize, color); 

        // Update the Y coordinate for displaying the next line
        currentY += lineHeight;

        // If there are still remaining strings but they have reached the bottom of the area, stop displaying them
        if (currentY + lineHeight > endY) {
            break;
        }
    }
}