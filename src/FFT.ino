/*----------------------------------------------------------------------------------------------------*\
My code was originally copied from G6EJD on Github as well as the ArduinoFFT library examples:
    https://github.com/G6EJD/ESP32-8-Octave-Audio-Spectrum-Display
    https://github.com/kosme/arduinoFFT

However I've modified the way it displays to look better on 
my single LED strips. It will have to be slightly modified for
different numbers of LEDs, for the best output.

FFT primer: (F)ast (F)ourier (T)ransform
  The "Nyquist" frequency is the *highest* frequency you can detect.
  It is exactly half your sampling rate - basically, you can reconstruct
  nearly any analog wave with a MINIMUM of 2 points.
  So, to detect sounds up to x Hz (your Nyquist frequency), you MUST sample at 2*x sps(samples per second)
  15kHz Nyquist = 30ksps sampling frequency
 
  The number of samples you take *at that sampling speed* determines the *lowest* frequency you can detect.
  Sampling_freq / num_samples = lowest frequency
  40000 kHz / 512 samples ~~> 78 Hz
  NOTE: by the end of the FFT calculations, your samples are transformed into samples/2 usable frequency
  values, so we discard the top half of the data

If your ESP32 is running at 240MHz, it can handily handle:
- (mono) one channel of audio at 1024 samples, or
- (stereo) two independent channels at 512 samples per channel
If, like mine, your ESP32 is locked at 160 MHz, stick with one channel, 512 samples 
\*----------------------------------------------------------------------------------------------------*/

#define LeftPin 39
#ifdef STEREO
#define RightPin  36
#endif

#define samples  512 // must ALWAYS be a power of 2 // VERY IMPORTANT
#define samplingFrequency 25000 // samples per second, not to be confused with Nyquist frequency which will be half of this
//// you don't always need all sounds up to 20kHz
  // 10-12kHz tends to work fine for most songs and speech
  // also picks up lower frequencies due to the math relation mentioned above
  // 25000 / 512 ~~> 50Hz

#define noise 1500 // Cheap noise rejection. If you have a high quality low-noise audio signal, you can lower this value, experiment to see.
#define MAX 50000  // if you have a weak audio signal you may want to adjust this. Experiment to see.

unsigned int sampling_period_us;
unsigned long microseconds;

double vReal[2][samples]; // holds the FFT calculations
double vImag[2][samples]; // holds the FFT calculations
double spectrum[3][samples/2]; // holds the final result, only half the final values are usable, hence 'samples/2'

arduinoFFT LFFT = arduinoFFT(vReal[0], vImag[0], samples, samplingFrequency);
#ifdef STEREO
arduinoFFT RFFT = arduinoFFT(vReal[1], vImag[1], samples, samplingFrequency);
#endif

void fftSetup(){
    sampling_period_us = round(1000000*(1.0/samplingFrequency));
    
    double exponent = 0.66;  //// this number will have to change for the best output on different numbers of LEDs and different numbers of samples.
    for (uint16_t i = 2; i < samples/2; i++){
        spectrum[0][i] = pow((i-2)/(samples/2.0-2), exponent) * NUM_LEDS; // **
        spectrum[1][i] = 0; // left  channel values
        spectrum[2][i] = 0; // right channel values
    }
    // ** the purpose of this line is to set up the 'logarithmic' spacing 
     // of the LED lights for different frequencies. The human perception of audio
    // frequency doesn't ascend linearly, so 
    // 120Hz vs 130Hz is 'musically' (and proportionally) a bigger change than
    // 1020Hz vs 1030Hz which is itself a smaller change than 
    // 10020Hz vs 10030 Hz
    // For this reason, I tried to set the exponent such that the lowest frequencies
    // each get their 'own' LED spot, while the higher frequencies 'share' LED spots
     // right now it is optimized for 72 LEDs, 512 samples ~ 0.66
     // with more LEDs you'll want a higher exponent, less LEDs a lower exponent, but it's a logarithmic relationship not linear.
     // let me know if you figure out a way to calculate it automatically, I figured it manually by trying different numbers.
}

//// remember that this code is running independently of your 
  // main LED code, on the other core of the ESP32.
void fftLoop(){
#ifdef debug
    Serial.println("Starting fftLoop");
#endif

    microseconds = micros();
    //// audio signal capture happens here
    for(int i=0; i<samples; i++){
        vReal[0][i] = analogRead(LeftPin);
        vImag[0][i] = 0;
#ifdef STEREO
        vReal[1][i] = analogRead(RightPin);
        vImag[1][i] = 0;
#endif
        while(micros() - microseconds < sampling_period_us){  }
        microseconds += sampling_period_us;
    }
    
    //// FFT magic happens here
    LFFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    LFFT.Compute(FFT_FORWARD);
    LFFT.ComplexToMagnitude();
    
#ifdef STEREO
    RFFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    RFFT.Compute(FFT_FORWARD);
    RFFT.ComplexToMagnitude();
    PrintVector(vReal[1], (samples >> 1), 2);
#endif

    //// audio frequencies are output here into the 'spectrum' array which is then read in the LED code
    PrintVector(vReal[0], (samples >> 1), 1);
#ifdef debug
    Serial.println("Ending fftLoop");
#endif
}

void PrintVector(double *vData, uint16_t bufferSize, int leftRight) {
    for (uint16_t i = 2; i < bufferSize; i++){ // starting at 2 because the first two data values aren't usable
        if(vData[i] > noise){
            spectrum[leftRight][i] = vData[i]-noise;
            if(spectrum[leftRight][i] > MAX)
                spectrum[leftRight][i] = MAX;
        }else{
            spectrum[leftRight][i] = 0;
        }
#ifndef STEREO
        spectrum[2][i] = spectrum[1][i]; // mono only
#endif
        yield();
    }
}