#include "include/RGBLed.h"

int red_light_pin = 4;
int green_light_pin = 16;
int blue_light_pin = 17;

void setupLed(){
    pinMode(red_light_pin, OUTPUT);
    pinMode(green_light_pin, OUTPUT);
    pinMode(blue_light_pin, OUTPUT);
}

void setRedLight(){
  digitalWrite(red_light_pin, HIGH);
  digitalWrite(green_light_pin, LOW);
  digitalWrite(blue_light_pin, LOW);
}

void setGreenLight(){
  digitalWrite(red_light_pin, LOW);
  digitalWrite(green_light_pin, HIGH);
  digitalWrite(blue_light_pin, LOW);
}

void setBlueLight(){
  digitalWrite(red_light_pin, LOW);
  digitalWrite(green_light_pin, LOW);
  digitalWrite(blue_light_pin, HIGH);
}

void turnOffLed(){
  digitalWrite(red_light_pin, LOW);
  digitalWrite(green_light_pin, LOW);
  digitalWrite(blue_light_pin, LOW);
}

void setRgbColor(int red_light_value, int green_light_value, int blue_light_value){
    digitalWrite(red_light_pin, red_light_value);
    digitalWrite(green_light_pin, green_light_value);
    digitalWrite(blue_light_pin, blue_light_value);
} 
