#include <WiFi.h>
#include <NTPClient.h>

#include <WiFiUdp.h>
#include "RTClib.h"

RTC_DS1307 rtc;

const char* ssid     = "ssid";
const char* password = "password";

void setupWiFi(){
  Serial.printf("Connecting to: %s\n", ssid);
  WiFi.begin(ssid, password);
  
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.printf("\nWiFi connected!\n");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void setupRTC() {
  rtc.begin();
  
  if (! rtc.isrunning())
    Serial.println("RTC is NOT running!");
}

void syncTimeWithNTP(){
  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 3600;
  const int   daylightOffset_sec = 3600;

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;

  if (getLocalTime(&timeinfo))
    rtc.adjust(DateTime((timeinfo.tm_year -100), timeinfo.tm_mon+1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
}

String getTimestamp(){
  DateTime time = rtc.now();
  return time.timestamp(DateTime::TIMESTAMP_FULL);
}

void setup() {
  Serial.begin(115200);

  setupWiFi();
  setupRTC();
  syncTimeWithNTP();
  Serial.print("\nRTC TIME: ");
  Serial.println(getTimestamp());

}

void loop() {
  // put your main code here, to run repeatedly:

}
