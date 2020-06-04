String processor(const String&);
void onRequest(AsyncWebServerRequest *);
void onBody(AsyncWebServerRequest *, uint8_t *, size_t , size_t , size_t );
void onUpload(AsyncWebServerRequest *, String , size_t , uint8_t *, size_t , bool );
void onWsEvent(AsyncWebSocket * , AsyncWebSocketClient * , AwsEventType , void * , uint8_t *, size_t );
void serverStart();
StaticJsonDocument<1000> infoJson();
