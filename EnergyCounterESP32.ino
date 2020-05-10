#include <WiFi.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <RTClib.h>

#include "include/Consumo.h"
#include "include/Impulso.h"
#include "include/ManageTime.h"
#include "include/DataLogger.h"
#include "include/CaptivePortal.h"

TaskHandle_t Task1;
TaskHandle_t Task2;

RTC_DS1307 rtc;
AsyncWebServer webServer(80);
int num_ssid = 0;
String ssid_list[50];
volatile boolean is_connected = false;
volatile boolean setup_completed = false;
volatile boolean sdcard_inserted = false;

// =================== INTERRUPTS E CONSUMI =======================
int i,j,k;
unsigned long prev_millis;
int toll = 0;
DateTime cur_ts;
bool consumo_b;

struct impulso Impulsi1[300];
int dimI1 = 0;
struct consumo Consumi1[300];
int dimC1 = 0;

struct impulso Impulsi2[300];
int dimI2 = 0;
struct consumo Consumi2[300];
int dimC2 = 0;

struct impulso Impulsi3[300];
int dimI3 = 0;
struct consumo Consumi3[300];
int dimC3 = 0;

struct impulso Impulsi4[300];
int dimI4 = 0;
struct consumo Consumi4[300];
int dimC4 = 0;

unsigned long t1_1 = 0;
unsigned long Tt1 = 0;
unsigned long t1_0 = 0;

volatile bool flag_ap1 = false;
volatile bool flag_ap2 = false;
volatile bool flag_ap3 = false;
volatile bool flag_ap4 = false;

void IRAM_ATTR ap1_int() {
  t1_1 = millis();
  Tt1 = (t1_1 - t1_0);
  if (Tt1 > 1000) {
    Impulsi1[dimI1].dur = Tt1;
    t1_0 = t1_1;
    flag_ap1 = true;
  }
}

void IRAM_ATTR ap2_int() {
  t1_1 = millis();
  Tt1 = (t1_1 - t1_0);
  if (Tt1 > 1000) {
    Impulsi2[dimI2].dur = Tt1;
    t1_0 = t1_1;
    flag_ap2 = true;
  }
}

void IRAM_ATTR ap3_int() {
  t1_1 = millis();
  Tt1 = (t1_1 - t1_0);
  if (Tt1 > 1000) {
    Impulsi3[dimI3].dur = Tt1;
    t1_0 = t1_1;
    flag_ap3 = true;
  }
}

void IRAM_ATTR ap4_int() {
  t1_1 = millis();
  Tt1 = (t1_1 - t1_0);
  if (Tt1 > 1000) {
    Impulsi4[dimI4].dur = Tt1;
    t1_0 = t1_1;
    flag_ap4 = true;
  }
}

// =================== FINE INTERRUPTS E CONSUMI =======================

// Topic MQTT
String ssid;
String username;
String password;
String WiFiEnc;
char mqtt_server[15];
char *clientID = "Building0Test";
char outTopic_Ap1[10];
char outTopic_Ap2[10];
char outTopic_Ap3[10];
char outTopic_Ap4[10];
WiFiClient espClient;
PubSubClient client(espClient);

boolean startWebServer() {
  Serial.println("Starting web server...");
  
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", index_page(num_ssid, ssid_list) );
  });
  
  webServer.on("/connect", HTTP_POST, [] (AsyncWebServerRequest * request) {
    int params = request->params();
    String wifi_data[params];
    for (int i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);

      if (p->isPost()) {
        wifi_data[i] = p->value().c_str();
      }
    }

    String ssid_to_string = wifi_data[0].substring(1, wifi_data[0].length());
    String encryption;
    
    for (int i = 0; i < num_ssid; i++) {
      if (ssid_list[i] == ssid_to_string.substring(0, ssid_to_string.length()-1)) {
        encryption = getEncryptionType(WiFi.encryptionType(i));
        break;
      }
    }
    WiFiEnc = encryption;
    wifi_data[3].toCharArray(mqtt_server, wifi_data[3].length()+1);
    wifi_data[4].toCharArray(outTopic_Ap1, wifi_data[4].length()+1);
    wifi_data[5].toCharArray(outTopic_Ap2, wifi_data[5].length()+1);
    wifi_data[6].toCharArray(outTopic_Ap3, wifi_data[6].length()+1);
    wifi_data[7].toCharArray(outTopic_Ap4, wifi_data[7].length()+1);

    ssid = ssid_to_string.substring(0, wifi_data[0].length()-1);
    password = wifi_data[2];

    Serial.println("Disabling access point...");
    request->send(200, "text/plain", "Connessione alla rete in corso...<br /><br />Modalità AP disabilitata." );
    WiFi.softAPdisconnect(true);
    is_connected = selectEncryptionType(encryption, ssid, wifi_data[1], wifi_data[2]);
  });
  webServer.begin();
  Serial.println("Web server is on.");
  return true;
}

void defineInterrupts(){
  pinMode(14, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  pinMode(15, INPUT);
  attachInterrupt(digitalPinToInterrupt(14), ap1_int, FALLING);
  attachInterrupt(digitalPinToInterrupt(12), ap2_int, FALLING);
  attachInterrupt(digitalPinToInterrupt(13), ap3_int, FALLING);
  attachInterrupt(digitalPinToInterrupt(15), ap4_int, FALLING);
}

void setupRoutines(){
  Serial.println("Starting setup...");
  setupRTC(&rtc);
  syncTimeWithNTP(&rtc);

  client.setServer(mqtt_server, 1883);
  Serial.print("ORA RTC: ");
  Serial.println(rtc.now().timestamp());

  defineInterrupts();
  xTaskCreate (
    setTimeInt,    // Function that should be called
    "Task1",   // Name of the task (for debugging)
    20000,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );
  setup_completed = true;
}

void runTask2(void * parameter ){
  for(;;){
    if(is_connected && !setup_completed){
      setupRoutines();
    }
    if(setup_completed){
      if(sdcard_inserted)
        saveSettingsToSdCard(WiFiEnc, ssid, username, password, mqtt_server, outTopic_Ap1, outTopic_Ap2, outTopic_Ap3, outTopic_Ap4);
      
      Serial.println("Setup completed.");
      vTaskDelete(Task2);
    }
  }
}

void loadSettingsFromSDCard(){
  int i = 0;
  String configurationData[8];
  File configFile = SD.open("/connection.config", FILE_READ);
  if(configFile.available()){
    while(configFile.available()){
      configurationData[i] = configFile.readStringUntil('\n');
      i++;
    }
    ssid = configurationData[0];
    username = configurationData[1];
    password = configurationData[2];
    WiFiEnc = configurationData[3];
    configurationData[4].toCharArray(mqtt_server, configurationData[4].length());
    configurationData[5].toCharArray(outTopic_Ap1, configurationData[5].length());
    configurationData[6].toCharArray(outTopic_Ap2, configurationData[6].length());
    configurationData[7].toCharArray(outTopic_Ap3, configurationData[7].length());
    configurationData[8].toCharArray(outTopic_Ap4, configurationData[8].length());
  }
}

void setup() { 
  Serial.begin(115200);
  sdcard_inserted = setupDataLogger();
  if(searchConfigurationFile()){
    Serial.println("Configuration file found\nLoading settings from SD card...");
    loadSettingsFromSDCard();
    is_connected = selectEncryptionType(WiFiEnc.substring(0, WiFiEnc.length()-1), ssid.substring(0, ssid.length()), username.substring(0, username.length()-1),
      password.substring(0, password.length()-1));
    
    setupRoutines();
  }else{
    Serial.println("Configuration file doesn't exists\nStarting captive portal...");
    if(!sdcard_inserted){
      Serial.println("SD card not detected, the configuration will not be saved.");
    }
    scanNetwork(&num_ssid, ssid_list);
    printNetwork(num_ssid, ssid_list);
    setupNetwork();
    startWebServer();
    xTaskCreate (runTask2, "Task2", 10000, NULL, 2, NULL);
  }
}

void setTimeInt( void * parameter ) {
  //Serial.print("Task1 running on core ");
  //Serial.println(xPortGetCoreID());
  for (;;) {
    if (flag_ap1) {
      Serial.println("interrupt 1");
      Impulsi1[dimI1].t = rtc.now();
      dimI1++;
      flag_ap1 = false;
    }

    if (flag_ap2) {
      Serial.println("interrupt 2");
      Impulsi2[dimI2].t = rtc.now();
      dimI2++;
      flag_ap2 = false;
    }

    if (flag_ap3) {
      Serial.println("interrupt 3");
      Impulsi3[dimI3].t = rtc.now();
      dimI3++;
      flag_ap3 = false;
    }

    if (flag_ap4) {
      Serial.println("interrupt 4");
      Impulsi4[dimI4].t = rtc.now();
      dimI4++;
      flag_ap4 = false;
    } 
  }
}

void sendMqttData(char* topic, consumo consumi[], int *dim){
  char buffer0[30];
  String payload;
  int i=0;
  client.connect(clientID);
  
  Serial.println("Invio vettori in corso...");
  for(int j=0; j<*dim; j++) {
    if(consumi[j].sent==0){
      dtostrf(consumi[j].w,4,0,buffer0);
      payload=getTimestamp(consumi[j].t)+"_"+buffer0;
      payload.toCharArray(buffer0,30);
      if (client.connected()) {
        client.publish(topic, buffer0);
        Serial.print("MQTT DATA: ");
        Serial.print(topic);
        Serial.print(" -> ");
        Serial.println(payload);
        delay(100);
      }else {
        Serial.println("Non riesco a ricollegarmi.");
        if(sdcard_inserted){
          writeDataFile(topic, buffer0); 
        }
      }
      Consumi1[j].sent=true;
     }
    }
    *dim=0;
}

void convertToWatt(impulso *impulsi, consumo consumi[], int *dimI, int *dimC) {
  if(*dimI == 0){
    Serial.println("Impulsi a vuoto carico!");
    consumi[0].w=0;
    consumi[0].t = rtc.now();
    *dimC=1; 
    } else{
      j=0;
      k=1;
      prev_millis=impulsi[0].dur;
      cur_ts=impulsi[0].t;
      
      for(i=1; i < *dimI; i++){
        toll=(prev_millis/100);
        if(impulsi[i].dur < (prev_millis-toll) || impulsi[i].dur > (prev_millis+toll)){
          consumi[j].t=cur_ts;
          consumi[j].w=3600*1000/prev_millis;
          consumi[j].sent=false;
          j++;
          cur_ts=impulsi[i].t;
          prev_millis=impulsi[i].dur; 
          k=1;
          consumo_b=true; 
          }else{
            prev_millis=(impulsi[i].dur+prev_millis*k)/(k+1);
            k++;
            consumo_b=false;
          }
      }

      if(!consumo_b){
        consumi[j].t=cur_ts;
        consumi[j].w=3600*1000/prev_millis;  
        consumi[j].sent=false;
        j++;
        cur_ts=impulsi[i].t;
        prev_millis=impulsi[i].dur;
      }
      
      impulsi[0].dur =impulsi[*dimI-1].dur;
      impulsi[0].t   =impulsi[*dimI-1].t;
      *dimI=1;
      *dimC=j;
    }
}

void loop() {
  delay(180000);
  if(is_connected && setup_completed){
    convertToWatt(Impulsi1, Consumi1, &dimI1, &dimC1);
    convertToWatt(Impulsi2, Consumi2, &dimI2, &dimC2);
    convertToWatt(Impulsi3, Consumi3, &dimI3, &dimC3);
    convertToWatt(Impulsi4, Consumi4, &dimI4, &dimC4);
  
    sendMqttData(outTopic_Ap1, Consumi1, &dimC1);
    sendMqttData(outTopic_Ap2, Consumi2, &dimC2);
    sendMqttData(outTopic_Ap3, Consumi3, &dimC3);
    sendMqttData(outTopic_Ap4, Consumi4, &dimC4);

    if(sdcard_inserted)
      resendBackupData(&client, clientID);
  }
}
