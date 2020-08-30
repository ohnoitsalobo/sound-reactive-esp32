#include <SPIFFS.h>        // 
#include <SPIFFSEditor.h>  // filesystem libraries
#include <FS.h>            // 

#include <WiFi.h>          // WiFi library
#include <ArduinoOTA.h>    // OTA update library
#include <ESPmDNS.h>       // mDNS library
const char* ssid = "linksys1";       // ssid
const char* password = "9182736450"; // password
const char * hostName = "esp-async"; // mDNS name = http://[hostname].local
const char* http_username = "admin";
const char* http_password = "admin";


#define CONFIG_ASYNC_TCP_RUNNING_CORE 0
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
String WSdata = "";

#include <WiFiUdp.h>
#include <TimeLib.h>
static const char ntpServerName[] = "pool.ntp.org";
const double timeZone = 5.5; // IST

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

#include <arduinoFFT.h>

#define FASTLED_INTERNAL // suppresses FastLED pragma messages
#include <FastLED.h>
#define NUM_LEDS 72
bool music = 1;
bool manual = 0;
bool _auto = 0;
bool FFTenable = true;

#define APPLEMIDI_INITIATOR
#include <AppleMIDI.h>
bool MIDIconnected = false;
byte MIDIdata[] = {0, 0, 0};
uint8_t _hue = 0;             // modifier for key color cycling
bool sustain = false;         // is sustain pedal on?
bool MidiEventReceived = false;