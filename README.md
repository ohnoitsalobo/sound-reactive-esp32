# Audio frequency and wireless-MIDI reactive WS2812B LED program for ESP32-Arduino

This code was all written (and/or copied) in Notepad++, and compiled in PlatformIO. It _should_ would right out of the box.
**Should**.

1) Install necessary libraries.

- [FastLED](https://github.com/FastLED/FastLED)

- [ArduinoFFT](https://github.com/kosme/arduinoFFT)

- [AppleMIDI](https://github.com/lathoub/Arduino-AppleMIDI-Library)

- [Time](https://github.com/PaulStoffregen/Time)

- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)

- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)

2) Compile and upload program (`src` folder) AND filesystem (`data` folder)

- `pio run -t upload`

- `pio run -t uploadfs`

3) Wire up to necessary electronics

- LED data out -> ESP32 pin 13

- audio in -> pin 39 (mono), pins 39 and 36 (stereo) (see diagram `stereo_input_wiring.jpg`)
