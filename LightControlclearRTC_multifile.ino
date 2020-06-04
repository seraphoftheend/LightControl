#include <Update.h>
#include "FS.h"
#include "SPIFFS.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <WiFiUdp.h>
#include <Time.h>
#include <TimeLib.h>
#include "AsyncJson.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <ds3231.h>

//-------------------------------//
#include "Types&variables.h"
#include "ParseTime.h"
#include "FileSystem.h"
#include "WeatherCode.h"
#include "ServerFunctions.h"
#include "NTPCode.h"
#include "RTCCode.h"

#define ledPin 15
#define ON  0
#define OFF 1
bool ledOn = OFF;      //текущее состояние нагрузки


void setup() {
  Serial.begin(115200);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  Wire.begin(21, 22);
  DS3231_init(DS3231_INTCN); //register the ds3231 (DS3231_INTCN is the default address of ds3231, this is set by macro for no performance loss)

  pinMode(ledPin, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    delay(500);
    // return;
  } else {
    Serial.println(WiFi.localIP());
  }

  if (!SPIFFS.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  listDir(SPIFFS, "/", 0);

  File time_data = SPIFFS.open("/time_data.txt", FILE_READ);
  //Serial.println("Read data from /time_data.txt:");
  str_sunriseFromFile = time_data.readStringUntil('\n');
  str_sunsetFromFile = time_data.readStringUntil('\n');
  ul_sunriseFromFile = str_sunriseFromFile.toInt();
  ul_sunsetFromFile = str_sunsetFromFile.toInt();
  weather.sunrise = ul_sunriseFromFile;
  weather.sunset = ul_sunsetFromFile;
  Serial.println(String("Setup Sunrise: ") + str_sunriseFromFile + " : " + ul_sunriseFromFile + " : " + weather.sunrise);
  Serial.println(String("Setup Sunset: ") + str_sunsetFromFile + " : " + ul_sunsetFromFile + " : " + weather.sunset);
  time_data.close();

  Udp.begin(localPort);
  Serial.println("waiting for sync");
  GetWeather();
  if (weather.data_update_time) {
    if (getRTCtime() < 946684800) {
      setRtcTime(weather.data_update_time);
    }
  }
  setSyncProvider(getRTCtime);
  setSyncInterval(100);
  serverStart();
}

void loop() {
  if (shouldReboot) {
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.disconnect();
    Serial.println("Reconnecting");
    WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {                  //!!!!!!!!!!!!!!!!!!!!!!!!11
      delay (1000);
    } else {
      LedStateOnOff();
      Serial.println("Reconnecting complete" + WiFi.localIP());
    }
  }

  static char temp[128];
  if (millis() < lastConnectionTime) {
    lastConnectionTime = 0;
  }
  if (WiFi.status() == WL_CONNECTED) {
    if (lastConnectionTime == 0) {
      GetWeather();
      Serial.println(lastConnectionTime);
      setTime(getRTCtime());
    } else {
      if ( millis() - lastConnectionTime > postingInterval) {
        GetWeather();
      }
    }
  }

  if (now() / 30 != prevDisplay / 30) { //update the display only if time has changed
    prevDisplay = now();
    Serial.println(String("Current time: ") + day() + "." + month() + "." + year() + " " + hour() + ":" + minute() + ":" + second());
    Serial.println(String("switchTime: ") + day(switchTime) + "." + month(switchTime) + "." + year(switchTime) + " " + hour(switchTime) + ":" + minute(switchTime) + ":" + second(switchTime));
    DS3231_get(&time_struct);
    Serial.println(get_unixtime(time_struct));
    Serial.println(ledOn);
    Serial.println(String("NTPsyncInterval:: ") + NTPsyncInterval);
    Serial.println(WiFi.localIP());
  }

  curtime = getRTCtime();
  if (curtime - lastNTPsync > NTPsyncInterval) {
    Serial.println(String("lastNTPsync: ") + lastNTPsync);
    Serial.println(String("curtime (RTC): ") + curtime);
    Serial.println(String("NTPsyncInterval: ") + NTPsyncInterval);

    RTC_NTP_Sync();
    curtime = getRTCtime();
  }
  if (curtime > switchTime) {
    LedStateOnOff();
    digitalWrite (ledPin, ledOn);
  }

  static bool lastLedOn = 0;
  if (ledOn != lastLedOn) {
    if (ledOn == ON) {
      ws.textAll("ON");
    } else if (ledOn == OFF) {
      ws.textAll("OFF");
    }
    lastLedOn = ledOn;
  }
  static time_t lastSwitchTime = 0;
  if (switchTime != lastSwitchTime) {
    ws.textAll(String("Switch time: ") + day(switchTime) + "." + month(switchTime) + "." + year(switchTime) + " " + hour(switchTime) + ":" + minute(switchTime) + ":" + second(switchTime));
    lastSwitchTime = switchTime;
  }
  static time_t lastCurtime = 0;
  if (curtime != lastCurtime) {
    ws.textAll(String("Current time: ") + day(curtime) + "." + month(curtime) + "." + year(curtime) + " " + hour(curtime) + ":" + minute(curtime) + ":" + second(curtime));
    lastCurtime = curtime;
  }
}

/*----------Light state controlling code----------*/

void LedStateOnOff() {
  if (curtime > weather.sunset) {
    if (weather.sunrise <= weather.sunset) {
      if ((curtime - 300) / 86400 > (weather.sunset / 86400)) {
        if (WiFi.status() == WL_CONNECTED and GetWeather() and (weather.data_update_time > curtime - curtime % 86400)) {
          ledOn = ON;
          switchTime = weather.sunrise;
        } else {
          ledOn = ON;
          weather.sunrise += 86400;
          weather.sunset += 86400;
          switchTime = weather.sunrise;
        }
      } else {
        ledOn = ON;
        switchTime = (weather.sunrise / 86400 + 1) * 86400 + 300;
      }
    }
  } else {
    if (curtime > weather.sunrise) {
      ledOn = OFF;
      switchTime = weather.sunset;
    } else {
      ledOn = ON;
      switchTime = weather.sunrise;
    }
  }
}
