#pragma once
#include <cstdint>
#include <vector>
#include <mutex>

class Memory {
    public:
    Memory(size_t size_words);
    uint32_t readMem(size_t addr);
    void writeMem(size_t addr, uint32_t data);

    private:
    std::vector<uint32_t> mem_;
    std::mutex mtx_;

};