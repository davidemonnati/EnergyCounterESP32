#ifndef datalogger_h
#define datalogger_h

#include <SD.h>
#include <SPI.h>
#include <PubSubClient.h>


boolean setupDataLogger();
void writeConsumptionToFile(char* topic, char* data);
void resendBackupData(PubSubClient *client, char* clientID);

#endif