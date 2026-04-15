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

unsigned long DAY = 80000000;
unsigned long HOUR = 3000000;
// unsigned long DAY = 80000;
// unsigned long HOUR = 3000;
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
// fullscreen.startX = 0;    // left margin
// fullscreen.startY = 0;    // top margin
// fullscreen.endX = 400;    // right margin
// fullscreen.endY = 300;    // bottom margin

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
  if (currentMillis - lastFullRefresh >= DAY) {
    lastFullRefresh = currentMillis; // save the last executed time
    Serial.println("Full Refresh");
    EPD_Init();
    clear_all();

    lastAPICall = currentMillis;
    PrintData(fullscreen);
    EPD_Sleep();
  }

  if (currentMillis - lastAPICall >= HOUR) {
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

  JsonDocument data = fetch_data(client);

  JsonVariant current = data["current"];
  float temp = current["temp"];
  char stemp[10];

  //Serial.println(current);

  String display_text = String(String("Current Temperature is: ")+ dtostrf(temp, 2, 0, stemp));

  Part_Text_Display(display_text.c_str(), block.startX, block.startY, fontSize, BLACK, block.endX, block.endY);
  //bool weather = fetch_data(client);

  //display Block 1

  //EPD_ShowPicture(0, 0, 312, 152, gImage_1, WHITE); // Display image gImage_1, starting coordinates (0, 0), width 312, height 152, background color white
  // EPD_ShowPicture(0, 0, 352, 104, gImage_tiaoma_1, WHITE);
  EPD_Display_Fast(Image_BW); // Quickly display the image stored in the Image_BW array
}

void display_current(int x, int y, int w, int h){

  EPD_ShowPicture(0, 0, 134, 150, , BLACK)

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