#define _serial_ Serial
#include "headers.h"

// #define debug 1
void setup(){
#ifdef debug
    _serial_.println("Starting setup");
#endif
    _serial_.begin(115200); pinMode(2, OUTPUT);
    _serial_.setDebugOutput(true);

    wifiSetup();
    
    fftSetup();
    
    ledSetup();
    
    MIDIsetup();

    dualCoreInit();

#ifdef debug
    _serial_.println("Ending setup");
#endif
}

void loop(){
#ifdef debug
    _serial_.println("Starting loop");
#endif
    
    wifiLoop();
    
    MIDIloop();

    ledLoop();
    
#ifdef debug
    _serial_.println("Ending loop");
#endif
}
