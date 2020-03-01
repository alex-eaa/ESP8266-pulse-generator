///////////////////////////////////////////////////////////
//////  WI-FI  WI-FI  WI-FI  //////////////////////////////
///////////////////////////////////////////////////////////

unsigned int wifiConnectTimer = millis();     //перемнная для таймера включения WIFI
unsigned int ledBlinkTimer = millis();        //перемнная для мигания LED WIFI
bool wlConnectedMsgSend = 0;
bool wifiAP_runned = 0;

void wifi_init()
{
  //Разовое сообщение при подключении к точке доступа WIFI
  if (WiFi.status() == WL_CONNECTED && wlConnectedMsgSend == 0) {
    Serial.println(F("\nCONNECTED to WiFi AP!"));
    Serial.print(F("Is Connected to AP: "));           Serial.println(WiFi.isConnected());
    Serial.print(F("My IP address: "));   Serial.println(WiFi.localIP());
    Serial.print(F("Subnet mask: "));     Serial.println(WiFi.subnetMask());
    Serial.print(F("Subnet gateway: "));  Serial.println(WiFi.gatewayIP());
    Serial.print(F("My macAddress: "));   Serial.println(WiFi.macAddress());
    Serial.print(F("My default hostname: "));   Serial.println(WiFi.hostname());
    Serial.print(F("Connected to AP with SSID: "));       Serial.println(WiFi.SSID());
    Serial.print(F("Connected to AP with password: "));   Serial.println(WiFi.psk());
    wlConnectedMsgSend = 1;
    digitalWrite(LED_WIFI, 0);
  }

  if (wifiAP_mode == 0 && wifiAP_runned == 0) {
    //startWifiClient();
    //Мигание LED WIFI при поиске точки доступа
    if (WiFi.status() != WL_CONNECTED && millis() - ledBlinkTimer > 250) {
      digitalWrite(LED_WIFI, !digitalRead(LED_WIFI));
      ledBlinkTimer = millis();
      wlConnectedMsgSend = 0;
    }
  } else if (wifiAP_runned == 1) {
    //Мигание LED WIFI при работе точки доступа
    if (wifiAP_runned == 1) {
      if (millis() - ledBlinkTimer > 750) {
        digitalWrite(LED_WIFI, !digitalRead(LED_WIFI));
        ledBlinkTimer = millis();
      }
    }
  }
}


//Функция запуска модуля в режиме AP
void startAp(char *ap_ssid, const char *ap_password)
{
  Serial.println(F("\n\nSTART ESP in AP WIFI mode"));
  
  if (WiFi.getPersistent() == true)    WiFi.persistent(false);   //disable saving wifi config into SDK flash area
  WiFi.disconnect();
  WiFi.softAP(ap_ssid, ap_password);
  WiFi.persistent(true);                                      //enable saving wifi config into SDK flash area

  wifiAP_runned = 1;
  Serial.print(F("SSID AP: "));      Serial.println(ap_ssid);
  Serial.print(F("Password AP: "));  Serial.println(ap_password);
  Serial.print(F("Start AP with SSID: "));       Serial.println(WiFi.softAPSSID());
  Serial.print(F("Soft-AP IP: "));   Serial.println(WiFi.softAPIP());
  Serial.print(F("Soft-AP MAC: "));  Serial.println(WiFi.softAPmacAddress());

}



//WiFi client control
void startWifiClient()
{
  //if (WiFi.getMode() != WIFI_STA) WiFi.mode(WIFI_STA);

  if (WiFi.status() == WL_IDLE_STATUS && millis() - wifiConnectTimer > 3000) {
    Serial.println(F("\n\nSTART WIFI module"));
    set_staticIP();
    WiFi.begin(p_ssid, p_password);
    Serial.println(F("ESP start connect to AP wifi."));
    Serial.print(F("To AP SSID: "));       Serial.println(WiFi.SSID());
    Serial.print(F("To AP password: "));   Serial.println(WiFi.psk());
    wifiConnectTimer = millis();
  }
  /*
    if (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_IDLE_STATUS && millis() - wifiConnectTimer > 10000) {
      Serial.println(F("\nFailed to connect to AP."));
      WiFi.disconnect();
      WifiStatus();
      Serial.println(F("STOP WIFI module."));
      wifiConnectTimer = millis();
    }
  */
}  //WiFi control - END


void set_staticIP()   //Настройка статических параметров сети. Не записывабтся во flash память.
{
  IPAddress ipAdr(ip[0], ip[1], ip[2], ip[3]);
  IPAddress gateway(gtw[0], gtw[1], gtw[2], gtw[3]);
  IPAddress subnet(sbnt[0], sbnt[1], sbnt[2], sbnt[3]);
  //Serial.println("");
  //Serial.print("Set IP address: ");  Serial.println(ipAdr);
  //Serial.print("Set Subnet: ");      Serial.println(subnet);
  //Serial.print("Set IP gateway: ");  Serial.println(gateway);
  WiFi.config(ipAdr, gateway, subnet);
}



void WifiStatus(void)
{
  //Monitoring Status WiFi module to serial
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F(": WiFi.status = WL_CONNECTED. ")); //3
    Serial.print(F("IP address: "));  Serial.println(WiFi.localIP());
  }
  else if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F(": WiFi.status = WL_NO_SHIELD ")); //255
  }
  else if (WiFi.status() == WL_IDLE_STATUS) {
    Serial.println(F(": WiFi.status = WL_IDLE_STATUS ")); //0
  }
  else if (WiFi.status() == WL_NO_SSID_AVAIL) {
    Serial.println(F(": WiFi.status = WL_NO_SSID_AVAIL ")); //1
  }
  else if (WiFi.status() == WL_SCAN_COMPLETED) {
    Serial.println(F(": WiFi.status = WL_SCAN_COMPLETED ")); //2
  }
  else if (WiFi.status() == WL_CONNECT_FAILED) {
    Serial.println(F(": WiFi.status = WL_CONNECT_FAILED ")); //4
  }
  else if (WiFi.status() == WL_CONNECTION_LOST) {
    Serial.println(F(": WiFi.status = WL_CONNECTION_LOST ")); //5
  }
  else if (WiFi.status() == WL_DISCONNECTED) {
    Serial.println(F(": WiFi.status = WL_DISCONNECTED ")); //6
  }
}


void wifiInfo(){
    Serial.print(F("\n\n"));
    WifiStatus();

    Serial.println(F("\n   INFORMATION ON AP"));
    Serial.print(F("Start AP with SSID: "));               Serial.println(WiFi.softAPSSID());
    Serial.print(F("Soft-AP IP: "));                       Serial.println(WiFi.softAPIP());
    Serial.printf("Stations connected to soft-AP = %d\n",  WiFi.softAPgetStationNum());
    Serial.printf("Soft-AP MAC address = %s\n",            WiFi.softAPmacAddress().c_str());

    Serial.println(F("\n   INFORMATION ON CLIENT"));
    Serial.print(F("Is Connected to AP: "));           Serial.println(WiFi.isConnected());
    Serial.print(F("To AP SSID: "));                   Serial.println(WiFi.SSID());
    Serial.print(F("To AP password: "));               Serial.println(WiFi.psk());
    Serial.print("Connected to AP, my IP address: ");  Serial.println(WiFi.localIP());
    Serial.print(F("To AP set AutoConnect: "));        Serial.println(WiFi.getAutoConnect());
    Serial.print("RSSI: dBm: ");                       Serial.println(WiFi.RSSI());
}
