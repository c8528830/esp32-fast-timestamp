# esp32-fast-timestamp
ESP32 Fast timestamp, Micros Nanos measure library. A high-resolution timestamp library for ESP32, providing microsecond and nanosecond precision using the CPU cycle counter.
On ESP32 at 240 MHz, a single timestamp acquisition (Now()) takes only ~90 ns (≈ 20 CPU cycles).

When disabling the watchdog and running a delay-free loop is very helpful for precise timing control.

A high-resolution timestamp library for ESP32, providing
microsecond and nanosecond precision using the CPU cycle counter.

- 🚀 Ultra-fast: reads directly from `xthal_get_ccount()`
- ⏱️ Provides both `FastTimestampMicros` and `FastTimestampNanos`
- 🔄 Wrap-safe difference calculation
- ⚡ Supports operator overloading

How to use
---
* TimeStampMicros::Now() get micros  (A single execution takes only 90 ns.)
* TimeStampNanos::Now() get nanos

* .Refresh() refresh new time.
* .Reset() reset time.
* .Diff() get time difference

(Example 1: Basic Time Difference)
---
```c++
#include <Arduino.h>
#include "fast_Timestamp/timestamp_micros.h"
#include "fast_timestamp/timestamp_nanos.h"

void setup() {
    Serial.begin(115200);
    TimeStampMicros::SyncCpuHzFromIDF();
}
void loop() {
    TimeStampMicros t0 = TimeStampMicros::Now();
    delay(10);
    int32_t diffTime_1 = t0.Diff();  // micros difference
    Serial.printf("Elapsed = %d us\n", diffTime_1);
    
    t0.Refresh(); //you can use this method refresh new time.
    delay(10);
    TimeStampMicros t1 = TimeStampMicros::Now();
    int32_t diffTime_2 = t1 - t0;    // micros difference
    Serial.printf("Elapsed = %d us\n", diffTime_2);
}
```

(Example 2: disable the watchdog and delay-free loop)
---
```c++
#include <Arduino.h>
#include "fast_Timestamp/timestamp_micros.h"
#include "fast_timestamp/timestamp_nanos.h"

void SpeedTest();
void FreeLoopTask(void* pvParameters);
void setup() {
    Serial.begin(115200);
    TimeStampMicros::SyncCpuHzFromIDF();
    TimeStampNanos::SyncCpuHzFromIDF();
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
    ESP_LOGI(TAG, "Time get new use %lld ns.", elapsed / 10000); //SpeedTest: Time get new use 96 ns.
    
    TimeStampMicros diffTime = TimeStampMicros::Now();
    speedTimer.Refresh();
    for(int i = 0; i < 10000; i++) {
        if(diffTime - currentTime > 500) {
            preventOptimization = i;
        }
    }
    elapsed = speedTimer.Diff();
    ESP_LOGI(TAG, "operator(-) use %lld ns.", elapsed / 10000); //operator(-) use 4 ns.


    ESP_LOGI(TAG, "CurrentTime: %lu PreventOptimization: %d", currentTime, preventOptimization);
}

```

Output
---
```
[0;32mI (692) SpeedTest: Time get new use 96 ns.[0m
[0;32mI (692) SpeedTest: operator(-) use 4 ns.[0m
[0;32mI (692) SpeedTest: CurrentTime: 1070514176 PreventOptimization: 0[0m
[0;32mI (688) main_task: Returned from app_main()[0m
[0;32mI (699) FreeLoopTask: Task Start. Micros:430480[0m
[0;32mI (1209) FreeLoopTask: Timer_1 target. Micros:940500[0m
[0;32mI (1709) FreeLoopTask: Timer_1 target. Micros:1440500[0m
[0;32mI (2209) FreeLoopTask: Timer_1 target. Micros:1940501[0m
[0;32mI (2709) FreeLoopTask: Timer_1 target. Micros:2440502[0m
[0;32mI (3209) FreeLoopTask: Timer_1 target. Micros:2940502[0m
[0;32mI (3709) FreeLoopTask: Timer_1 target. Micros:3440502[0m
[0;32mI (4209) FreeLoopTask: Timer_1 target. Micros:3940503[0m

```
