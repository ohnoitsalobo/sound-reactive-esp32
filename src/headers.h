#include <SPIFFS.h>
#include <SPIFFSEditor.h>
#include <FS.h>

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
const char* ssid = "linksys1";
const char* password = "9182736450";
const char * hostName = "esp-async";
const char* http_username = "admin";
const char* http_password = "admin";


#define CONFIG_ASYNC_TCP_RUNNING_CORE 0
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
String WSdata = "";

#include <WiFiUdp.h>
#include <TimeLib.h>
static const char ntpServerName[] = "in.pool.ntp.org";
const double timeZone = 5.5; // IST

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

#include <arduinoFFT.h>
// #define FASTLED_ALLOW_INTERRUPTS 0
// #define INTERRUPT_THRESHOLD 1
// #define FASTLED_INTERRUPT_RETRY_COUNT 0
// #define FASTLED_ESP32_FLASH_LOCK 1
#define FASTLED_INTERNAL
#include <FastLED.h>
#define NUM_LEDS 144
bool music = 1;
bool FFTenable = true;

#define APPLEMIDI_INITIATOR
#include <AppleMIDI.h>
bool MIDIconnected = false;
byte MIDIdata[] = {0, 0, 0};
uint8_t _hue = 0;             // modifier for key color cycling
bool sustain = false;         // is sustain pedal on?
bool MidiEventReceived = false;

// SemaphoreHandle_t FFTMutex;
