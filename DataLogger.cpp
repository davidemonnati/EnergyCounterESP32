#include "include/DataLogger.h"

const int chipSelect = 5;

void setupDataLogger(){
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("card initialized.");
}

void writeDataFile(char* topic, char* data) {
  File dataFile = SD.open("/consumptions_data.txt", FILE_APPEND);
  
  char consumption[60];
  sprintf(consumption, "%s|%s", topic, data);
  if(dataFile) {
    dataFile.println(String(consumption));
    dataFile.close();
  }else {
    Serial.println("Error opening consumptions_data.txt");
  }
}
