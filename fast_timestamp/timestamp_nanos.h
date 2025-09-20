#pragma once
#include <Arduino.h>
#include "esp_private/esp_clk.h"  // for esp_clk_cpu_freq()

struct TimeStampNanos {
    // 內部狀態：32-bit CPU cycle counter 快照（xthal_get_ccount()）
    uint32_t cycles;

    // 每微秒的 cycles 數（= CPU MHz）。預設 240，建議 setup() 時呼叫 SyncCpuHzFromIDF()
    inline static uint32_t cpuHz = 240;

    // ---- 建構/指派 ----
    TimeStampNanos() : cycles(0) {}
    explicit TimeStampNanos(uint64_t c) : cycles(c) {}
    TimeStampNanos(const TimeStampNanos& o) : cycles(o.cycles) {}
    TimeStampNanos(TimeStampNanos&& o) noexcept : cycles(o.cycles) {}

    TimeStampNanos& operator=(uint64_t ns) { cycles = ns_to_cycles(ns); return *this; }
    volatile TimeStampNanos& operator=(uint64_t ns) volatile { cycles = ns_to_cycles(ns); return *this; }

    // 從同型別指派
    TimeStampNanos& operator=(const TimeStampNanos& o) { cycles = o.cycles; return *this; }
    volatile TimeStampNanos& operator=(const TimeStampNanos& o) volatile { cycles = o.cycles; return *this; }

    // ---- 公用工具 ----
    static inline void SyncCpuHzFromIDF() { cpuHz = esp_clk_cpu_freq() / 1000000u; }

    // cycles -> ns（四捨五入）
    static inline uint64_t cycles_to_ns(uint32_t cyc) {
        // 1 cycle = (1000 / cpuHz) ns
        // 四捨五入：(cyc*1000 + cpuHz/2) / cpuHz
        return ( (uint64_t)cyc * 1000ull + (cpuHz >> 1) ) / cpuHz;
    }

    // ns -> cycles（四捨五入）
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

    // ---- 差值（回傳奈秒，wrap-safe、有號）----
    // 有效視窗：±(2^31-1) cycles ≈ ±8.94 s @ 240MHz
    int64_t operator-(const TimeStampNanos& other) const {
        int32_t diff_cyc = (int32_t)(cycles - other.cycles);          // signed 環形差
        return (int64_t)diff_cyc * 1000ll / (int64_t)cpuHz;           // 轉 ns（向 0 取整）
    }
    int64_t operator-(const TimeStampNanos& other) const volatile {
        int32_t diff_cyc = (int32_t)(cycles - other.cycles);
        return (int64_t)diff_cyc * 1000ll / (int64_t)cpuHz;
    }

    // ---- 以「微秒」為單位的加減 ----
    // TimeStampNanos  operator+(uint64_t ns) const { return TimeStampNanos{ (uint32_t)(cycles + ns_to_cycles(ns)) }; }
    // TimeStampNanos  operator-(uint64_t ns) const { return TimeStampNanos{ (uint32_t)(cycles - ns_to_cycles(ns)) }; }
    TimeStampNanos& operator+=(uint64_t ns) { cycles += ns_to_cycles(ns); return *this; }
    TimeStampNanos& operator-=(uint64_t ns) { cycles -= ns_to_cycles(ns); return *this; }

    volatile TimeStampNanos& operator+=(uint64_t ns) volatile { cycles += ns_to_cycles(ns); return *this; }
    volatile TimeStampNanos& operator-=(uint64_t ns) volatile { cycles -= ns_to_cycles(ns); return *this; }

    // ---- 比較（wrap-safe，在 ±2^31 cycles 視窗內正確排序）----
        static inline int32_t cyc_cmp(uint32_t a, uint32_t b) { return (int32_t)(a - b); }

    bool operator<(const TimeStampNanos& o)  const { return cyc_cmp(cycles, o.cycles) <  0; }
    bool operator>(const TimeStampNanos& o)  const { return cyc_cmp(cycles, o.cycles) >  0; }
    bool operator<=(const TimeStampNanos& o) const { return cyc_cmp(cycles, o.cycles) <= 0; }
    bool operator>=(const TimeStampNanos& o) const { return cyc_cmp(cycles, o.cycles) >= 0; }
    bool operator==(const TimeStampNanos& o) const { return cycles == o.cycles; }
    bool operator!=(const TimeStampNanos& o) const { return cycles != o.cycles; } // ← 修正了原本的邏輯錯誤

    bool operator<(const TimeStampNanos& o)  const volatile { return cyc_cmp(cycles, o.cycles) <  0; }
    bool operator>(const TimeStampNanos& o)  const volatile { return cyc_cmp(cycles, o.cycles) >  0; }
    bool operator<=(const TimeStampNanos& o) const volatile { return cyc_cmp(cycles, o.cycles) <= 0; }
    bool operator>=(const TimeStampNanos& o) const volatile { return cyc_cmp(cycles, o.cycles) >= 0; }
    bool operator==(const TimeStampNanos& o) const volatile { return cycles == o.cycles; }
    bool operator!=(const TimeStampNanos& o) const volatile { return cycles != o.cycles; }

    // ⚠ 與「整數微秒」比較：僅是「當前快照的微秒值」直比，不具 wrap 安全
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


    // ---- 工具：現在時間（以 cycles 快照）----
    static inline TimeStampNanos IRAM_ATTR Now() {
        return TimeStampNanos{ xthal_get_ccount() };
    }
};