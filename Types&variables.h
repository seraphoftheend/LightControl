ts time_struct; //RTC time struct

//const char* ssid = "*******";
const char* ssid = "*******";
//const char* password = "*******";
const char* password = "*******";
const char* http_username = "admin";
const char* http_password = "admin";

const char ntpServerName[] = "europe.pool.ntp.org";

String ledState;     //строка с текущим состо€нием нагрузки дл€ отображени€ в веб-интерфейсе

time_t curtime = now(), switchTime = 0;  //текущее врем€, врем€ следующего переключени€ нагрузки
time_t getRTCtime();

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
time_t prevDisplay = 0; // when the digital clock was displayed
unsigned long lastConnectionTime = 0;  //врем€ последнего запроса на сервер погоды погоды
unsigned long postingInterval = 0;  //интервал обновлени€ погоды
unsigned long NTPsyncInterval = 0;
unsigned long lastNTPsync = 0;

time_t timeShiftSunrise = 60;   //сдвиг включени€ света относительно восхода
time_t timeShiftSunset = -60;   //сдвиг включени€ света относительно заката

struct weather_structure {
  const char* main;
  const char* icon;
  const char* descript;
  const char* city;
  long  city_id;
  float temp;
  float pressure;
  byte  humidity;
  float speed;
  float deg;
  time_t data_update_time = 0;
  time_t sunrise;
  time_t sunset;
  time_t offset;
};
weather_structure weather;

String httpData;  //переменна€ дл€ хранени€ json-строки с погодой
StaticJsonDocument<1000> doc;

// «адайте свой статический IP-адрес
//IPAddress local_IP(***, ***, ***, ***);  
IPAddress local_IP(***, ***, ***, ***);   
// ”кажите IP-адрес шлюза
//IPAddress gateway(***, ***, ***, ***);  
IPAddress gateway(***, ***, ***, ***);  

IPAddress subnet(255, 255, 254, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // опционально
IPAddress secondaryDNS(8, 8, 4, 4); // опционально

AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
AsyncEventSource events("/events"); // event source (Server-Sent events)

//flag to use from web update to reboot the ESP
bool shouldReboot = false;

String str_sunriseFromFile = "";
String str_sunsetFromFile = "";
unsigned long ul_sunriseFromFile = 0; 
unsigned long ul_sunsetFromFile = 0;