#include <WiFi.h>
#include <NTPClient.h>
#include <PubSubClient.h>

#include <WiFiUdp.h>
#include "RTClib.h"

TaskHandle_t Task1;
RTC_DS1307 rtc;

// =================== INTERRUPTS E CONSUMI =======================
int i,j,k;
unsigned long prev_millis;
int toll = 0;
DateTime cur_ts;
bool consumo_b;
  
struct impulso {
  DateTime t;
  unsigned long dur;
};

struct consumo {
  DateTime  t;
  int     w;
  bool    sent;
};

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
const char* ssid     = "ssid";
const char* password = "password";

// Topic MQTT
char* mqtt_server = "192.168.1.14";
char* clientID = "Building0Test"; 
char* outTopic_Ap1 = "N/121";
char* outTopic_Ap2 = "N/122";
char* outTopic_Ap3 = "N/123";
char* outTopic_Ap4 = "N/124";
WiFiClient espClient;
PubSubClient client(espClient);


void setupWiFi() {
  Serial.printf("Connecting to: %s\n", ssid);
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi connected!\n");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void setupRTC() {
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (! rtc.isrunning())
    Serial.println("RTC is NOT running!");
}

void syncTimeWithNTP() {
  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 3600;
  const int   daylightOffset_sec = 3600;

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;

  if (getLocalTime(&timeinfo))
    rtc.adjust(DateTime((timeinfo.tm_year - 100), timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
}

void setup() { 
  Serial.begin(115200);

  setupWiFi();
  setupRTC();
  syncTimeWithNTP();
  
  client.setServer(mqtt_server, 1883);

  Serial.print("ORA RTC: ");
  Serial.println(rtc.now().timestamp());

  pinMode(14, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  pinMode(15, INPUT);

  attachInterrupt(digitalPinToInterrupt(14), ap1_int, FALLING);
  attachInterrupt(digitalPinToInterrupt(12), ap2_int, FALLING);
  attachInterrupt(digitalPinToInterrupt(13), ap3_int, FALLING);
  attachInterrupt(digitalPinToInterrupt(15), ap4_int, FALLING);

  xTaskCreate (
    setTimeInt,    // Function that should be called
    "Task1",   // Name of the task (for debugging)
    20000,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );
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

String getTimestamp(DateTime dateTime) {
  return String(dateTime.year()) + "-" + String(dateTime.month()) + "-" + String(dateTime.day()) 
    + "T" + String(dateTime.hour()) + ":" + String(dateTime.minute()) + ":" +String(dateTime.second());;
}

void sendMqttData(char* topic, consumo consumi[], int *dim){
  char buffer0[30];
  String payload;
  int i=0;
  
  while ((!client.connected()) && (i<3)) {
    Serial.println("Client disconnesso da MQTT.. provo la riconnessione :");
    delay(1000);
    client.connect(clientID);
    i++;
  }
  if (client.connected()) {
    Serial.println("Mi sono collegato al broker MQTT:");
    Serial.println("Invio vettori in corso...");
    for(int j=0; j<*dim; j++) {
      if(consumi[j].sent==0){
        dtostrf(consumi[j].w,4,0,buffer0);
        payload=getTimestamp(consumi[j].t)+"_"+buffer0;
        Serial.print("MQTT DATA: ");
        Serial.print(topic);
        Serial.print(" -> ");
        Serial.println(payload);
        payload.toCharArray(buffer0,30);  
        client.publish(topic, buffer0);
        Consumi1[j].sent=1;
        delay(200);
      }
    }
    *dim=0;
  }else{
    Serial.println("Non riesco a ricollegarmi :");
    Serial.println("Scrivo su Flash e proverò tra un minuto"); 
    // scrittura su sd
  }  
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
  delay(60000);
  convertToWatt(Impulsi1, Consumi1, &dimI1, &dimC1);
  convertToWatt(Impulsi2, Consumi2, &dimI2, &dimC2);
  convertToWatt(Impulsi3, Consumi3, &dimI3, &dimC3);
  convertToWatt(Impulsi4, Consumi4, &dimI4, &dimC4);

  sendMqttData(outTopic_Ap1, Consumi1, &dimC1);
  sendMqttData(outTopic_Ap2, Consumi2, &dimC2);
  sendMqttData(outTopic_Ap3, Consumi3, &dimC3);
  sendMqttData(outTopic_Ap4, Consumi4, &dimC4);
}
