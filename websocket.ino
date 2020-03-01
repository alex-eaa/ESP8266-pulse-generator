bool flag;

// функция иннициализации сервера WebSocket
void webSocket_init()
{
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}


//функция обработки входящих на сервер WebSocket сообщений
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  switch (type) {

    case WStype_DISCONNECTED:
      sendSpeedDataEnable[num] = 0;
      Serial.printf("[%u] Disconnected!\n", num);
      //Serial.printf("WStype_DISCONNECTED sendSpeedDataEnable [%u][%u][%u][%u][%u]\n", sendSpeedDataEnable[0], sendSpeedDataEnable[1], sendSpeedDataEnable[2], sendSpeedDataEnable[3], sendSpeedDataEnable[4]);
      break;

    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        if (strcmp((char *)payload, "/index.htm") == 0) {
          sendSpeedDataEnable[num] = 1;               //вкл. обновления быстрых данных
          String data = serializationToJson_index();
          //Serial.println(data);
          webSocket.sendTXT(num, data);
        } else if (strcmp((char *)payload, "/setup.htm") == 0) {
          sendSpeedDataEnable[num] = 0;                //откл. обновления быстрых данных
          String data = serializationToJson_setup();
          Serial.println(data);
          webSocket.sendTXT(num, data);
        }
        //Serial.printf("WStype_CONNECTED sendSpeedDataEnable [%u][%u][%u][%u][%u]\n", sendSpeedDataEnable[0], sendSpeedDataEnable[1], sendSpeedDataEnable[2], sendSpeedDataEnable[3], sendSpeedDataEnable[4]);
        break;
      }

    case WStype_TEXT:
      {
        Serial.printf("[%u] get Text: %s\n", num, payload);

        if (strcmp((char *)payload, "RESET") == 0)  ESP.reset();
        else if (strcmp((char *)payload, "start") == 0)  {
          impulsOutEnable = 1;
          //digitalWrite(LED_RED, 1);
          //timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
          //timer1_write(impulsTime);
        }
        else if (strcmp((char *)payload, "stop") == 0)   {
          impulsOutEnable = 0;
          digitalWrite(LED_RED, 0);
          //timer1_disable();
          //digitalWrite(LED_RED, 0);
          //digitalWrite(IMPULS_OUT, 0);
        }
        else if (strcmp((char *)payload, "plus") == 0)   {
          timer1_write(impulsTime);
        }
        else if (strcmp((char *)payload, "minus") == 0)   {
          timer1_write(impulsTime);
        }
        else {
          DynamicJsonDocument doc(1024);
          DeserializationError error = deserializeJson(doc, payload);
          if (error) {
            Serial.println(F("Failed to deserialization data from client"));
          }

          if (doc["page"].as<String>() == "index") {
            impulsTime = doc["impulsTime"];                    //Serial.println(var1);
            impulsOutEnable = doc["impulsOutEnable"];          //Serial.println(var2);
          }
          else if (doc["page"].as<String>() == "setup") {
            String stemp = doc["p_ssid"].as<String>();
            p_ssid = new char [stemp.length() + 1];
            stemp.toCharArray(p_ssid, stemp.length() + 1);
            //Serial.print(F("p_ssid="));   Serial.println(p_ssid);

            stemp = doc["p_password"].as<String>();
            p_password = new char [stemp.length() + 1];
            stemp.toCharArray(p_password, stemp.length() + 1);
            //Serial.print(F("p_password="));   Serial.println(p_password);

            stemp = doc["p_ssidAP"].as<String>();
            p_ssidAP = new char [stemp.length() + 1];
            stemp.toCharArray(p_ssidAP, stemp.length() + 1);
            //Serial.print(F("p_ssidAP="));   Serial.println(p_ssidAP);

            stemp = doc["p_passwordAP"].as<String>();
            p_passwordAP = new char [stemp.length() + 1];
            stemp.toCharArray(p_passwordAP, stemp.length() + 1);
            //Serial.print(F("p_passwordAP="));   Serial.println(p_passwordAP);

            //strcpy(p_ssid, doc["p_ssid"]);              //Serial.println(p_ssid);
            //strcpy(p_password, doc["p_password"]);      //Serial.println(p_password);
            //strcpy(p_ssidAP, doc["p_ssidAP"]);          //Serial.println(p_ssidAP);
            //strcpy(p_passwordAP, doc["p_passwordAP"]);  //Serial.println(p_passwordAP);
            ip[0] = doc["ip"][0];    //Serial.println(ip[0]);
            ip[1] = doc["ip"][1];    //Serial.println(ip[1]);
            ip[2] = doc["ip"][2];    //Serial.println(ip[2]);
            ip[3] = doc["ip"][3];    //Serial.println(ip[3]);
            sbnt[0] = doc["sbnt"][0];    //Serial.println(sbnt[0]);
            sbnt[1] = doc["sbnt"][1];    //Serial.println(sbnt[1]);
            sbnt[2] = doc["sbnt"][2];    //Serial.println(sbnt[2]);
            sbnt[3] = doc["sbnt"][3];    //Serial.println(sbnt[3]);
            gtw[0] = doc["gtw"][0];    //Serial.println(gtw[0]);
            gtw[1] = doc["gtw"][1];    //Serial.println(gtw[1]);
            gtw[2] = doc["gtw"][2];    //Serial.println(gtw[2]);
            gtw[3] = doc["gtw"][3];    //Serial.println(gtw[3]);
            wifiAP_mode = doc["wifiAP_mode"];  //Serial.println(wifiAP_mode);
            saveConfiguration();
          }
        }

        // send message to client
        // webSocket.sendTXT(num, "message here");
        // send data to all connected clients
        // webSocket.broadcastTXT("message here");
        break;
      }

    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\n", num, length);
      hexdump(payload, length);
      // send message to client
      // webSocket.sendBIN(num, payload, length);
      break;

  }

}


String serializationToJson_index()
{
  DynamicJsonDocument doc(1024);
  doc["impulsTime"] = impulsTime;
  doc["impulsOutEnable"] = impulsOutEnable;
  String output = "";
  serializeJson(doc, output);
  return output;
}


String serializationToJson_setup()
{
  DynamicJsonDocument doc(1024);
  doc["p_ssid"] = p_ssid;
  doc["p_password"] = p_password;
  doc["p_ssidAP"] = p_ssidAP;
  doc["p_passwordAP"] = p_passwordAP;
  JsonArray ipJsonArray = doc.createNestedArray("ip");
  for (int n = 0; n < 4; n++)  ipJsonArray.add(ip[n]);
  JsonArray sbntJsonArray = doc.createNestedArray("sbnt");
  for (int n = 0; n < 4; n++)  sbntJsonArray.add(sbnt[n]);
  JsonArray gtwJsonArray = doc.createNestedArray("gtw");
  for (int n = 0; n < 4; n++)  gtwJsonArray.add(gtw[n]);
  doc["wifiAP_mode"] = wifiAP_mode;

  String output = "";
  serializeJson(doc, output);
  return output;
}


String serializator()
{
  DynamicJsonDocument doc(1024);
}


// Проверка состояния соединения с websocket-клиентами. Отключение тех с которыми нет связи.
void checkPing() {
  //Serial.println("checkPing checkPing checkPing checkPing checkPing");
  for (uint8_t nums = 0; nums < 5; nums++) {
    //int timeStart = micros();
    if ( !webSocket.sendPing(nums, ping) )  webSocket.disconnect(nums);
    //int timeTotal = micros() - timeStart;
    //Serial.printf("TIME ping [%u]: %u\n", nums, timeTotal);
  }
}


void deSerializationFromJson(uint8_t payload)
{
  //Serial.println(payload);
  //Serial.println(*payload);
  //Serial.println(&payload);
  //StaticJsonDocument<1024> doc;
  //DeserializationError error = deserializeJson(doc, payload);
  //if (error)
  //Serial.println(F("Failed to deserialization data from client"));
}

/*
  if (type == WStype_TEXT) {
    String data;

    for (int x = 0; x < length; x++) {
      //if (!isdigit(payload[x])) continue;
      //data += (char) payload[x];
      data += (byte) payload[x];

    }
    Serial.println(data);

    if (payload[0] == 'B') {
      flag = 0;
      Serial.print("Bright: ");
      //bright = data.toInt();
      Serial.println(data);
      //LEDS.setBrightness(bright);

    }
    else if (payload[0] == 'F') {
      flag = 0;
      Serial.print("Function: ");
      //ledMode = data.toInt();
      Serial.println(data);
      //ledEffect(ledMode);

    }
    else if (payload[0] == '#') {

      if (!flag) {
        Serial.print("flag : ");
        Serial.println(flag);
        //ledMode = flag;
        //ledEffect(ledMode);
        flag = 1;

      }
      else {
        //преобразуем в 24 битное цветовое число
        uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);

        //преобразуем 24 бит по 8 бит на канал
        uint8_t r = abs(0 + (rgb >> 16) & 0xFF);
        uint8_t g = abs(0 + (rgb >>  8) & 0xFF);
        uint8_t b = abs(0 + (rgb >>  0) & 0xFF);

        Serial.print("ColorPicker: ");
        Serial.print(r);
        Serial.print(g);
        Serial.println(b);


      }
    }
  }
*/
