// SemaphoreHandle_t FFTMutex;
TaskHandle_t Task0;

void core0_Task0( void * parameter )
{
  for (;;) {
    if(FFTenable){
        // xSemaphoreTake( FFTMutex, portMAX_DELAY );
        fftLoop();
        // xSemaphoreGive( FFTMutex );
        delay(1);
    }else{
        delay(100);
    }
  }
}

void dualCoreInit(){
#ifdef debug
    _serial_.println("\tStarting dualCoreInit");
#endif
   xTaskCreatePinnedToCore(
        core0_Task0,    /* Task function. */
        "core0Task0",   /* name of task. */
        1000,           /* Stack size of task */
        NULL,           /* parameter of the task */
        10,              /* priority of the task */
        &Task0,         /* Task handle to keep track of created task */
        0               /* pin task to core 0 */
    );
    delay(500);  // needed to start-up task1

    // FFTMutex = xSemaphoreCreateMutex();
#ifdef debug
    _serial_.println("\tEnding dualCoreInit");
#endif
}

// /* A task that uses the semaphore. */
// void vAnotherTask( void * pvParameters )
// {
    // /* ... Do other things. */

    // if( xSemaphore != NULL )
    // {
        // /* See if we can obtain the semaphore.  If the semaphore is not
        // available wait 10 ticks to see if it becomes free. */
        // if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        // {
            // /* We were able to obtain the semaphore and can now access the
            // shared resource. */

            // /* ... */

            // /* We have finished accessing the shared resource.  Release the
            // semaphore. */
            // xSemaphoreGive( xSemaphore );
        // }
        // else
        // {
            // /* We could not obtain the semaphore and can therefore not access
            // the shared resource safely. */
        // }
    // }
// }
