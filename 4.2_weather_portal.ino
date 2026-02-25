#include <Arduino.h>        // Include the core library for Arduino platform development
#include "EPD.h"            // Include the EPD library to control the E-Paper Display
#include "EPD_GUI.h"        // Include the EPD_GUI library which provides GUI functionalities
#include <ArduinoJson.h>    // https://github.com/bblanchon/ArduinoJson
#include <WiFi.h>           //built in
#include "time.h"           //built in
#include <SPI.h>            //built in
#include <WiFiClient.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "epaper_fonts.h"
#include "fetch_weather.h"

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

  EPD_GPIOInit();                // Initialize the GPIO pin configuration for the EPD e-ink screen

  // The SPI initialization part is commented out
  // default baud is 115200
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  SPI.begin ();
  status = connect_wifi();

  // WiFi.init();
  // if (!WiFi.is_initialized()){
  //   SPI.println("Could not connect to WiFi.")
  // }
  // if (!Wifi.is_connected()){
  //   WiFi.register_sta(
  //     "home",
  //     {.ssid = ssid,
  //     .password = password,
  //     .num_connect_retries = CONFIG_ESP_MAXIMUM_RETRY,
  //     .auto_connect = true,
  //     .on_got_ip = [&](ip_event_got_ip_t *eventdata) 
  //     {
  //       logger.info("Home network - got IP: {}.{}.{}.{}", IP2STR(&eventdata->ip_info.ip));
  //     },
  //     .log_level = espp::Logger::Verbosity::INFO},
  //     set_active=true;
  //     );
  // }
    // Now manually initiate the connection
  

  PrintData(fullscreen);
  
  EPD_Sleep();                // Set the screen to sleep mode to save power

  delay(5000);                // Wait for 5000 milliseconds (5 seconds), allowing the screen to stay in sleep mode for some time

  clear_all();               // Call the clear_all function to clear the screen content
}

void loop() {
  // Main loop function, currently does not perform any actions
  // Code that needs to be repeatedly executed can be added here
}

void PrintData(BLOCK block)
{
  const char *My_Text = "The quick brown fox jumped over the lazy dog";

  /*First refresh the screen*/
  EPD_Clear();                   // Clear the screen content, restoring it to its default state
  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create a new image buffer, size EPD_W x EPD_H, background color white
  EPD_Full(WHITE);              // Fill the entire canvas with white
  EPD_Display_Part(0, 0, EPD_W, EPD_H, Image_BW); // Display the image stored in the Image_BW array
  //EPD_Init(); // initalize slowly
  EPD_Init_Fast(Fast_Seconds_1_5s); // Quickly initialize the EPD screen, setting it to 1.5 second fast mode

  Part_Text_Display(My_Text, block.startX, block.startY, fontSize, BLACK, block.endX, block.endY);
  bool weather = fetch_data(client);

  //display Block 1

  //EPD_ShowPicture(0, 0, 312, 152, gImage_1, WHITE); // Display image gImage_1, starting coordinates (0, 0), width 312, height 152, background color white
  // EPD_ShowPicture(0, 0, 352, 104, gImage_tiaoma_1, WHITE);
  EPD_Display_Fast(Image_BW); // Quickly display the image stored in the Image_BW array
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