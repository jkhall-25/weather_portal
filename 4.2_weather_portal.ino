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

WiFiClient client;

struct BLOCK {
  int startX, startY, endX, endY;
};
BLOCK fullscreen = {0, 0, 400, 300};
BLOCK status_bar = {0, 0, 400, 12};
BLOCK current_block = {0, 14, 134, 164};

struct W_DATA weather_data;

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
  // Main loop function, currently does not perform any actions
  // Code that needs to be repeatedly executed can be added here

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
  display_current(current_block);

  //String display_text = String("Current Temperature is: ") + weather_data.temp;

  //Part_Text_Display(display_text.c_str(), block.startX, block.startY, fontSize, BLACK, block.endX, block.endY);
  //display_icon(0, 150, 0);

  EPD_Display_Fast(Image_BW); // Quickly display the image stored in the Image_BW array
}

void display_status(BLOCK block) {
  int fontSize = 12;
  String status_text = String("Last updated: ") + weather_data.time;
  Part_Text_Display(status_text.c_str(), block.startX, block.startY, fontSize, BLACK, block.endX, block.endY);
}

void display_current(BLOCK block) {
  int startX = block.startX;
  int startY = block.startY;
  int endX = block.endX;
  int endY = block.endY;
  int fontSize = 32;
  Part_Text_Display("Now:", startX, startY, fontSize, BLACK, endX, (startY+fontSize));
  // startY += fontSize; // move down past current text
  fontSize = 48;
  
  char current_temp[3];
  itoa(weather_data.temp, current_temp, 10);
  //= weather_data.temp;
  Serial.println(current_temp);
  Part_Text_Display(current_temp, startX, (startY+32), fontSize, BLACK, endX, endY);
  //String forecast_today = weather_data.high + "/" + weather_data.low;
  //Part_Text_Display(forecast_today.c_str(), block.endX-50, block.startY, font, BLACK, block.endX, block.endY);
  display_icon((startX+75), startY, weather_data.icon_now);
}

void display_icon(int x, int y, int code) {

  const unsigned char* icon;

  if (code >= 0 && code <=2) {
    if (weather_data.is_day == 1) {
        if (code == 0) { icon = i_sunny; }
        if (code == 1) { icon = i_mostly_clear_day; }
        if (code == 2) { icon = i_partly_cloudy_day; }
    }
    if (weather_data.is_day == 0) { 
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

  EPD_ShowPicture(x, y, 88, 88, icon, WHITE);

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