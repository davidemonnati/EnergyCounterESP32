#include "include/ManageTime.h"

void setupRTC(RTC_DS1307 *rtc) {
  if (! rtc->begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  rtc->adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (! rtc->isrunning())
    Serial.println("RTC is NOT running!");
}

void syncTimeWithNTP(RTC_DS1307 *rtc) {
  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 3600;
  const int   daylightOffset_sec = 3600;

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;

  if (getLocalTime(&timeinfo))
    rtc->adjust(DateTime((timeinfo.tm_year - 100), timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
}

String getTimestamp(DateTime dateTime) {
  return String(dateTime.year()) + "-" + String(dateTime.month()) + "-" + String(dateTime.day()) 
    + "T" + String(dateTime.hour()) + ":" + String(dateTime.minute()) + ":" +String(dateTime.second());;
}
