#ifndef managetime_h
#define managetime_h

#include "RTClib.h"

void setupRTC(RTC_DS1307 *rtc);
void syncTimeWithNTP(RTC_DS1307 *rtc);
String getTimestamp(DateTime dateTime);

#endif