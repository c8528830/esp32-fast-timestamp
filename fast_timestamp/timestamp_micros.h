#pragma once
#include <Arduino.h>
#include "esp_private/esp_clk.h"  // for esp_clk_cpu_freq()

struct TimeStampMicros {
    uint32_t cycles;

    inline static uint32_t cpuHz = 240;

    TimeStampMicros() : cycles(0) {}
    explicit TimeStampMicros(uint32_t c) : cycles(c) {}
    TimeStampMicros(const TimeStampMicros& o) : cycles(o.cycles) {}
    TimeStampMicros(TimeStampMicros&& o) noexcept : cycles(o.cycles) {}

    TimeStampMicros& operator=(uint32_t us) { cycles = us_to_cycles(us); return *this; }
    volatile TimeStampMicros& operator=(uint32_t us) volatile { cycles = us_to_cycles(us); return *this; }

    TimeStampMicros& operator=(const TimeStampMicros& o) { cycles = o.cycles; return *this; }
    volatile TimeStampMicros& operator=(const TimeStampMicros& o) volatile { cycles = o.cycles; return *this; }

    static inline void SyncCpuHzFromIDF() { cpuHz = esp_clk_cpu_freq() / 1000000u; }

    static inline uint32_t cycles_to_us(uint32_t cyc) {
        return (uint32_t)(((uint64_t)cyc + (cpuHz >> 1)) / cpuHz);
    }
    static inline uint32_t us_to_cycles(uint32_t us) {
        return (uint32_t)((uint64_t)us * (uint64_t)cpuHz);
    }

    uint32_t Micros() const { return cycles_to_us(cycles); }
    uint32_t Micros() const volatile { return cycles_to_us(cycles); }

    operator uint32_t() const { return Micros(); }
    operator uint32_t() const volatile { return Micros(); }

    void Refresh() { cycles = xthal_get_ccount(); }
    void Reset() { cycles = 0; }
    void Reset() volatile { cycles = 0; }
    int32_t Diff() { 
        uint32_t diff = (uint32_t)(xthal_get_ccount() - cycles);
        return (int32_t)cycles_to_us(diff);
     }

    int32_t operator-(const TimeStampMicros& other) const {
        uint32_t diff = (uint32_t)(cycles - other.cycles);
        return (int32_t)cycles_to_us(diff);
    }
    int32_t operator-(const TimeStampMicros& other) const volatile {
        uint32_t diff = (uint32_t)(cycles - other.cycles);
        return (int32_t)cycles_to_us(diff);
    }

    // TimeStampMicros  operator+(uint32_t us) const { return TimeStampMicros{ (uint32_t)(cycles + us_to_cycles(us)) }; }
    // TimeStampMicros  operator-(uint32_t us) const { return TimeStampMicros{ (uint32_t)(cycles - us_to_cycles(us)) }; }
    TimeStampMicros& operator+=(uint32_t us) { cycles += us_to_cycles(us); return *this; }
    TimeStampMicros& operator-=(uint32_t us) { cycles -= us_to_cycles(us); return *this; }

    volatile TimeStampMicros& operator+=(uint32_t us) volatile { cycles += us_to_cycles(us); return *this; }
    volatile TimeStampMicros& operator-=(uint32_t us) volatile { cycles -= us_to_cycles(us); return *this; }

    static inline int32_t cyc_cmp(uint32_t a, uint32_t b) { return (int32_t)(a - b); }

    bool operator<(const TimeStampMicros& o)  const { return cyc_cmp(cycles, o.cycles) <  0; }
    bool operator>(const TimeStampMicros& o)  const { return cyc_cmp(cycles, o.cycles) >  0; }
    bool operator<=(const TimeStampMicros& o) const { return cyc_cmp(cycles, o.cycles) <= 0; }
    bool operator>=(const TimeStampMicros& o) const { return cyc_cmp(cycles, o.cycles) >= 0; }
    bool operator==(const TimeStampMicros& o) const { return cycles == o.cycles; }
    bool operator!=(const TimeStampMicros& o) const { return cycles != o.cycles; }

    bool operator<(const TimeStampMicros& o)  const volatile { return cyc_cmp(cycles, o.cycles) <  0; }
    bool operator>(const TimeStampMicros& o)  const volatile { return cyc_cmp(cycles, o.cycles) >  0; }
    bool operator<=(const TimeStampMicros& o) const volatile { return cyc_cmp(cycles, o.cycles) <= 0; }
    bool operator>=(const TimeStampMicros& o) const volatile { return cyc_cmp(cycles, o.cycles) >= 0; }
    bool operator==(const TimeStampMicros& o) const volatile { return cycles == o.cycles; }
    bool operator!=(const TimeStampMicros& o) const volatile { return cycles != o.cycles; }

    // bool operator<(uint32_t us)  const { return Micros() <  us; }
    // bool operator>(uint32_t us)  const { return Micros() >  us; }
    // bool operator<=(uint32_t us) const { return Micros() <= us; }
    // bool operator>=(uint32_t us) const { return Micros() >= us; }
    // bool operator==(uint32_t us) const { return Micros() == us; }
    // bool operator!=(uint32_t us) const { return Micros() != us; }

    // bool operator<(uint32_t us)  const volatile { return Micros() <  us; }
    // bool operator>(uint32_t us)  const volatile { return Micros() >  us; }
    // bool operator<=(uint32_t us) const volatile { return Micros() <= us; }
    // bool operator>=(uint32_t us) const volatile { return Micros() >= us; }
    // bool operator==(uint32_t us) const volatile { return Micros() == us; }
    // bool operator!=(uint32_t us) const volatile { return Micros() != us; }

    static inline TimeStampMicros IRAM_ATTR Now() {
        return TimeStampMicros{ xthal_get_ccount() };
    }
};
