AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
    if(type == WS_EVT_CONNECT){
        _serial_.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("Hello Client %u :)", client->id());
        client->ping();
    } else if(type == WS_EVT_DISCONNECT){
        _serial_.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    } else if(type == WS_EVT_ERROR){
        _serial_.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    } else if(type == WS_EVT_PONG){
        _serial_.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
    } else if(type == WS_EVT_DATA){
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        WSdata = "";
        if(info->final && info->index == 0 && info->len == len){
            //the whole message is in a single frame and we got all of it's data
            _serial_.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

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
            _serial_.printf("%s\n",WSdata.c_str());

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
                    _serial_.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
                    _serial_.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
            }

            _serial_.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

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
            _serial_.printf("%s\n",WSdata.c_str());

            if((info->index + len) == info->len){
                _serial_.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
                if(info->final){
                    _serial_.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
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
    _serial_.println("\tStarting wifiSetup");
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
    _serial_.println("\tEnding wifiSetup");
#endif
}

void wifiLoop(){
    if(WiFi.status() == WL_CONNECTED){
        ArduinoOTA.handle();
        ws.cleanupClients();
        EVERY_N_MILLISECONDS(2000){
            _serial_.println(WiFi.localIP());
        }
        // EVERY_N_MILLISECONDS(20){
            // if(music && webSocketConn()){
                // eqBroadcast = "E";
                // for(uint8_t i = 0; i < NUM_LEDS/4; i++){
                    // eqBroadcast += ",";
                    // eqBroadcast += String(eq[0][i]);
                    // if(eq[0][i] != 0) eq[0][i] /= 5.0;
                // }
                // for(uint8_t i = 0; i < NUM_LEDS/4; i++){
                    // eqBroadcast += ",";
                    // eqBroadcast += String(eq[1][i]);
                    // if(eq[1][i] != 0) eq[1][i] /= 5.0;
                // }
                // webSocket.broadcastTXT(eqBroadcast);
                // eqBroadcast = "";
            // }
        // }
        if(!digitalRead(2)){
            digitalWrite(2, HIGH);
            _serial_.println("Wifi connected!");
        }
    }
    
    if(WiFi.status() != WL_CONNECTED){
        EVERY_N_SECONDS(1){
            _serial_.println("Wifi disconnected");
            WiFi.begin(ssid, password);
        }
        if(digitalRead(2))
            digitalWrite(2, LOW);
    }
    yield();
}

void setupOTA(){
#ifdef debug
    _serial_.println("\t\tStarting setupOTA");
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
            _serial_.println("\r\nStart updating " + type);
            
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        })
        .onEnd([]() {
            digitalWrite(2, LOW);
            _serial_.println("\r\nEnd");
            events.send("Update End", "ota");
            delay(10);
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            digitalWrite(2, !digitalRead(2));
            char p[32];
            sprintf(p, "Progress: %u%%\n", (progress/(total/100)));
            events.send(p, "ota");
            uint32_t temp = progress / (total / 100);
            _serial_.printf("Progress: %u%%\r", temp);
            if(temp<99){
                // fill_solid (LEFT, map(temp, 0, 99, 0, NUM_LEDS/2), 0x020202);
                // fill_solid (RIGHT, map(temp, 0, 99, 0, NUM_LEDS/2), 0x020202);
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
            _serial_.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)        { _serial_.println("Auth Failed");    events.send("Auth Failed", "ota");    }
            else if (error == OTA_BEGIN_ERROR)  { _serial_.println("Begin Failed");   events.send("Begin Failed", "ota");   }
            else if (error == OTA_CONNECT_ERROR){ _serial_.println("Connect Failed"); events.send("Connect Failed", "ota"); }
            else if (error == OTA_RECEIVE_ERROR){ _serial_.println("Receive Failed"); events.send("Recieve Failed", "ota"); }
            else if (error == OTA_END_ERROR)    { _serial_.println("End Failed");     events.send("End Failed", "ota");     }
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        });
    ArduinoOTA.begin();
#ifdef debug
    _serial_.println("\t\tEnding setupOTA");
#endif
}

void setupServer(){
#ifdef debug
    _serial_.println("\t\tStarting setupServer");
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
            _serial_.printf("NOT_FOUND: ");
        if(request->method() == HTTP_GET)
            _serial_.printf("GET");
        else if(request->method() == HTTP_POST)
            _serial_.printf("POST");
        else if(request->method() == HTTP_DELETE)
            _serial_.printf("DELETE");
        else if(request->method() == HTTP_PUT)
            _serial_.printf("PUT");
        else if(request->method() == HTTP_PATCH)
            _serial_.printf("PATCH");
        else if(request->method() == HTTP_HEAD)
            _serial_.printf("HEAD");
        else if(request->method() == HTTP_OPTIONS)
            _serial_.printf("OPTIONS");
        else
            _serial_.printf("UNKNOWN");
            _serial_.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

        if(request->contentLength()){
            _serial_.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
            _serial_.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
        }

        int headers = request->headers();
        int i;
        for(i=0;i<headers;i++){
            AsyncWebHeader* h = request->getHeader(i);
            _serial_.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
        }

        int params = request->params();
        for(i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isFile()){
                _serial_.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
            } else if(p->isPost()){
                _serial_.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            } else {
                _serial_.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
        }

        request->send(404);
    });
        server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
        if(!index)
        _serial_.printf("UploadStart: %s\n", filename.c_str());
        _serial_.printf("%s", (const char*)data);
        if(final)
        _serial_.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
    });
    server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        if(!index)
        _serial_.printf("BodyStart: %u\n", total);
        _serial_.printf("%s", (const char*)data);
        if(index + len == total)
        _serial_.printf("BodyEnd: %u\n", total);
    });
    server.begin();
#ifdef debug
    _serial_.println("\t\tEnding setupServer");
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
            // fill_solid (LEFT, NUM_LEDS/2, manualColor_L);
        }else if(WSdata.endsWith("R")){
            manualColor_R = manualColor;
            // fill_solid (RIGHT, NUM_LEDS/2, manualColor_R);
        }else if(WSdata.endsWith("B")){
            manualColor_L = manualColor;
            manualColor_R = manualColor;
            // fill_solid (LEFT , NUM_LEDS/2, manualColor_L);
            // fill_solid (RIGHT, NUM_LEDS/2, manualColor_R);
        }
    }
    // WSdata = "";
}

//////// TIME //////////

void timeSetup(){
    Udp.begin(localPort);
}

void timeLoop(){
	if(timeStatus() != timeSet){
        // EVERY_N_SECONDS(30){
            setSyncProvider(getNtpTime);
            setSyncInterval(5000);
        // }
    }
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
    IPAddress ntpServerIP; // NTP server's ip address

    while (Udp.parsePacket() > 0) ; // discard any previously received packets
    _serial_.println("Transmit NTP Request");
    // get a random server from the pool
    WiFi.hostByName(ntpServerName, ntpServerIP);
    _serial_.print(ntpServerName);
    _serial_.print(": ");
    _serial_.println(ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE) {
            _serial_.println("Receive NTP Response");
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
    _serial_.println("No NTP Response :-(");
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
