//// DISCLAIMER: Wireless MIDI is usable, but not 100% reliable at least in my testing.
//// There will be a few missed notes on a melody line, and several missed notes if
//// many chords are played in succession. Let me know if you find a way to fix this.
//// AppleMIDI works flawlessly on a wired network connection.

//// Once the program is loaded on the ESP32, Apple devices should immediately be able to
  // detect it as a network MIDI device.
//// On Windows, install rtpMIDI (https://www.tobias-erichsen.de/software/rtpmidi.html) in
  // order to use the wireless MIDI functionality.



APPLEMIDI_CREATE_DEFAULTSESSION_ESP32_INSTANCE();
// #define APPLEMIDI_CREATE_DEFAULTSESSION_ESP32_INSTANCE()
// APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "ESP32", DEFAULT_CONTROL_PORT);
// APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, hostName, DEFAULT_CONTROL_PORT);

USING_NAMESPACE_APPLEMIDI

void MIDIsetup(){
    MIDI.begin(); // start listening on MIDI channel 1
    AppleMIDI.setHandleConnected(OnAppleMidiConnected);
    AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);
    AppleMIDI.setHandleError(OnAppleMidiError);

    MIDI.setHandleNoteOn(handleNoteOn);
    // MIDI.setHandleNoteOff(handleNoteOff);

    // MDNS.begin(AppleMIDI.getName());
    MDNS.addService("apple-midi", "udp", AppleMIDI.getPort());
    MDNS.addService("http", "tcp", 80);
    IPAddress remote(192, 168, 1, 4);
    AppleMIDI.sendInvite(remote); // port is 5004 by default
}

void MIDIloop(){
    MIDI.read();
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(const ssrc_t & ssrc, const char* name) {
    MIDIconnected = true;
    Serial.print(F("Connected to session "));
    Serial.println(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(const ssrc_t & ssrc) {
    MIDIconnected = false;
    Serial.println(F("Disconnected"));
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiError(const ssrc_t& ssrc, int32_t err) {
    Serial.print  (F("Exception "));
    Serial.print  (err);
    Serial.print  (F(" from ssrc 0x"));
    Serial.println(ssrc, HEX);

    switch (err){
        case Exception::NoResponseFromConnectionRequestException:
            Serial.println(F("xxx:yyy did't respond to the connection request. Check the address and port, and any firewall or router settings. (time)"));
        break;
        }
    }

// -----------------------------------------------------------------------------
// 
// -----------------------------------------------------------------------------
// static void OnAppleMidiNoteOn(byte channel, byte note, byte velocity) {
    // Serial.print(F("Incoming NoteOn  from channel: "));
    // Serial.print(channel);
    // Serial.print(F(", note: "));
    // Serial.print(note);
    // Serial.print(F(", velocity: "));
    // Serial.println(velocity);
// }

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// static void OnAppleMidiNoteOff(byte channel, byte note, byte velocity) {
    // Serial.print(F("Incoming NoteOff from channel: "));
    // Serial.print(channel);
    // Serial.print(F(", note: "));
    // Serial.print(note);
    // Serial.print(F(", velocity: "));
    // Serial.println(velocity);
// }

//////// MIDI stuff

CRGB lastPressed;  // holder for last-detected key color

void runLED(){
    EVERY_N_MILLISECONDS(50){ _hue++; gHue1++; gHue2--;}
    EVERY_N_MILLISECONDS(20){ 
        nscale8( leds, NUM_LEDS, 240); // ( sustain ? 3 : 10) );
    }
    MIDI2LED();
    FastLED.show();
    yield();
}

void MIDI2LED(){
    // MIDI note values 0 - 127 
    // 36-96 (for 61-key keboard)

    uint8_t _pos = MIDIdata[1]/127.0 * (NUM_LEDS/2-1); // map note to position
    uint8_t _col = MIDIdata[1]/127.0 * 224; // map note to position
    
    // assign color based on note position and intensity (velocity)
    RIGHT[_pos] = CHSV(_col + _hue, 255 - (MIDIdata[2]/2.0), MIDIdata[2]/127.0 * 255);
    LEFT [_pos] = RIGHT[_pos];
    if(MIDIdata[2] > 0 && millis()%2 == 0)
        MIDIdata[2]--;
    lastPressed = RIGHT[_pos]; // remember last-detected note color
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
    MIDIdata[0] = channel;
    MIDIdata[1] = pitch;
    MIDIdata[2] = velocity;
    MidiEventReceived = true;
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
    MIDIdata[0] = channel;
    MIDIdata[1] = pitch;
    MIDIdata[2] = velocity;
    MidiEventReceived = true;
}

void handlePitchBend(byte channel, int bend) {
    // fill strip with solid color based on pitch bend amount
    fill_solid(leds, NUM_LEDS, CHSV(map(bend, -8192, 8192, 0, 224), 255, 125)); // 0  8192  16383
    yield();
}

void handleControlChange(byte channel, byte number, byte value){
    // channel 1 = modulation
    if( number == 1 ){
        fill_solid( leds, NUM_LEDS, 0x222222 );
    }
    // channel 64 = damper / sustain pedal
    if( number == 64 ){
        if( value >= 64 ){
            fill_solid( leds, NUM_LEDS, lastPressed );
            sustain = true;
        } else {
            sustain = false;
        }
    }
    yield();
}

