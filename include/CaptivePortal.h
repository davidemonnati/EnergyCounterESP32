#ifndef captiveportal_h
#define captiveportal_h

#include <SD.h>
#include <SPI.h>
#include <AsyncWebSocket.h>
#include <AsyncWebSynchronization.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "esp_wpa2.h" //wpa2 library for connections to Enterprise networks

String index_page(int num_ssid, String ssid_list[50]);
void setupNetwork();
void scanNetwork(int *num_ssid, String ssid_list[50]);
boolean selectEncryptionType(String encryption, String ssid, String username, String password);
String getEncryptionType(wifi_auth_mode_t encryptionType);
String printNetwork(int num_ssid, String ssid_list[50]);
boolean connect_open_network(char* ssid);
boolean connect_wpa(char* ssid, char* password);
boolean connect_wpa2_enterprise(char* ssid, char* username, char* password);
boolean saveSettingsToSdCard(String encryption, String ssid, String username, String password, char mqtt_server[], char outTopic_Ap1[], char outTopic_Ap2[], char outTopic_Ap3[], char outTopic_Ap4[]);
boolean searchConfigurationFile();

#endif