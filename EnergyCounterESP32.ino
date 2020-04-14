#include <WiFi.h>
#include <NTPClient.h>

#include <WiFiUdp.h>
#include "RTClib.h"


TaskHandle_t Task1;
RTC_DS1307 rtc;

// =================== INTERRUPTS E CONSUMI =======================
struct impulso {
  DateTime t;
  unsigned long dur;
};
struct consumo {
  String  t;
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

  Serial.print("ORA RTC: ");
  Serial.print(rtc.now().timestamp());

  pinMode(14, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  pinMode(15, INPUT);

  attachInterrupt(digitalPinToInterrupt(14), ap1_int, FALLING);
  attachInterrupt(digitalPinToInterrupt(12), ap2_int, FALLING);
  attachInterrupt(digitalPinToInterrupt(13), ap3_int, FALLING);
  attachInterrupt(digitalPinToInterrupt(15), ap4_int, FALLING);

  xTaskCreate (
    setTimeInt1,    // Function that should be called
    "Task1",   // Name of the task (for debugging)
    1000,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );
  delay(500);
}

void setTimeInt1( void * parameter ) {
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

void loop() {
  // put your main code here, to run repeatedly:

}
