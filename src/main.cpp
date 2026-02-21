#include <iostream>
#include "riscv-core.hpp"
#include "memory.hpp"

int main() {
    std::cout << "Hallo Welt\n";
    Memory memory(1024);
    memory.writeMem(1, 23);
    
    RiscVCore cpu(memory);
    
    std::cout << memory.readMem(1) << std::endl;

    cpu.step();

    return 0;
}