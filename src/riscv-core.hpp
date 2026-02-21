#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <iostream>
#include "memory.hpp"
#include <mutex>

class RiscVCore {
    public:
    RiscVCore(Memory& memory);
    void step();
    

    private:
    Memory& mem_;
    std::array<uint32_t, 32> x_{};
    uint32_t pc{0};
    uint32_t readPC();
    void writePC(uint32_t value);
    uint32_t readX(size_t pos);
    void writeX(size_t pos, uint32_t value);
    uint32_t instruction{0};
    void fetch();
    inline static constexpr size_t program_mem_base_offset = 0;
    std::mutex mtx_;
    void decode();

};