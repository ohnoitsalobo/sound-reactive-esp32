TaskHandle_t Task0;

void core0_Task0( void * parameter )
{
  for (;;) {
    if(FFTenable){
        fftLoop();
        delay(1);
    }else{
        delay(100);
    }
  }
}

void dualCoreInit(){
#ifdef debug
    Serial.println("\tStarting dualCoreInit");
#endif
   xTaskCreatePinnedToCore(
        core0_Task0,    /* Task function. */
        "core0Task0",   /* name of task. */
        1000,           /* Stack size of task */
        NULL,           /* parameter of the task */
        10,             /* priority of the task */
        &Task0,         /* Task handle to keep track of created task */
        0               /* pin task to core 0 */
    );
    delay(500);  // needed to start-up task1

#ifdef debug
    Serial.println("\tEnding dualCoreInit");
#endif
}
