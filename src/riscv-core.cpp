#include "riscv-core.hpp"

RiscVCore::RiscVCore(Memory& memory) : mem_(memory) {}

uint32_t RiscVCore::readPC(){
    std::lock_guard<std::mutex> lock(mtx_);
    return pc;
}

void RiscVCore::writePC(uint32_t value){
    std::lock_guard<std::mutex> lock(mtx_);
    pc = value;
}

uint32_t RiscVCore::readX(size_t pos){
    std::lock_guard<std::mutex> lock(mtx_);
    return x_[pos];
}

void RiscVCore::writeX(size_t pos, uint32_t value){
    std::lock_guard<std::mutex> lock(mtx_);
    if (pos != 0) {
        x_[pos] = value;
    }
}

void RiscVCore::step(){
    std::cout << "single step" << std::endl;
    fetch();
}

void RiscVCore::fetch(){
    instruction = mem_.readMem(program_mem_base_offset + pc);
}

void RiscVCore::decode(){
    uint32_t opcode = instruction & 0x7FU;
    uint32_t rd(0);
    uint32_t funct3(0);
    uint32_t rs1(0);
    uint32_t rs2(0);
    uint32_t funct7(0);
    uint32_t imm(0);

    switch(opcode){
        case 0b0110011: //R-Type
            break;
        case 0b0010011: //I-Type
            break;
        case 0b0000011: //I-Type
            break;
        case 0b0100011: //S-Type
            break;
        case 0b1100011: //B-Type
            break;
        case 0b1101111: //J-Type
            break;
        case 0b1100111: //I-Type
            break;
        default:
            //illegal instruction
            break;
    }
}