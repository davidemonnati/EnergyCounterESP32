#include "include/DataLogger.h"

const int chipSelect = 5;

boolean setupDataLogger(){
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return false;
  }
  Serial.println("card initialized.");
  return true;
}

void writeConsumptionToFile(char* topic, char* data) {
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

void resendBackupData(PubSubClient *client, char* clientID){
  char bufferTopic[6];
  char bufferConsumption[30];
  client->connect(clientID);
  String buffer;
  File readFile = SD.open("/consumptions_data.txt");
  
  if(readFile && client->connected()){
    while (readFile.available()) {
      buffer = readFile.readStringUntil('\n');
      int ind1 = buffer.indexOf("|");
      String topic = buffer.substring(0,ind1);
      int ind2 = buffer.indexOf("\n");
      String consumption = buffer.substring(ind1+1, ind2);
      topic.toCharArray(bufferTopic, 6);
      consumption.toCharArray(bufferConsumption, 30);
      client->publish(bufferTopic, bufferConsumption);
    }
    SD.remove("/consumptions_data.txt");
  }
}
