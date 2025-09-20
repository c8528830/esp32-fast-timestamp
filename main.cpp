#include <Arduino.h>
#include "fast_Timestamp/timestamp_micros.h"
#include "fast_timestamp/timestamp_nanos.h"

void SpeedTest();
void FreeLoopTask(void* pvParameters);
void setup() {
    Serial.begin(115200);
    TimeStampMicros::SyncCpuHzFromIDF();
    SpeedTest();
    xTaskCreatePinnedToCore(FreeLoopTask, "FreeLoopTask", 5120, nullptr, configMAX_PRIORITIES - 1, nullptr, 1); //Use Core 1
}
void loop() {
    delay(10);
}
void FreeLoopTask(void* pvParameters) {
    const char* TAG = "FreeLoopTask";
    ESP_LOGI(TAG, "Task Start. Micros:%lu", micros());

    TimeStampMicros timer_1 = TimeStampMicros::Now();
    TimeStampMicros timer_2 = TimeStampMicros::Now();
    for(;;) {
        TimeStampMicros currentTime = TimeStampMicros::Now();

        //500ms timer
        if(currentTime - timer_1 >= 500000) {
            timer_1.Refresh();
            ESP_LOGI(TAG, "Timer_1 target. Micros:%lu", micros());
        }

        //200us timer
        if(currentTime - timer_2 >= 200) {
            //Run custom code. You can operator the gpio
            timer_2.Refresh();
        }
    }
}

void SpeedTest() {
    const char* TAG = "SpeedTest";
    ESP_LOGI(TAG, "Start.");
    TimeStampNanos speedTimer;
    TimeStampMicros currentTime;
    int preventOptimization = 0;

    speedTimer.Refresh();
    for(int i = 0; i < 10000; i++) {
        currentTime = TimeStampMicros::Now(); // or currentTime.Refresh();
    }
    int64_t elapsed = speedTimer.Diff();
    ESP_LOGI(TAG, "Time get new use %lld ns.", elapsed / 10000);
    
    TimeStampMicros diffTime = TimeStampMicros::Now();
    speedTimer.Refresh();
    for(int i = 0; i < 10000; i++) {
        if(diffTime - currentTime > 500) {
            preventOptimization = i;
        }
    }
    elapsed = speedTimer.Diff();
    ESP_LOGI(TAG, "operator(-) use %lld ns.", elapsed / 10000);


    ESP_LOGI(TAG, "CurrentTime: %lu PreventOptimization: %d", currentTime, preventOptimization);
}
