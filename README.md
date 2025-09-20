# esp32-fast-timestamp
ESP32 Fast timestamp, Micros Nanos measure library. A high-resolution timestamp library for ESP32, providing microsecond and nanosecond precision using the CPU cycle counter.
On ESP32 at 240 MHz, a single timestamp acquisition (Now()) takes only ~85 ns (≈ 20 CPU cycles).

A high-resolution timestamp library for ESP32, providing
microsecond and nanosecond precision using the CPU cycle counter.

- 🚀 Ultra-fast: reads directly from `xthal_get_ccount()`
- ⏱️ Provides both `TimeStampMicros` and `TimeStampNanos`
- 🔄 Wrap-safe difference calculation
- ⚡ Supports `volatile` (ISR safe) and operator overloading

#include
#include "FastTimeStampMicros.h"

void setup() {
    Serial.begin(115200);
    TimeStampMicros::SyncCpuHzFromIDF();
}

void loop() {
    TimeStampMicros t0 = TimeStampMicros::Now();
    delay(10);
    int32_t diffTime_1 = t0.Diff();  // micros difference
    Serial.printf("Elapsed = %d us\n", diffTime_1);
    
    t0.Refresh(); //Get new time.
    delay(10);
    TimeStampMicros t1 = TimeStampMicros::Now();
    int32_t diffTime_2 = t1 - t0;    // micros difference
    Serial.printf("Elapsed = %d us\n", diffTime_2);

}
