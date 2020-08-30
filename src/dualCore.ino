//// There are recommended practices for sharing data between independent cores,
//// practices which I have not implemented here due to 1) ignorance and 2) laziness.
//// I'm sharing *very little* data, just a couple of variables, and in any case
//// one core writes to the variable while the other core reads the variable.
//// Never both reading and writing to the same variable. Sort of like a one-way comm.

//// Basically, you don't want both cores trying to read from or write to a variable
//// simultaneously, as that could corrupt memory / cause crashes. You should 1) block
//// off the data while it is being accessed, or 2) send the shared data via a separate pipeline.

//// https://www.freertos.org/Inter-Task-Communication.html


TaskHandle_t Task0; // handle to 'track' task

//// this is your "loop" for Core 0 which will run independently of the main loop()
void core0_Task0( void * parameter )
{
  for (;;) {
    
    if(FFTenable){ //// IF I need FFT, I enable it using FFTenable from the other core and FFT will run
        fftLoop();
        delay(1);   // ESP32 crashes if this delay is not there, don't remove
    }else{         //// else just do nothing
        delay(100);
    }
    
  }
}

void dualCoreInit(){
#ifdef debug
    Serial.println("\tStarting dualCoreInit");
#endif

    //// to understand more about how the dual core code works,
    //// read up on FreeRTOS which is part of ESP32-Arduino Core.
    xTaskCreatePinnedToCore(
        core0_Task0,    /* Task function. */
        "core0Task0",   /* name of task. */
        1000,           /* Stack size of task */
        NULL,           /* parameter of the task */
        10,             /* priority of the task */
        &Task0,         /* Task handle to keep track of created task */
        0               /* pin task to core 0 */
    );
    delay(500);  // delay needed to start-up task

#ifdef debug
    Serial.println("\tEnding dualCoreInit");
#endif
}

