#pragma once
#include <Arduino.h>
#include "esp_private/esp_clk.h"  // for esp_clk_cpu_freq()

#if CONFIG_IDF_TARGET_ESP32S3
#define TIMG0_T0CONFIG_REG (*(volatile unsigned *)(0x6001F000)) // configuration register
#define TIMG0_T0LO_REG     (*(volatile uint32_t*)(0x6001F004)) // bottom 32-bits of the timer value
#define TIMG0_T0HI_REG     (*(volatile uint32_t*)(0x6001F008)) // top 32-bits of the timer value
#define TIMG0_T0UPDATE_REG (*(volatile uint32_t*)(0x6001F00C)) // write any value this to latch the timer value into hi_reg and lo_reg
#define TIMG0_T0LOAD_LO_REG (*(volatile uint32_t*)(0x6001F018)) 
#define TIMG0_T0LOAD_HI_REG (*(volatile uint32_t*)(0x6001F01C)) 
#define TIMG0_T0LOAD_REG    (*(volatile uint32_t*)(0x6001F020)) 

#define TIMG0_T1CONFIG_REG (*(volatile unsigned *)(0x60020024)) // configuration register
#define TIMG0_T1LO_REG     (*(volatile unsigned *)((0x60020028)))
#define TIMG0_T1HI_REG     (*(volatile unsigned *)((0x6002002C)))
#define TIMG0_T1UPDATE_REG (*(volatile unsigned *)((0x60020030)))
#define TIMG0_T1LOAD_LO_REG (*(volatile uint32_t*)(0x6002003C)) 
#define TIMG0_T1LOAD_HI_REG (*(volatile uint32_t*)(0x60020040)) 
#define TIMG0_T1LOAD_REG    (*(volatile uint32_t*)(0x60020044)) 
#else
#define TIMG0_T0CONFIG_REG (*(volatile unsigned *)(0x3FF5F000)) // configuration register
#define TIMG0_T0LO_REG     (*(volatile uint32_t*)(0x3FF5F004)) // bottom 32-bits of the timer value
#define TIMG0_T0HI_REG     (*(volatile uint32_t*)(0x3FF5F008)) // top 32-bits of the timer value
#define TIMG0_T0UPDATE_REG (*(volatile uint32_t*)(0x3FF5F00C)) // write any value this to latch the timer value into hi_reg and lo_reg
#define TIMG0_T0LOAD_LO_REG (*(volatile uint32_t*)(0x3FF5F018)) 
#define TIMG0_T0LOAD_HI_REG (*(volatile uint32_t*)(0x3FF5F01C)) 
#define TIMG0_T0LOAD_REG    (*(volatile uint32_t*)(0x3FF5F020)) 

#define TIMG0_T1CONFIG_REG (*(volatile unsigned *)(0x3FF5F024)) // configuration register
#define TIMG0_T1LO_REG     (*(volatile unsigned *)(0x3FF5F028))
#define TIMG0_T1HI_REG     (*(volatile unsigned *)(0x3FF5F02C))
#define TIMG0_T1UPDATE_REG (*(volatile unsigned *)(0x3FF5F030))
#define TIMG0_T1LOAD_LO_REG (*(volatile uint32_t*)(0x3FF5F03C)) 
#define TIMG0_T1LOAD_HI_REG (*(volatile uint32_t*)(0x3FF5F040)) 
#define TIMG0_T1LOAD_REG    (*(volatile uint32_t*)(0x3FF5F044)) 
#endif

struct TimeStampTimerG {
    uint32_t time;

    TimeStampTimerG() : time(0) {}
    explicit TimeStampTimerG(uint64_t c) : time(c) {}
    TimeStampTimerG(const TimeStampTimerG& o) : time(o.time) {}
    TimeStampTimerG(TimeStampTimerG&& o) noexcept : time(o.time) {}

    TimeStampTimerG& operator=(uint32_t micros) { time = micros; return *this; }
    volatile TimeStampTimerG& operator=(uint32_t micros) volatile { time = micros; return *this; }

    // 從同型別指派
    TimeStampTimerG& operator=(const TimeStampTimerG& o) { time = o.time; return *this; }
    volatile TimeStampTimerG& operator=(const TimeStampTimerG& o) volatile { time = o.time; return *this; }

    void Refresh() { time = Micros(); }
    void Reset() { time = 0; }
    void Reset() volatile { time = 0; }

    int32_t Diff() { return (int32_t)(Micros() - time); }


    // ---- 以「微秒」為單位的加減 ----
    TimeStampTimerG& operator+=(uint32_t us) { time += us; return *this; }
    TimeStampTimerG& operator-=(uint32_t us) { time -= us; return *this; }

    volatile TimeStampTimerG& operator+=(uint32_t us) volatile { time += us; return *this; }
    volatile TimeStampTimerG& operator-=(uint32_t us) volatile { time -= us; return *this; }

    bool operator<(const TimeStampTimerG& o)  const { return cyc_cmp(time, o.time) <  0; }
    bool operator>(const TimeStampTimerG& o)  const { return cyc_cmp(time, o.time) >  0; }
    bool operator<=(const TimeStampTimerG& o) const { return cyc_cmp(time, o.time) <= 0; }
    bool operator>=(const TimeStampTimerG& o) const { return cyc_cmp(time, o.time) >= 0; }
    bool operator==(const TimeStampTimerG& o) const { return time == o.time; }
    bool operator!=(const TimeStampTimerG& o) const { return time != o.time; } // ← 修正了原本的邏輯錯誤

    bool operator<(const TimeStampTimerG& o)  const volatile { return cyc_cmp(time, o.time) <  0; }
    bool operator>(const TimeStampTimerG& o)  const volatile { return cyc_cmp(time, o.time) >  0; }
    bool operator<=(const TimeStampTimerG& o) const volatile { return cyc_cmp(time, o.time) <= 0; }
    bool operator>=(const TimeStampTimerG& o) const volatile { return cyc_cmp(time, o.time) >= 0; }
    bool operator==(const TimeStampTimerG& o) const volatile { return time == o.time; }
    bool operator!=(const TimeStampTimerG& o) const volatile { return time != o.time; }
    
    static inline int32_t cyc_cmp(uint32_t a, uint32_t b) { return (int32_t)(a - b); }

    static inline uint32_t Micros() {
        TIMG0_T0UPDATE_REG = 1;
        while (TIMG0_T0UPDATE_REG) { __asm__ volatile ("waiti 0"); }
        return TIMG0_T0LO_REG;
    }
    static inline TimeStampTimerG IRAM_ATTR Now(void) {
        return TimeStampTimerG { Micros() };
    }

    static void Initialize() {
        unsigned divider = 80;
        TIMG0_T0CONFIG_REG &= ~(1 << 31);
        TIMG0_T0LOAD_LO_REG = 0;
        TIMG0_T0LOAD_HI_REG = 0;
        TIMG0_T0LOAD_REG = 1;
        TIMG0_T0CONFIG_REG = (divider << 13) | (1 << 31) | (1 << 30);
    }
};