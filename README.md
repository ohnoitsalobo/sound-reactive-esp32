# Audio frequency and wireless-MIDI reactive WS2812B LED program for ESP32-Arduino

This code was all written (and/or copied) in Notepad++, and compiled in PlatformIO. It _should_ work right out of the box.
*Should*.

I originally wrote this program for my [sound-reactive speaker lights](https://www.instagram.com/p/BvR3FLChP0C/).

Later on I incorporated some code from [a previous MIDI LED project](https://www.instagram.com/p/BZefjNADfg1/) to make it MIDI-responsive, with some modifications to make it work over WiFi. Network MIDI is natively supported by Mac, and can be added Windows by installing [rtpMIDI](https://www.tobias-erichsen.de/software/rtpmidi.html).

Finally, I recently switched from the regular ESP32 Server code to AsyncServer, and learned how to run the audio analysis routine (FFT) on the secondary ESP32 processor, freeing up the primary CPU to run the LEDs independently. Thanks to [Andreas Spiess](https://github.com/SensorsIot/ESP32-Dual-Core) for his code.

___

1) **Install necessary libraries.**

Make sure you have the latest [Arduino Core for ESP32](https://github.com/espressif/arduino-esp32) installed. Then install the following libraries:

- [FastLED](https://github.com/FastLED/FastLED)

- [ArduinoFFT](https://github.com/kosme/arduinoFFT)

- [AppleMIDI](https://github.com/lathoub/Arduino-AppleMIDI-Library)

- [Time](https://github.com/PaulStoffregen/Time)

- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)

- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)

2) **Compile and upload program (`src`) AND filesystem (`data`)**

- change wireless SSID and password, LED data pin, and number of LEDs in `headers.h` (you may need to adjust the exponent in `FFT.ino` for the best audio-reactive display, but it should still look OK without adjustment).

- `pio run -t upload` (program)

- `pio run -t uploadfs` (filesystem)

3) **Wire it up**

- LED data out -> ESP32 pin 13

- audio in -> pin 39 (mono), pins 39 and 36 (stereo) - see diagram [`stereo_input_wiring.jpg`](https://github.com/ohnoitsalobo/sound-reactive-esp32/blob/master/Stereo_Input_Wiring.jpg)
