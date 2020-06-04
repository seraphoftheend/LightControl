#include "RTCCode.h"

/*---------- RTC Functions ----------*/

void setRtcTime(time_t rawtime){
  //time_t rawtime = now();
  struct tm * timeinfo;
  //time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  time_struct.sec = (uint8_t)timeinfo->tm_sec;    //!!!!!!!! Проверить, куда добавить единицы
  time_struct.min = (uint8_t)timeinfo->tm_min;
  time_struct.hour = (uint8_t)timeinfo->tm_hour;
  time_struct.mday = (uint8_t)timeinfo->tm_mday;
  time_struct.mon = (uint8_t)timeinfo->tm_mon + 1;
  time_struct.year = (uint16_t)timeinfo->tm_year + 1900;
  time_struct.wday = (uint8_t)timeinfo->tm_wday;
  time_struct.yday = (uint16_t)timeinfo->tm_yday ;  //исправил в библиотеке с uint8_t на uint16_t
  time_struct.isdst = (uint8_t)timeinfo->tm_isdst;
  //time_struct.unixtime = get_unixtime(time_struct);
  Serial.println(String(" ") + time_struct.hour + ":" + time_struct.min + ":"  + time_struct.sec + " "  + time_struct.mday + " " + time_struct.mon + " " + time_struct.year + "::" + time_struct.yday);
  DS3231_set(time_struct);
  DS3231_get(&time_struct);
  Serial.println(String("Set RTC time: ") + time_struct.hour + ":" + time_struct.min + ":"  + time_struct.sec + " "  + time_struct.mday + " " + time_struct.mon + " " + time_struct.year + "::" + time_struct.yday);
  Serial.println(String("RTC Unixtime from time_struct: ") + time_struct.unixtime);
  Serial.println(String("RTC Unixtime get_unixtime: ") + get_unixtime(time_struct));
}

time_t getRTCtime(){
  /*time_struct.sec = 0;
  time_struct.min = 0;
  time_struct.hour = 0;
  time_struct.mday = 0;
  time_struct.mon = 0;
  time_struct.year = 1900;
  time_struct.wday = 0;
  time_struct.yday = 0 ;  //исправил в библиотеке с uint8_t на uint16_t
  time_struct.isdst = 0;*/
  DS3231_get(&time_struct);
  time_struct.yday = calculate_yday(time_struct);
  //  Serial.println(String("Get RTC time: ") + time_struct.hour + ":" + time_struct.min + ":"  + time_struct.sec + " "  + time_struct.mday + " " + time_struct.mon + " " + time_struct.year);
  //  Serial.println(time_struct.yday);
  //  Serial.println(String("RTC Unixtime: ") + get_unixtime(time_struct));
  return get_unixtime(time_struct);
}

void RTC_NTP_Sync(){
  static uint32_t prevTmp = 0;
  time_t t = 0;
  t = getNtpTime();
  if (t){
    uint32_t tmp = abs(t - getRTCtime());
    if (prevTmp){
      if (prevTmp - tmp <= 1){
        setTime(t);
        setRtcTime(t);
        prevTmp = 0;
        lastNTPsync = t;
        NTPsyncInterval = 86400;
        Serial.println("RTC NTP time sync completed");
        Serial.println(String("NTPsyncInterval set to: ") + NTPsyncInterval);
      }
    }
    if (tmp < 30){                //когда будет батарейка, уменьшить
      if(tmp > 1){
        setTime(t);
        setRtcTime(t);
        lastNTPsync = t;
        NTPsyncInterval = 86400;
        Serial.println("RTC NTP time sync completed");
        Serial.println(String("NTPsyncInterval set to: ") + NTPsyncInterval);
      }
      else{
        lastNTPsync = t;
        NTPsyncInterval = 86400;
        Serial.println("Time sync not need");
        Serial.println(String("NTPsyncInterval set to: ") + NTPsyncInterval);
      }
      prevTmp = 0;
    } else {
      prevTmp = tmp;
      NTPsyncInterval = 15;
    }
  } else{
    NTPsyncInterval = 15;
  }
}

uint16_t calculate_yday(ts timeStruct){
  uint16_t yday = 0;
  uint8_t days_in_feb = 0;
  if( (timeStruct.year % 4 == 0 && timeStruct.year % 100 != 0 ) || (timeStruct.year % 400 == 0) )
    {
        days_in_feb = 29;
    }
  switch(timeStruct.mon){
    case 1:
      yday = timeStruct.mday - 1;
      break;
    case 2:
      yday = 31 + timeStruct.mday - 1;
      break;
    case 3:
      yday = 31 + days_in_feb + timeStruct.mday - 1;
      break;
    case 4:
      yday = 62 + days_in_feb + timeStruct.mday - 1;   //31+31
      break;
    case 5:
      yday = 92 + days_in_feb + timeStruct.mday - 1;   //31+31+30
      break;
    case 6:
      yday = 123 + days_in_feb + timeStruct.mday - 1;   //31+31+30+31
      break;
    case 7:
      yday = 153 + days_in_feb + timeStruct.mday - 1;   //31+31+30+31+30
      break;
    case 8:
      yday = 184 + days_in_feb + timeStruct.mday - 1;   //31+31+30+31+30+31
      break;
    case 9:
      yday = 215 + days_in_feb + timeStruct.mday - 1;   //31+31+30+31+30+31+31
      break;
    case 10:
      yday = 245 + days_in_feb + timeStruct.mday - 1;   //31+31+30+31+30+31+31+30
      break;
    case 11:
      yday = 276 + days_in_feb + timeStruct.mday - 1;   //31+31+30+31+30+31+31+30+31
      break;
    case 12:
      yday = 306 + days_in_feb + timeStruct.mday - 1;   //31+31+30+31+30+31+31+30+31+30
      break;
  }
  return yday;
}
