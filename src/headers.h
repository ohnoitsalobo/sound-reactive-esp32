#include <SPIFFS.h>
#include <SPIFFSEditor.h>
#include <FS.h>

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
// change ssid/password
const char* ssid = "linksys1";
const char* password = "9182736450";
// change hostname to be something recognizable - network address will be http://[hostName].local
const char * hostName = "ESP32";


#define CONFIG_ASYNC_TCP_RUNNING_CORE 0  // configure async server to run from second CPU core
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
const char* http_username = "admin";
const char* http_password = "admin";
String WSdata = "";  // holder for WebSocket data

#include <TimeLib.h>
static const char ntpServerName[] = "pool.ntp.org";
const double timeZone = 5.5; // IST
time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
#include <WiFiUdp.h>
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

#include <arduinoFFT.h>

#define FASTLED_INTERNAL // suppress FastLED pragma messages
#include <FastLED.h>
// change number of LEDs
#define NUM_LEDS 144
bool music = 1;    //
bool manual = 0;   // change which is '1' to change the default starting mode.
bool _auto = 0;    //
bool FFTenable = true;

#define APPLEMIDI_INITIATOR
#include <AppleMIDI.h>
bool MIDIconnected = false;
byte MIDIdata[] = {0, 0, 0};
uint8_t _hue = 0;             // modifier for key color cycling
bool sustain = false;         // is sustain pedal on?
bool MidiEventReceived = false;
