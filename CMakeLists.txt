# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(4.2_weather_portal)

add_executable(4.2_weather_portal.ino)

find_package(Arduino.h REQUIRED) 
find_package(ArduinoJson.h REQUIRED) 
find_package(StreamUtils.h REQUIRED) 
find_package(ESP32MultiWiFiProvision.h REQUIRED) 
find_package(HTTPClient.h REQUIRED)
find_package(U8g2_for_Adafruit_GFX.h REQUIRED)