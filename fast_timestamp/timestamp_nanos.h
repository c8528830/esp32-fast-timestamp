#pragma once
#include <Arduino.h>
#include "esp_private/esp_clk.h"  // for esp_clk_cpu_freq()

struct TimeStampNanos {
    uint32_t cycles;

    inline static uint32_t cpuHz = 240;

    TimeStampNanos() : cycles(0) {}
    explicit TimeStampNanos(uint64_t c) : cycles(c) {}
    TimeStampNanos(const TimeStampNanos& o) : cycles(o.cycles) {}
    TimeStampNanos(TimeStampNanos&& o) noexcept : cycles(o.cycles) {}

    TimeStampNanos& operator=(uint64_t ns) { cycles = ns_to_cycles(ns); return *this; }
    volatile TimeStampNanos& operator=(uint64_t ns) volatile { cycles = ns_to_cycles(ns); return *this; }

    TimeStampNanos& operator=(const TimeStampNanos& o) { cycles = o.cycles; return *this; }
    volatile TimeStampNanos& operator=(const TimeStampNanos& o) volatile { cycles = o.cycles; return *this; }

    static inline void SyncCpuHzFromIDF() { cpuHz = esp_clk_cpu_freq() / 1000000u; }

    static inline uint64_t cycles_to_ns(uint32_t cyc) {
        return ( (uint64_t)cyc * 1000ull + (cpuHz >> 1) ) / cpuHz;
    }

    static inline uint32_t ns_to_cycles(uint64_t ns) {
        // cycles = ns * cpuHz / 1000
        return (uint32_t)((ns * (uint64_t)cpuHz + 500ull) / 1000ull);
    }

    uint64_t Nanos() const { return cycles_to_ns(cycles); }
    uint64_t Nanos() const volatile { return cycles_to_ns(cycles); }

    operator uint64_t() const { return Nanos(); }
    operator uint64_t() const volatile { return Nanos(); }

    void Refresh() { cycles = xthal_get_ccount(); }
    void Reset() { cycles = 0; }
    void Reset() volatile { cycles = 0; }

    int64_t Diff() { 
        uint32_t diff = (uint32_t)(xthal_get_ccount() - cycles);
        return (int64_t)cycles_to_ns(diff);
     }

    int64_t operator-(const TimeStampNanos& other) const {
        int32_t diff_cyc = (int32_t)(cycles - other.cycles);  
        return (int64_t)diff_cyc * 1000ll / (int64_t)cpuHz; 
    }
    int64_t operator-(const TimeStampNanos& other) const volatile {
        int32_t diff_cyc = (int32_t)(cycles - other.cycles);
        return (int64_t)diff_cyc * 1000ll / (int64_t)cpuHz;
    }

    // TimeStampNanos  operator+(uint64_t ns) const { return TimeStampNanos{ (uint32_t)(cycles + ns_to_cycles(ns)) }; }
    // TimeStampNanos  operator-(uint64_t ns) const { return TimeStampNanos{ (uint32_t)(cycles - ns_to_cycles(ns)) }; }
    TimeStampNanos& operator+=(uint64_t ns) { cycles += ns_to_cycles(ns); return *this; }
    TimeStampNanos& operator-=(uint64_t ns) { cycles -= ns_to_cycles(ns); return *this; }

    volatile TimeStampNanos& operator+=(uint64_t ns) volatile { cycles += ns_to_cycles(ns); return *this; }
    volatile TimeStampNanos& operator-=(uint64_t ns) volatile { cycles -= ns_to_cycles(ns); return *this; }

        static inline int32_t cyc_cmp(uint32_t a, uint32_t b) { return (int32_t)(a - b); }

    bool operator<(const TimeStampNanos& o)  const { return cyc_cmp(cycles, o.cycles) <  0; }
    bool operator>(const TimeStampNanos& o)  const { return cyc_cmp(cycles, o.cycles) >  0; }
    bool operator<=(const TimeStampNanos& o) const { return cyc_cmp(cycles, o.cycles) <= 0; }
    bool operator>=(const TimeStampNanos& o) const { return cyc_cmp(cycles, o.cycles) >= 0; }
    bool operator==(const TimeStampNanos& o) const { return cycles == o.cycles; }
    bool operator!=(const TimeStampNanos& o) const { return cycles != o.cycles; } 

    bool operator<(const TimeStampNanos& o)  const volatile { return cyc_cmp(cycles, o.cycles) <  0; }
    bool operator>(const TimeStampNanos& o)  const volatile { return cyc_cmp(cycles, o.cycles) >  0; }
    bool operator<=(const TimeStampNanos& o) const volatile { return cyc_cmp(cycles, o.cycles) <= 0; }
    bool operator>=(const TimeStampNanos& o) const volatile { return cyc_cmp(cycles, o.cycles) >= 0; }
    bool operator==(const TimeStampNanos& o) const volatile { return cycles == o.cycles; }
    bool operator!=(const TimeStampNanos& o) const volatile { return cycles != o.cycles; }

    // bool operator<(uint64_t us)  const { return Nanos() <  us; }
    // bool operator>(uint64_t us)  const { return Nanos() >  us; }
    // bool operator<=(uint64_t us) const { return Nanos() <= us; }
    // bool operator>=(uint64_t us) const { return Nanos() >= us; }
    // bool operator==(uint64_t us) const { return Nanos() == us; }
    // bool operator!=(uint64_t us) const { return Nanos() != us; }

    // bool operator<(uint64_t us)  const volatile { return Nanos() <  us; }
    // bool operator>(uint64_t us)  const volatile { return Nanos() >  us; }
    // bool operator<=(uint64_t us) const volatile { return Nanos() <= us; }
    // bool operator>=(uint64_t us) const volatile { return Nanos() >= us; }
    // bool operator==(uint64_t us) const volatile { return Nanos() == us; }
    // bool operator!=(uint64_t us) const volatile { return Nanos() != us; }


    static inline TimeStampNanos IRAM_ATTR Now() {
        return TimeStampNanos{ xthal_get_ccount() };
    }
};
