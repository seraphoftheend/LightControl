#include "WeatherCode.h"

/*------------Weather code------------*/

const String weather_server  = "api.openweathermap.org";
const String lat   = "47.01";
const String lon   = "28.86";
const String appid = "96d53ec95416111a70be22a15a65409c";
const String url   = "http://" + weather_server + "/data/2.5/weather?lat=" + lat + "&lon=" + lon + "&units=metric&appid=" + appid;

//Функция вывода погоды в Serial port и запись в файл
bool GetWeather(){
   if (httpRequest() and parseData()) {
        Serial.println("\nWeather");
        Serial.println(String("City: ") + weather.city + "(id "+ weather.city_id + ") ");
        Serial.println(String("main: ") + weather.main);
        Serial.println(String("description: ") + weather.descript);
        Serial.println(String("icon: ") + weather.icon);
        Serial.println(String("temp: ") + (int)round(weather.temp) + " celsius");
        Serial.println(String("humidity: ") + (int)round(weather.humidity)+"%");
        Serial.println(String("pressure: ") + (int)round(weather.pressure) + " hPa or " + (int)round(weather.pressure * 0.75) + " mmHg");
        Serial.println(String("wind's speed: ") + (int)round(weather.speed));
        Serial.println(String("wind's direction: ") + (int)round(weather.deg));
        Serial.println(String("Data updated at: ") + day(weather.data_update_time) + "." + month(weather.data_update_time) + "." +year(weather.data_update_time) + " " + hour(weather.data_update_time) + ":" + minute(weather.data_update_time) + ":" + second(weather.data_update_time));
        Serial.println(String("Sunrise: ") + day(weather.sunrise) + "." + month(weather.sunrise) + "." +year(weather.sunrise) + " " + hour(weather.sunrise) + ":" + minute(weather.sunrise) + ":" + second(weather.sunrise));
        Serial.println(String("Sunset: ") + day(weather.sunset) + "." + month(weather.sunset) + "." +year(weather.sunset) + " " + hour(weather.sunset) + ":" + minute(weather.sunset) + ":" + second(weather.sunset));
        
        if (!writeFile(SPIFFS,"/weather_data.txt","Weather\n")){
           ESP.restart();
        }
        appendFile(SPIFFS,"/weather_data.txt",(String("City: ") + weather.city + "(id " + weather.city_id + ") \n").c_str());
        appendFile(SPIFFS,"/weather_data.txt",(String("main: ") + weather.main + " \n").c_str());
        appendFile(SPIFFS,"/weather_data.txt",(String("description: ") + weather.descript + "\n").c_str());
        appendFile(SPIFFS,"/weather_data.txt",(String("icon: ") + weather.icon + "\n").c_str());
        appendFile(SPIFFS,"/weather_data.txt",(String("temp: ") + (int)round(weather.temp) + " celsius\n").c_str());
        appendFile(SPIFFS,"/weather_data.txt",(String("humidity: ") + (int)round(weather.humidity)+"%\n").c_str());
        appendFile(SPIFFS,"/weather_data.txt",(String("pressure: ") + (int)round(weather.pressure) + " hPa or " + (int)round(weather.pressure * 0.75) + " mmHg\n").c_str());
        appendFile(SPIFFS,"/weather_data.txt",(String("wind's speed: ") + (int)round(weather.speed) + " m/s\n").c_str());
        appendFile(SPIFFS,"/weather_data.txt",(String("wind's direction: ") + (int)round(weather.deg) + "\n").c_str());
        appendFile(SPIFFS,"/weather_data.txt",(String("Data updated at: ") + day(weather.data_update_time) + "." + month(weather.data_update_time) + "." +year(weather.data_update_time) + " " + hour(weather.data_update_time) + ":" + minute(weather.data_update_time) + ":" + second(weather.data_update_time) + "\n").c_str());
        appendFile(SPIFFS,"/weather_data.txt",(String("Sunrise: ") + day(weather.sunrise) + "." + month(weather.sunrise) + "." +year(weather.sunrise) + " " + hour(weather.sunrise) + ":" + minute(weather.sunrise) + ":" + second(weather.sunrise) + "\n").c_str());
        appendFile(SPIFFS,"/weather_data.txt",(String("Sunset: ") + day(weather.sunset) + "." + month(weather.sunset) + "." +year(weather.sunset) + " " + hour(weather.sunset) + ":" + minute(weather.sunset) + ":" + second(weather.sunset) + "\n").c_str());

        if (!writeFile(SPIFFS,"/time_data.txt","")){
           ESP.restart();
        }
        appendFile(SPIFFS,"/time_data.txt",(String(weather.sunrise) + "\n").c_str());
        appendFile(SPIFFS,"/time_data.txt",(String(weather.sunset) + "\n").c_str());
        //readFile(SPIFFS,"/time_data.txt");
        File file = SPIFFS.open("/time_data.txt", FILE_READ);
        Serial.println("Read data from /time_data.txt:");
        String s = file.readStringUntil('\n');
        String ss = file.readStringUntil('\n');
        unsigned long l = s.toInt();
        unsigned long ll = ss.toInt();
        Serial.println(String("Sunrise: ") + s + " : " + l);
        Serial.println(String("Sunset: ") + ss + " : " + ll);
        file.close();
        
        if (!readFile(SPIFFS,"/weather_data.txt")){
           ESP.restart();
        }
        Serial.println();
        return 1;
      }
      else return 0;
}

//Функция запроса на сервер погоды OpenWeatherMap
bool httpRequest() {
  
  HTTPClient client;
  bool find = false;
  
  Serial.print("Connecting ");
  client.begin(url);
  client.setTimeout(2000);
  int httpCode = client.GET();

  if (httpCode > 0) {
    Serial.printf("successfully, code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      httpData = client.getString();
      if (httpData.indexOf(F("\"main\":{\"temp\":")) > -1) {
        lastConnectionTime = millis();
        find = true;
      }
      else Serial.println("Failed, json string is not found");
    }
  }
  else Serial.printf("failed, error: %s\n", client.errorToString(httpCode).c_str());

  postingInterval = find ? 600L * 1000L : 60L * 1000L;
  client.end();

  return find;
}

bool parseData() {
  Serial.println(httpData);
  
  DeserializationError error = deserializeJson(doc, httpData);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }
  
  weather.main     = doc["weather"][0]["main"];
  weather.descript = doc["weather"][0]["description"];
  weather.icon     = doc["weather"][0]["icon"];
  weather.city     = doc["name"];
  weather.city_id  = doc["id"];
  weather.temp     = doc["main"]["temp"];
  weather.humidity = doc["main"]["humidity"];
  weather.pressure = doc["main"]["pressure"];
  weather.speed    = doc["wind"]["speed"];
  weather.deg      = doc["wind"]["deg"];
  weather.data_update_time = doc["dt"];
  weather.sunrise  = doc["sys"]["sunrise"];
  weather.sunset   = doc["sys"]["sunset"];
  weather.offset   = doc["timezone"];
  weather.data_update_time += weather.offset;
  weather.sunrise += weather.offset;
  weather.sunset += weather.offset;
  
  httpData = "";
  return true;
}
