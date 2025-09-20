# esp32-fast-timestamp
`ESP32 Fast Timestamp – Micros / Nanos Measurement Library.`
---
A high-resolution timestamp library for ESP32, providing microsecond and nanosecond precision using the CPU cycle counter.
On ESP32 at 240 MHz, a single timestamp acquisition (Now()) takes only ~90 ns (≈ 20 CPU cycles).


When disabling the watchdog and running a delay-free loop is very helpful for precise timing control.

A high-resolution timestamp library for ESP32, providing
microsecond and nanosecond precision using the CPU cycle counter.

- 🚀 Ultra-fast: reads directly from `xthal_get_ccount()`
- ⏱️ Provides both `TimestampMicros` and `TimestampNanos`
- 🔄 Wrap-safe difference calculation (handles 32-bit cycle counter overflow)
- ⚡ Supports operator overloading (-, +, comparisons, assignment)
- 🔧 Refresh(), Reset(), Diff() methods for convenient usage

---
| Method                   | Resolution | Avg. Call Overhead (240 MHz) |
| ------------------------ | ---------- | ---------------------------- |
| `TimeStampMicros::Now()` | 1 µs       | \~90–100 ns (\~20–24 cycles) |
| `TimeStampNanos::Now()`  | 4.17 ns    | \~90–100 ns (\~20–24 cycles) |
| `operator-` (diff calc)  | 1 µs / ns  | \~4 ns (≈1 cycle)            |
| `esp_timer_get_time()`   | 1 µs       | \~1050 ns (≈250 cycles)      |

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
    int preventOptimization1 = 0;
    int64_t preventOptimization2 = 0;
    int64_t preventOptimization3 = 0;

    speedTimer.Refresh();
    for(int i = 0; i < 10000; i++) {
        currentTime = TimeStampMicros::Now(); // or currentTime.Refresh();
    }
    int64_t elapsed = speedTimer.Diff();
    ESP_LOGI(TAG, "Time get new use %lld ns.", elapsed / 10000); //SpeedTest: Time get new use 96 ns.
    
    TimeStampMicros diffTime = TimeStampMicros::Now();
    speedTimer.Refresh();
    for(int i = 0; i < 10000; i++) {
        if(diffTime - currentTime < 500) {
            preventOptimization1 = i;
        }
    }
    elapsed = speedTimer.Diff();
    ESP_LOGI(TAG, "operator(-) use %lld ns.", elapsed / 10000); //operator(-) use 4 ns.

    speedTimer.Refresh();
    for(int i = 0; i < 10000; i++) {
        preventOptimization2 = esp_timer_get_time();  // it's same millis() or micros();
    }
    elapsed = speedTimer.Diff();
    ESP_LOGI(TAG, "esp_timer_get_time use %lld ns.", elapsed / 10000); //esp_timer_get_time use 1075 ns.[0m

    ESP_LOGI(TAG, "CurrentTime: %lu PreventOptimization1: %d PreventOptimization2: %lld", currentTime, preventOptimization1, preventOptimization2);
}


```

Output
---
```
[0;32mI (692) SpeedTest: Time get new use 97 ns.[0m
[0;32mI (692) SpeedTest: operator(-) use 0 ns.[0m
[0;32mI (688) main_task: Returned from app_main()[0m
[0;32mI (703) SpeedTest: esp_timer_get_time use 1062 ns.[0m
[0;32mI (703) SpeedTest: CurrentTime: 1070514176 PreventOptimization1: 9999 PreventOptimization2: 434277[0m
[0;32mI (712) FreeLoopTask: Task Start. Micros:443702[0m
[0;32mI (1217) FreeLoopTask: Timer_1 target. Micros:948828[0m
[0;32mI (1717) FreeLoopTask: Timer_1 target. Micros:1448828[0m
[0;32mI (2217) FreeLoopTask: Timer_1 target. Micros:1948829[0m
[0;32mI (2717) FreeLoopTask: Timer_1 target. Micros:2448829[0m
[0;32mI (3217) FreeLoopTask: Timer_1 target. Micros:2948830[0m
[0;32mI (3717) FreeLoopTask: Timer_1 target. Micros:3448830[0m

```
