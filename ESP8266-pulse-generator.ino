/*
   Генератор импульсов заданной частоты

   Добавил файловый менеджер
   Добавил обновление через вебстраницу "Системные настройки"
   Автозапуск АР Default, если не найден файл настроек
*/

// кнопка, подключена к пину GPIO 4 (D2)
// голубой wifi светодиод GPIO 2
// зеленый светодиод GPIO 12
// синий светодиод GPIO 13
// красный светодиод GPIO 15

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <WebSocketsServer.h>
#include <FS.h>
#include <ArduinoJson.h>

#define LED_WIFI 2     // номер пина светодиода GPIO2 (D4)
#define LED_RED 15     // пин, красного светодиода 
#define LED_GREEN 12   // пин, зеленого светодиода 
#define LED_BLUE 13    // пин, синего светодиода
#define BUTTON 4       // номер пина кнопки GPIO4 (D2)
#define IMPULS_OUT 14          // пин, выход импульсов 

bool wifiAP_mode = 0;
char *p_ssidAP = "AP";             //SSID-имя вашей сети
char *p_passwordAP = "12345678";
char *p_ssid = "lamp";
char *p_password = "1234567890lamp";
byte ip[4] = {192, 168, 1, 43};
byte sbnt[4] = {255, 255, 255, 0};
byte gtw[4] = {192, 168, 1, 1};

bool sendSpeedDataEnable[] = {0, 0, 0, 0, 0};
String ping = "ping";
unsigned int speedT = 100;   //gпериод отправки данных, миллисек

unsigned int impulsTime = 5000;   //ширина импульса мкс
bool impulsOutEnable = 1;           //бит вкл./откл. генерации импульсов
bool timer1Enable = 0;

int timeT1 = millis();
int timeT2 = millis();

WebSocketsServer webSocket(81);
ESP8266WebServer server(80);

unsigned int t1 = micros();
unsigned int t2 = micros();
unsigned int time_msg1;
unsigned int time_msg2;

void ICACHE_RAM_ATTR bitTxIsr() {
  digitalWrite(IMPULS_OUT, !digitalRead(IMPULS_OUT));
  digitalWrite(LED_RED, !digitalRead(LED_RED));
}

void setup() {
  delay(2);
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Setup");
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(IMPULS_OUT, OUTPUT);
  digitalWrite(LED_WIFI, HIGH);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);
  //printChipInfo();

  SPIFFS.begin();
  //scanAllFile();
  //saveConfiguration();
  //printFile("/config.txt");

  //Запуск точки доступа с параметрами поумолчанию
  if ( !loadConfiguration() ||  digitalRead(BUTTON) == 0)  startAp("\nESP", "11111111");
  //Запуск точки доступа
  else if (digitalRead(BUTTON) == 1 && wifiAP_mode == 1)   startAp(p_ssidAP, p_passwordAP);
  //Запуск подключения клиента к точке доступа
  else if (digitalRead(BUTTON) == 1 && wifiAP_mode == 0) {
    if (WiFi.getPersistent() == true) WiFi.persistent(false);   //disable saving wifi config into SDK flash area
    WiFi.softAPdisconnect(true);
    WiFi.disconnect();
    set_staticIP();
    WiFi.begin(p_ssid, p_password);
    WiFi.persistent(true);                                      //enable saving wifi config into SDK flash area
    // if (WiFi.SSID() != p_ssid || WiFi.psk() != p_password) {
    //   Serial.println(F("\nCHANGE password or ssid"));
    //  WiFi.disconnect();
    //}
    //set_staticIP();
  }

  webServer_init();      //инициализация HTTP интерфейса
  webSocket_init();      //инициализация webSocket интерфейса

  timer1_isr_init();
  timer1_attachInterrupt(bitTxIsr);
}


void loop() {

    wifi_init();
    webSocket.loop();
    server.handleClient();


  if (impulsOutEnable == 1 && timer1Enable == 0 ) {
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
    timer1_write(impulsTime);
    timer1Enable = 1;
  } else if (impulsOutEnable == 0 && timer1Enable == 1 ) {
    timer1_disable();
    digitalWrite(LED_RED, 0);
    digitalWrite(IMPULS_OUT, 0);
    timer1Enable = 0;
  }


  /*
    //Отправка Speed данных клиентам каждые speedT миллисекунд, при условии что клиенты подключены
    if (sendSpeedDataEnable[0] || sendSpeedDataEnable[1] || sendSpeedDataEnable[2] || sendSpeedDataEnable[3] || sendSpeedDataEnable[4]) {
      if (millis() - timeT1 > speedT ) {
        //t1 = micros();
        StaticJsonDocument<128> docSpeed;
        //docSpeed["stRed"] = digitalRead(LED_RED);
        docSpeed["stBlue"] = digitalRead(LED_BLUE);
        docSpeed["stGreen"] = digitalRead(LED_GREEN);
        docSpeed["adc"] = analogRead(A0);
        String data = "";
        serializeJson(docSpeed, data);
        //time_msg1 = micros() - t1;

        int startT_broadcastTXT = micros();
        webSocket.broadcastTXT(data);
        int T_broadcastTXT = micros() - startT_broadcastTXT;
        if (T_broadcastTXT > 100000)  checkPing();

        timeT1 = millis();
      }
    }
  */
}


//Print configuration parameters
void printConfiguration () {
  Serial.println(F("Print ALL VARIABLE:"));
  Serial.print(F("wifiAP_mode="));  Serial.println(wifiAP_mode);

  Serial.print(F("p_ssidAP="));     Serial.println(p_ssidAP);
  Serial.print(F("p_passwordAP=")); Serial.println(p_passwordAP);
  Serial.print(F("p_ssid="));       Serial.println(p_ssid);
  Serial.print(F("p_password="));   Serial.println(p_password);

  Serial.print(F("ip="));    Serial.print(ip[0]);   Serial.print(":");  Serial.print(ip[1]);  Serial.print(":");  Serial.print(ip[2]);  Serial.print(":");  Serial.println(ip[3]);
  Serial.print(F("sbnt="));  Serial.print(sbnt[0]); Serial.print(":");  Serial.print(sbnt[1]);  Serial.print(":");  Serial.print(sbnt[2]);  Serial.print(":");  Serial.println(sbnt[3]);
  Serial.print(F("gtw="));   Serial.print(gtw[0]);  Serial.print(":");  Serial.print(gtw[1]);  Serial.print(":");  Serial.print(gtw[2]);  Serial.print(":");  Serial.println(gtw[3]);
}

void printChipInfo() {
  Serial.print(F("\n<-> LAST RESET REASON: "));  Serial.println(ESP.getResetReason());
  Serial.print(F("<-> ESP8266 CHIP ID: "));      Serial.println(ESP.getChipId());
  Serial.print(F("<-> CORE VERSION: "));         Serial.println(ESP.getCoreVersion());
  Serial.print(F("<-> SDK VERSION: "));          Serial.println(ESP.getSdkVersion());
  Serial.print(F("<-> CPU FREQ MHz: "));         Serial.println(ESP.getCpuFreqMHz());
  Serial.print(F("<-> FLASH CHIP ID: "));        Serial.println(ESP.getFlashChipId());
  Serial.print(F("<-> FLASH CHIP SIZE: "));      Serial.println(ESP.getFlashChipSize());
  Serial.print(F("<-> FLASH CHIP REAL SIZE: ")); Serial.println(ESP.getFlashChipRealSize());
  Serial.print(F("<-> FLASH CHIP SPEED: "));     Serial.println(ESP.getFlashChipSpeed());
  Serial.print(F("<-> FREE MEMORY: "));          Serial.println(ESP.getFreeHeap());
  Serial.print(F("<-> SKETCH SIZE: "));          Serial.println(ESP.getSketchSize());
  Serial.print(F("<-> FREE SKETCH SIZE: "));     Serial.println(ESP.getFreeSketchSpace());
  Serial.print(F("<-> CYCLE COUNTD: "));         Serial.println(ESP.getCycleCount());
}
