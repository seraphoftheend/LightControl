#include "ParseTime.h"

/*----------Code for parsing time from string----------*/
time_t ParseTime(String timeStr){
  if (timeStr[10] == 'T' and timeStr[13] == ':'){
            int year = (timeStr[0] - '0') * 1000 + (timeStr[1] - '0') * 100 + (timeStr[2] - '0') * 10 + (timeStr[3] - '0');
            int month = (timeStr[5] - '0') * 10 + (timeStr[6] - '0');
            int day = (timeStr[8] - '0') * 10 + (timeStr[9] - '0');
            int hour = (timeStr[11] - '0') * 10 + (timeStr[12] - '0');
            int minute = (timeStr[14] - '0') * 10 + (timeStr[15] - '0');
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            timeinfo->tm_year = year - 1900;
            timeinfo->tm_mon = month - 1;
            timeinfo->tm_mday = day;
            timeinfo->tm_hour = hour;
            timeinfo->tm_min = minute;
            rawtime = mktime (timeinfo);
            rawtime /= 60;
            rawtime *= 60;
            return rawtime;
          } 
   return 0;
}
