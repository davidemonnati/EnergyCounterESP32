#ifndef rgbled_h
#define rgbled_h

#include <Arduino.h>

void setupLed();
void setRedLight();
void setGreenLight();
void setBlueLight();
void turnOffLed();
void setRgbColor(int red_light_value, int green_light_value, int blue_light_value);

#endif