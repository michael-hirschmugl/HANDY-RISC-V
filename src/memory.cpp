#include "memory.hpp"

Memory::Memory(size_t size_words) : mem_(size_words, 0) {}

uint32_t Memory::readMem(size_t addr){
    std::lock_guard<std::mutex> lock(mtx_);
    return mem_[addr];
}

void Memory::writeMem(size_t addr, uint32_t data){
    std::lock_guard<std::mutex> lock(mtx_);
    mem_[addr] = data;
}