#define LeftPin  36
#define RightPin 39
#define samples  512 // must ALWAYS be a power of 2
#define samplingFrequency 25000 // 25000

#define noise 1500
#define MAX 50000
#define max_max MAX
#define max_min MAX/5
#define min_max noise
#define min_min noise/5

unsigned int sampling_period_us;
unsigned long microseconds;

double vReal[2][samples];
double vImag[2][samples];
double spectrum[3][samples/2];

arduinoFFT LFFT = arduinoFFT(vReal[0], vImag[0], samples, samplingFrequency);
arduinoFFT RFFT = arduinoFFT(vReal[1], vImag[1], samples, samplingFrequency);

void fftSetup(){
    sampling_period_us = round(1000000*(1.0/samplingFrequency));
    for (uint16_t i = 2; i < samples/2; i++){
        spectrum[0][i] = pow((i-2)/(samples/2.0-2), 0.66) * NUM_LEDS/2;
        spectrum[1][i] = 0;
        spectrum[2][i] = 0;

        // _serial_.print(i);
        // _serial_.print(",");
        // _serial_.print(((i-1) * 1.0 * samplingFrequency) / samples);
        // _serial_.print(",");
        // _serial_.print((int)spectrum[0][i]);
        /*  * /
        for(uint8_t x = 0; x < 40; x++){
            _serial_.print((int)(pow((i-2)/(samples/2.0-2), (0.4+x/100.0)) * NUMBER_OF_LEDS/2));
            _serial_.print(",");
        }
        /*  */
        // _serial_.print("\r\n");
    }
}

void fftLoop(){
#ifdef debug
    _serial_.println("Starting fftLoop");
#endif

    microseconds = micros();
    for(int i=0; i<samples; i++){
        vReal[0][i] = analogRead(LeftPin);
        // vReal[1][i] = analogRead(RightPin);
        vImag[0][i] = 0;
        // vImag[1][i] = 0;
        while(micros() - microseconds < sampling_period_us){  }
        microseconds += sampling_period_us;
    }

    LFFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    LFFT.Compute(FFT_FORWARD);
    LFFT.ComplexToMagnitude();
    
    // RFFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    // RFFT.Compute(FFT_FORWARD);
    // RFFT.ComplexToMagnitude();

    PrintVector(vReal[0], (samples >> 1), 1);
    // PrintVector(vReal[1], (samples >> 1), 2);

#ifdef debug
    _serial_.println("Ending fftLoop");
#endif
}

void PrintVector(double *vData, uint16_t bufferSize, int leftRight) {
    for (uint16_t i = 2; i < bufferSize; i++){
        if(vData[i] > noise){
            // _serial_.println(vData[i], 4);
            spectrum[leftRight][i] = vData[i]-noise;
            if(spectrum[leftRight][i] > MAX)
                spectrum[leftRight][i] = MAX;
        }else{
            spectrum[leftRight][i] = 0;
        }
        spectrum[2][i] = spectrum[1][i]; // mono only
        yield();
    }
}