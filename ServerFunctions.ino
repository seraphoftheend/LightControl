#include "ServerFunctions.h"

/*----------Code for web server----------*/

String processor(const String& var){
  Serial.println(var);
    if (var == "IP"){
        Serial.println(WiFi.localIP());
        return WiFi.localIP().toString();
     } else
    if (var == "STATE"){
        if(digitalRead(ledPin)){
          ledState = "ON";
        }
        else{
          ledState = "OFF";
        }
        Serial.println(ledState);
        return ledState;
    } else
    if (var == "City"){
        Serial.println(String(weather.city));
        return String(weather.city);
     } else
    if (var == "main"){
        Serial.println(String(weather.main));
        return String(weather.main);
      } else
    if (var == "description"){
        Serial.println(String(weather.descript));
        return String(weather.descript);
      } else
    if (var == "temp"){
        Serial.println(String((int)round(weather.temp)) + " celsius");
        return String((int)round(weather.temp)) + " celsius";
      } else
    if (var == "humidity"){
        Serial.println(String((int)round(weather.humidity))+" percent");
        return String((int)round(weather.humidity))+" percent";
      } else
    if (var == "pressure"){
        Serial.println(String((int)round(weather.pressure)) + " hPa or " + (int)round(weather.pressure * 0.75) + " mmHg");
        return String((int)round(weather.pressure)) + " hPa or " + (int)round(weather.pressure * 0.75) + " mmHg";
      } else
    if (var == "speed"){
        Serial.println(String((int)round(weather.speed)) + " m/s");
        return String((int)round(weather.speed)) + " m/s";
      } else
    if (var == "direction"){
        Serial.println(String((int)round(weather.deg)));
        return String((int)round(weather.deg));
      } else
    if (var == "Data_up_time"){
        Serial.println(String(day(weather.data_update_time)) + "." + month(weather.data_update_time) + "." + year(weather.data_update_time) + " " + hour(weather.data_update_time) + ":" + minute(weather.data_update_time) + ":" + second(weather.data_update_time));
        return String(day(weather.data_update_time)) + "." + month(weather.data_update_time) + "." + year(weather.data_update_time) + " " + hour(weather.data_update_time) + ":" + minute(weather.data_update_time) + ":" + second(weather.data_update_time);
      } else
    if (var == "Sunrise"){
        Serial.println(String(day(weather.sunrise)) + "." + month(weather.sunrise) + "." + year(weather.sunrise) + " " + hour(weather.sunrise) + ":" + minute(weather.sunrise) + ":" + second(weather.sunrise));
        return String(day(weather.sunrise)) + "." + month(weather.sunrise) + "." + year(weather.sunrise) + " " + hour(weather.sunrise) + ":" + minute(weather.sunrise) + ":" + second(weather.sunrise);
      } else
    if (var == "Sunset"){
        Serial.println(String(day(weather.sunset)) + "." + month(weather.sunset) + "." + year(weather.sunset) + " " + hour(weather.sunset) + ":" + minute(weather.sunset) + ":" + second(weather.sunset));
        return String(day(weather.sunset)) + "." + month(weather.sunset) + "." + year(weather.sunset) + " " + hour(weather.sunset) + ":" + minute(weather.sunset) + ":" + second(weather.sunset);
      }; 
  return String();
}

void onRequest(AsyncWebServerRequest *request){
  //Handle Unknown Request
  request->send(404,"text/plain","Page not found");
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  //Handle body
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
  //Handle upload
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  //Handle WebSocket event
  if(type == WS_EVT_CONNECT){
    Serial.println("Websocket client connection received");
    client->text(String("Current time: ") + day(curtime) + "." + month(curtime) + "." +year(curtime) + " " + hour(curtime) + ":" + minute(curtime) + ":" + second(curtime));
  } else if(type == WS_EVT_DISCONNECT){
    Serial.println("Client disconnected");
  } 
}

void serverStart(){
   // attach AsyncWebSocket
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // attach AsyncEventSource
 // server.addHandler(&events);

  // respond to GET requests on URL /heap
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  // upload a file to /upload
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200);
  }, onUpload);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("/index");
  });
  
  // send a file when /index is requested
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS,"/index.htm", String(), false, processor);
  });

  // Route to load style.css file
  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/styles.css", "text/css");
  });

  // HTTP basic authentication
  server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password))
        return request->requestAuthentication();
    request->send(200, "text/plain", "Login Success!");
  });

  //POST params handler
  server.on("/index", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("submit hit.");
    int params = request->params();
    Serial.printf("%d params sent in\n", params);
    for (int i = 0; i < params; i++)
    {
        AsyncWebParameter *p = request->getParam(i);
        if (p->isFile())
        {
            Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
        }
        else if (p->isPost())
        {
            Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
        else
        {
            Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
    }
    if (request->hasParam("LedON", true))
    {
        String LedOn = request->getParam("LedON", true)->value();
        if(LedOn == "0"){
          ledOn = OFF;
        } else if (LedOn == "1"){
          ledOn = ON;
        }
        digitalWrite(ledPin, ledOn);
        Serial.println(String("Current led state is set to: ") + ledOn);

        AsyncResponseStream *response = request->beginResponseStream("application/json");
        serializeJson(infoJson(), *response);
        request->send(response);
    } else if (request->hasParam("SwitchTime", true)){
     String timeString = request->getParam("SwitchTime", true)->value();
          time_t tmpTime = ParseTime(timeString);
          if(tmpTime != 0){
            switchTime = tmpTime;
          }
      Serial.println(now());
      Serial.println(switchTime);
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      serializeJson(infoJson(), *response);
      request->send(response);
  //  request->send(204);
    } else if (request->hasParam("CurrentTime", true)){
     String timeString = request->getParam("CurrentTime", true)->value();
          time_t tmpTime = ParseTime(timeString);
          if(tmpTime != 0){
            setTime (tmpTime);
            setRtcTime(tmpTime);
          }
      Serial.println(now());
      Serial.println(switchTime);
   //   request->send(204);
    }
  });

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(infoJson(), *response);
    request->send(response);
  });

  // Simple Firmware Update Form
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  });
  
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      Serial.printf("Update Start: %s\n", filename.c_str());
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        Serial.printf("Update Success: %uB\n", index+len);
      } else {
        Update.printError(Serial);
      }
    }
  });

  // attach filesystem root at URL /fs
  server.serveStatic("/fs", SPIFFS, "/");

  // Catch-All Handlers
  // Any request that can not find a Handler that canHandle it
  // ends in the callbacks below.
  server.onNotFound(onRequest);
  //server.onFileUpload(onUpload);
  //server.onRequestBody(onBody);

  server.begin();
}

StaticJsonDocument<1000> infoJson(){
  StaticJsonDocument<1000> info;
  info=doc;
  if (ledOn == OFF){
          info ["ledon"] = "Off";
        } else if (ledOn == ON){
          info ["ledon"] = "On";
        }
  String swtime = String("Switch time: ") + day(switchTime) + "." + month(switchTime) + "." +year(switchTime) + " " + hour(switchTime) + ":" + minute(switchTime) + ":" + second(switchTime);
  info ["swtime"] = swtime.c_str();
 return info;
}
