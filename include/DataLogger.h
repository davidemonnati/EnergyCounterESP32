#ifndef datalogger_h
#define datalogger_h

#include <SD.h>
#include <SPI.h>


void setupDataLogger();
void writeDataFile(char* topic, char* data);

#endif