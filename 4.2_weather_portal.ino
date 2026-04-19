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

  String display_text = String(String("Current Temperature is: ") + weather_data.temp);

  Part_Text_Display(display_text.c_str(), block.startX, block.startY, fontSize, BLACK, block.endX, block.endY);
  if (weather_data.icon_now != NULL) {
    display_icon(0, 150, 128, 128, weather_data.icon_now);
  }


  //display Block 1

  //EPD_ShowPicture(0, 0, 312, 152, gImage_1, WHITE); // Display image gImage_1, starting coordinates (0, 0), width 312, height 152, background color white
  // EPD_ShowPicture(0, 0, 352, 104, gImage_tiaoma_1, WHITE);
  EPD_Display_Fast(Image_BW); // Quickly display the image stored in the Image_BW array
}

void display_icon(int x, int y, int w, int h, int code) {

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

  EPD_ShowPicture(x, y, w, h, icon, WHITE);

  // const unsigned char** icon_array = NULL;
  // const unsigned char* icon;

  // const unsigned char* day_icons[15] = {i_01d, i_02d, i_03d, i_04d, 0, 0, 0, 0, i_09d, i_10d, i_11d, 0, i_13d, i_50d};
  // const unsigned char* night_icons[15] = {i_01n, i_02n, i_03n, i_04d, 0, 0, 0, 0, i_09d, i_10d, i_11d, 0, i_13d, i_50d};

  // char time = '0';

  // if (code.indexOf('d') >= 0){icon_array = day_icons;}
  // else if (code.indexOf('n') >= 0){icon_array = night_icons;}

  // Serial.println(code);
  
  // //if code contains a d or an n:

  // if (icon_array != NULL) {
  //   code[-1] = '\0';
  //   int index = std::stoi(code.c_str());
  //   index -= 1;
  //   Serial.println(index);
  //   if (index < sizeof(icon_array) && index >= 0){icon = icon_array[index];}
  //   else if (index == 50){icon = icon_array[13];}
  //   if (icon == 0){
  //     EPD_ShowPicture(x, y, w, h, i_unknown, WHITE);
  //   }
  //   else {
  //   EPD_ShowPicture(x, y, w, h, icon, WHITE);
  //   }
  // }
  // else{
  //   EPD_ShowPicture(x, y, w, h, i_unknown, WHITE);
  // }
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