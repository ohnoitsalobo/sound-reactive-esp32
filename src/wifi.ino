//// Most of the AsyncServer code was copied with minimal modification from one of the examples,
  // so most of the time I have no idea what's going on here.

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
    if(type == WS_EVT_CONNECT){
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("Hello Client %u :)", client->id());
        client->ping();
    } else if(type == WS_EVT_DISCONNECT){
        Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    } else if(type == WS_EVT_ERROR){
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    } else if(type == WS_EVT_PONG){
        Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
    } else if(type == WS_EVT_DATA){
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        WSdata = "";
        if(info->final && info->index == 0 && info->len == len){
            //the whole message is in a single frame and we got all of it's data
            Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

            if(info->opcode == WS_TEXT){
                for(size_t i=0; i < info->len; i++) {
                    WSdata += (char) data[i];
                }
            } else {
                char buff[3];
                for(size_t i=0; i < info->len; i++) {
                    sprintf(buff, "%02x ", (uint8_t) data[i]);
                    WSdata += buff ;
                }
            }
            Serial.printf("%s\n",WSdata.c_str());

            if(info->opcode == WS_TEXT){
                client->text("I got your text message");
                handleSliders();
            }
            else
                client->binary("I got your binary message");
            
        } else {
            //message is comprised of multiple frames or the frame is split into multiple packets
            if(info->index == 0){
                if(info->num == 0)
                    Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
                    Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
            }

            Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

            if(info->opcode == WS_TEXT){
                for(size_t i=0; i < len; i++) {
                    WSdata += (char) data[i];
                }
            } else {
                char buff[3];
                for(size_t i=0; i < len; i++) {
                    sprintf(buff, "%02x ", (uint8_t) data[i]);
                    WSdata += buff ;
                }
            }
            Serial.printf("%s\n",WSdata.c_str());

            if((info->index + len) == info->len){
                Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
                if(info->final){
                    Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
                if(info->message_opcode == WS_TEXT)
                    client->text("I got your text message");
                else
                    client->binary("I got your binary message");
                }
            }
        }
    }
}



void wifiSetup(){
#ifdef debug
    Serial.println("\tStarting wifiSetup");
#endif
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    setupOTA();

    MDNS.addService("http","tcp",80);

    SPIFFS.begin();

    setupServer();
    
    timeSetup();
#ifdef debug
    Serial.println("\tEnding wifiSetup");
#endif
}

void wifiLoop(){
    if(WiFi.status() == WL_CONNECTED){
        ArduinoOTA.handle();
        EVERY_N_MILLISECONDS(500){
            ws.cleanupClients();
        }
        EVERY_N_MILLISECONDS(2000){
            Serial.println(WiFi.localIP());
        }
        if(!digitalRead(2)){
            digitalWrite(2, HIGH);
            Serial.println("Wifi connected!");
        }
    }
    
    if(WiFi.status() != WL_CONNECTED){
        EVERY_N_SECONDS(1){
            Serial.println("Wifi disconnected");
            WiFi.begin(ssid, password);
        }
        if(digitalRead(2))
            digitalWrite(2, LOW);
    }
    yield();
}

void setupOTA(){
#ifdef debug
    Serial.println("\t\tStarting setupOTA");
#endif
    ArduinoOTA.setPort(3232);

    ArduinoOTA.setHostname(hostName);
    
    ArduinoOTA
        .onStart([]() {
            digitalWrite(2, HIGH);
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else{ // U_SPIFFS
                type = "filesystem";
                SPIFFS.end();
            }
            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            events.send("Update Start", "ota");
            Serial.println("\r\nStart updating " + type);
            
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        })
        .onEnd([]() {
            digitalWrite(2, LOW);
            Serial.println("\r\nEnd");
            events.send("Update End", "ota");
            delay(10);
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            digitalWrite(2, !digitalRead(2));
            char p[32];
            sprintf(p, "Progress: %u%%\n", (progress/(total/100)));
            events.send(p, "ota");
            uint32_t temp = progress / (total / 100);
            Serial.printf("Progress: %u%%\r", temp);
            if(temp<99){
                int t = map(temp, 0, 99, 0, NUM_LEDS/4);
                fill_solid( RIGHT(           0, NUM_LEDS/4), t, 0x020202);
                fill_solid( RIGHT(NUM_LEDS/2-t, NUM_LEDS/2), t, 0x020202);
                fill_solid( LEFT (           0, NUM_LEDS/4), t, 0x020202);
                fill_solid( LEFT (NUM_LEDS/2-t, NUM_LEDS/2), t, 0x020202);
            }else if(temp >= 99){
                fill_solid (leds, NUM_LEDS, 0x020202);
            }
            FastLED.show();
        })
        .onError([](ota_error_t error) {
            fill_solid (leds, NUM_LEDS, CRGB::Red);
            FastLED.show();
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)        { Serial.println("Auth Failed");    events.send("Auth Failed", "ota");    }
            else if (error == OTA_BEGIN_ERROR)  { Serial.println("Begin Failed");   events.send("Begin Failed", "ota");   }
            else if (error == OTA_CONNECT_ERROR){ Serial.println("Connect Failed"); events.send("Connect Failed", "ota"); }
            else if (error == OTA_RECEIVE_ERROR){ Serial.println("Receive Failed"); events.send("Recieve Failed", "ota"); }
            else if (error == OTA_END_ERROR)    { Serial.println("End Failed");     events.send("End Failed", "ota");     }
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        });
    ArduinoOTA.begin();
#ifdef debug
    Serial.println("\t\tEnding setupOTA");
#endif
}

void setupServer(){
#ifdef debug
    Serial.println("\t\tStarting setupServer");
#endif
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    events.onConnect([](AsyncEventSourceClient *client){
        client->send("hello!",NULL,millis(),1000);
    });
    server.addHandler(&events);

    server.addHandler(new SPIFFSEditor(SPIFFS, http_username,http_password));

    server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });

    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    server.onNotFound([](AsyncWebServerRequest *request){
            Serial.printf("NOT_FOUND: ");
        if(request->method() == HTTP_GET)
            Serial.printf("GET");
        else if(request->method() == HTTP_POST)
            Serial.printf("POST");
        else if(request->method() == HTTP_DELETE)
            Serial.printf("DELETE");
        else if(request->method() == HTTP_PUT)
            Serial.printf("PUT");
        else if(request->method() == HTTP_PATCH)
            Serial.printf("PATCH");
        else if(request->method() == HTTP_HEAD)
            Serial.printf("HEAD");
        else if(request->method() == HTTP_OPTIONS)
            Serial.printf("OPTIONS");
        else
            Serial.printf("UNKNOWN");
            Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

        if(request->contentLength()){
            Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
            Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
        }

        int headers = request->headers();
        int i;
        for(i=0;i<headers;i++){
            AsyncWebHeader* h = request->getHeader(i);
            Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
        }

        int params = request->params();
        for(i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isFile()){
                Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
            } else if(p->isPost()){
                Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            } else {
                Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
        }

        request->send(404);
    });
        server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
        if(!index)
        Serial.printf("UploadStart: %s\n", filename.c_str());
        Serial.printf("%s", (const char*)data);
        if(final)
        Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
    });
    server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        if(!index)
        Serial.printf("BodyStart: %u\n", total);
        Serial.printf("%s", (const char*)data);
        if(index + len == total)
        Serial.printf("BodyEnd: %u\n", total);
    });
    server.begin();
#ifdef debug
    Serial.println("\t\tEnding setupServer");
#endif
}

void handleSliders(){
    if(WSdata.startsWith("reset")){
        WiFi.disconnect();
        digitalWrite(2, LOW);
        ESP.restart();
    }
    if(WSdata.startsWith("next")){
        nextPattern();
    }
    if(WSdata.startsWith("prev")){
        previousPattern();
    }
    String temp = WSdata.substring(1, WSdata.length()-1);
    if(WSdata.startsWith("M")){
        music = temp.endsWith("0") ? true : false;
        _auto = temp.endsWith("1") ? true : false;
        manual = temp.endsWith("2") ? true : false;
        gCurrentPatternNumber = 0;
        if(_auto)
            FastLED.setBrightness(30);
        else
            _setBrightness = 255;
    }if(WSdata.startsWith("V")){
        int x = temp.toInt();
        x = (x*x)/255.0;
        _setBrightness = x;
    }
    if(manual){
         if(WSdata.startsWith("R")){
            int x = temp.toInt();
            x = (x*x)/255.0;
            manualColor.r = x;
        }else if(WSdata.startsWith("G")){
            int x = temp.toInt();
            x = (x*x)/255.0;
            manualColor.g = x;
        }else if(WSdata.startsWith("B")){
            int x = temp.toInt();
            x = (x*x)/255.0;
            manualColor.b = x;
        }else if(WSdata.startsWith("H")){
            gHue1 = temp.toInt();
            manualHSV.h = gHue1;
            manualColor = manualHSV;
        }else if(WSdata.startsWith("S")){
            manualHSV.s = temp.toInt();
            manualColor = manualHSV;
        }
        if(WSdata.endsWith("L")){
            manualColor_L = manualColor;
        }else if(WSdata.endsWith("R")){
            manualColor_R = manualColor;
        }else if(WSdata.endsWith("B")){
            manualColor_L = manualColor;
            manualColor_R = manualColor;
        }
    }
}

//////// TIME //////////

void timeSetup(){
    Udp.begin(localPort);
}

void timeLoop(){
	if(timeStatus() != timeSet){
        setSyncProvider(getNtpTime);
        setSyncInterval(5000);
    }
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
    IPAddress ntpServerIP; // NTP server's ip address

    while (Udp.parsePacket() > 0) ; // discard any previously received packets
    Serial.println("Transmit NTP Request");
    // get a random server from the pool
    WiFi.hostByName(ntpServerName, ntpServerIP);
    Serial.print(ntpServerName);
    Serial.print(": ");
    Serial.println(ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE) {
            Serial.println("Receive NTP Response");
            Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
        }
    }
    Serial.println("No NTP Response :-(");
    return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}
