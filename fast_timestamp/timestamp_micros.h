#pragma once
#include <Arduino.h>
#include "esp_private/esp_clk.h"  // for esp_clk_cpu_freq()

struct TimeStampMicros {
    // 內部狀態：32-bit CPU cycle counter 快照（xthal_get_ccount()）
    uint32_t cycles;

    // 每微秒的 cycles 數（= CPU MHz）。預設 240，建議 setup() 時呼叫 SyncCpuHzFromIDF()
    inline static uint32_t cpuHz = 240;

    // ---- 建構/指派 ----
    TimeStampMicros() : cycles(0) {}
    explicit TimeStampMicros(uint32_t c) : cycles(c) {}
    TimeStampMicros(const TimeStampMicros& o) : cycles(o.cycles) {}
    TimeStampMicros(TimeStampMicros&& o) noexcept : cycles(o.cycles) {}

    // 從微秒指派（外部看起來像 uint32_t 微秒）
    TimeStampMicros& operator=(uint32_t us) { cycles = us_to_cycles(us); return *this; }
    volatile TimeStampMicros& operator=(uint32_t us) volatile { cycles = us_to_cycles(us); return *this; }

    // 從同型別指派
    TimeStampMicros& operator=(const TimeStampMicros& o) { cycles = o.cycles; return *this; }
    volatile TimeStampMicros& operator=(const TimeStampMicros& o) volatile { cycles = o.cycles; return *this; }

    // ---- 公用工具 ----
    static inline void SyncCpuHzFromIDF() { cpuHz = esp_clk_cpu_freq() / 1000000u; }

    static inline uint32_t cycles_to_us(uint32_t cyc) {
        // 四捨五入： (cyc + cpuHz/2) / cpuHz
        return (uint32_t)(((uint64_t)cyc + (cpuHz >> 1)) / cpuHz);
    }
    static inline uint32_t us_to_cycles(uint32_t us) {
        return (uint32_t)((uint64_t)us * (uint64_t)cpuHz);
    }

    // 取「此快照的微秒值」（相對值，會隨 2^32 cycles 週期性回捲）
    uint32_t Micros() const { return cycles_to_us(cycles); }
    uint32_t Micros() const volatile { return cycles_to_us(cycles); }

    // 允許像 uint32_t 一樣被讀取（隱式轉微秒）
    operator uint32_t() const { return Micros(); }
    operator uint32_t() const volatile { return Micros(); }

    void Refresh() { cycles = xthal_get_ccount(); }
    void Reset() { cycles = 0; }
    void Reset() volatile { cycles = 0; }
    int32_t Diff() { 
        uint32_t diff = (uint32_t)(xthal_get_ccount() - cycles);
        return (int32_t)cycles_to_us(diff);
     }

    // ---- 差值（回傳微秒），自動處理 32-bit 溢位 ----
    // 注意：差值視窗限於 ±2^31 cycles（約 8.9 秒 @240MHz / 240 cycles/us）
    int32_t operator-(const TimeStampMicros& other) const {
        uint32_t diff = (uint32_t)(cycles - other.cycles);
        return (int32_t)cycles_to_us(diff);
    }
    int32_t operator-(const TimeStampMicros& other) const volatile {
        uint32_t diff = (uint32_t)(cycles - other.cycles);
        return (int32_t)cycles_to_us(diff);
    }

    // ---- 以「微秒」為單位的加減 ----
    // TimeStampMicros  operator+(uint32_t us) const { return TimeStampMicros{ (uint32_t)(cycles + us_to_cycles(us)) }; }
    // TimeStampMicros  operator-(uint32_t us) const { return TimeStampMicros{ (uint32_t)(cycles - us_to_cycles(us)) }; }
    TimeStampMicros& operator+=(uint32_t us) { cycles += us_to_cycles(us); return *this; }
    TimeStampMicros& operator-=(uint32_t us) { cycles -= us_to_cycles(us); return *this; }

    volatile TimeStampMicros& operator+=(uint32_t us) volatile { cycles += us_to_cycles(us); return *this; }
    volatile TimeStampMicros& operator-=(uint32_t us) volatile { cycles -= us_to_cycles(us); return *this; }

    // ---- 比較（wrap-safe，在 ±2^31 cycles 視窗內正確排序）----
    static inline int32_t cyc_cmp(uint32_t a, uint32_t b) { return (int32_t)(a - b); }

    bool operator<(const TimeStampMicros& o)  const { return cyc_cmp(cycles, o.cycles) <  0; }
    bool operator>(const TimeStampMicros& o)  const { return cyc_cmp(cycles, o.cycles) >  0; }
    bool operator<=(const TimeStampMicros& o) const { return cyc_cmp(cycles, o.cycles) <= 0; }
    bool operator>=(const TimeStampMicros& o) const { return cyc_cmp(cycles, o.cycles) >= 0; }
    bool operator==(const TimeStampMicros& o) const { return cycles == o.cycles; }
    bool operator!=(const TimeStampMicros& o) const { return cycles != o.cycles; } // ← 修正了原本的邏輯錯誤

    bool operator<(const TimeStampMicros& o)  const volatile { return cyc_cmp(cycles, o.cycles) <  0; }
    bool operator>(const TimeStampMicros& o)  const volatile { return cyc_cmp(cycles, o.cycles) >  0; }
    bool operator<=(const TimeStampMicros& o) const volatile { return cyc_cmp(cycles, o.cycles) <= 0; }
    bool operator>=(const TimeStampMicros& o) const volatile { return cyc_cmp(cycles, o.cycles) >= 0; }
    bool operator==(const TimeStampMicros& o) const volatile { return cycles == o.cycles; }
    bool operator!=(const TimeStampMicros& o) const volatile { return cycles != o.cycles; }

    // ⚠ 與「整數微秒」比較：僅是「當前快照的微秒值」直比，不具 wrap 安全
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

    // ---- 工具：現在時間（以 cycles 快照）----
    static inline TimeStampMicros IRAM_ATTR Now() {
        return TimeStampMicros{ xthal_get_ccount() };
    }
};