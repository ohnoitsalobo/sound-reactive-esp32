/*----------------------------------------------------------------------------------------------------*\
  With all the headers in a separate file, all functions and global variables
  are accessible anywhere in the code.
\*----------------------------------------------------------------------------------------------------*/
#include "headers.h"

// #define debug 1
void setup(){
#ifdef debug
    Serial.println("Starting setup");
#endif
    Serial.begin(115200); pinMode(2, OUTPUT);
    Serial.setDebugOutput(true);

    //// set up WiFi - see wifi.ino
    wifiSetup();
    
    //// set up FFT array - see FFT.ino
    fftSetup();
    
    //// set up LEDs - see LEDs.ino
    ledSetup();
    
    //// set up MIDI - see MIDI.ino
    MIDIsetup();

    //// initialize routine for Core 0 (main loop runs on Core 1)
    //// see dualCore.ino
    dualCoreInit();
    //// note: a few other things run in the background on Core 0,
    //// including WiFi related tasks, so be careful.

#ifdef debug
    Serial.println("Ending setup");
#endif
}

void loop(){
#ifdef debug
    Serial.println("Starting loop");
#endif
    
    //// run WiFi handlers
    wifiLoop();
    //// run MIDI handlers
    MIDIloop();
    //// make LED flashy flashy brrrrrrr
    ledLoop();
   
#ifdef debug
    Serial.println("Ending loop");
#endif
}
