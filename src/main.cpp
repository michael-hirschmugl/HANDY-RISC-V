#include <iostream>
#include "riscv-core.hpp"
#include "memory.hpp"

std::vector<uint32_t> program = {
    0x00A00093, // addi x1, x0, 10
    0x00300113, // addi x2, x0, 3
    0x002081B3, // add  x3, x1, x2
    0x40218233, // sub  x4, x3, x2
    0x00211293, // slli x5, x2, 2
    0x0010E313, // ori  x6, x1, 1
    0x00602023, // sw   x6, 0(x0)
    0x00002383, // lw   x7, 0(x0)
    0x00638463, // beq  x7, x6, 8
    0x07B00413, // addi x8, x0, 123
    0x004004EF, // jal  x9, 4
    0x3E700513, // addi x10, x0, 999
    0x123455B7, // lui  x11, 0x12345
    0x00001617, // auipc x12, 0x1
    0x00000073  // ecall
};

int main() {
    std::cout << "Hallo Welt\n";
    Memory memory(1024);
    memory.writeMem(0, program[0]);
    memory.writeMem(1, program[1]);
    memory.writeMem(2, program[2]);
    memory.writeMem(3, program[3]);
    memory.writeMem(4, program[4]);
    memory.writeMem(5, program[5]);
    memory.writeMem(6, program[6]);
    memory.writeMem(7, program[7]);
    memory.writeMem(8, program[8]);
    memory.writeMem(9, program[9]);
    memory.writeMem(10, program[10]);
    memory.writeMem(11, program[11]);
    memory.writeMem(12, program[12]);
    memory.writeMem(13, program[13]);
    memory.writeMem(14, program[14]);
    
    RiscVCore cpu(memory);
    
    std::cout << memory.readMem(1) << std::endl;

    cpu.step();
    cpu.step();
    cpu.step();
    cpu.step();
    cpu.step();
    cpu.step();
    cpu.step();
    cpu.step();
    cpu.step();
    cpu.step();

    return 0;
}