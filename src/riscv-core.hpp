#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <iostream>
#include "memory.hpp"
#include <mutex>
#include <string>
#include <sstream>
#include <iomanip>

enum class Op : uint16_t {
    // R-type
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,

    // I-type (OP-IMM)
    ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,

    // Loads
    LB, LH, LW, LBU, LHU,

    // Stores
    SB, SH, SW,

    // Branches
    BEQ, BNE, BLT, BGE, BLTU, BGEU,

    // Jumps
    JAL, JALR,

    // U-type
    LUI, AUIPC,

    // System
    ECALL, EBREAK,

    // Fence
    FENCE, FENCE_I,

    // Placeholder
    ILLEGAL
};

enum class ImmKind : uint8_t { None, I, S, B, U, J };

struct DecodedInstruction {
    Op      op = Op::ILLEGAL;

    uint8_t rd  = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;

    int32_t imm = 0;
    ImmKind imm_kind = ImmKind::None;

    uint32_t raw = 0;
};

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
    DecodedInstruction decoded_;
    std::string decoded_string_;

    static constexpr uint32_t bits(uint32_t x, int hi, int lo) noexcept {
        return (x >> lo) & ((1u << (hi - lo + 1)) - 1);
    }

    static constexpr int32_t sext(uint32_t x, int width) noexcept {
        uint32_t m = 1u << (width - 1);
        return (int32_t)((x ^ m) - m);
    }

    static inline std::string reg_name(uint8_t r) {
        return "x" + std::to_string(r);
    }

    static inline const char* op_mnemonic(Op op) {
        switch (op) {
            // R-type
            case Op::ADD:  return "add";
            case Op::SUB:  return "sub";
            case Op::SLL:  return "sll";
            case Op::SLT:  return "slt";
            case Op::SLTU: return "sltu";
            case Op::XOR:  return "xor";
            case Op::SRL:  return "srl";
            case Op::SRA:  return "sra";
            case Op::OR:   return "or";
            case Op::AND:  return "and";

            // OP-IMM
            case Op::ADDI:  return "addi";
            case Op::SLTI:  return "slti";
            case Op::SLTIU: return "sltiu";
            case Op::XORI:  return "xori";
            case Op::ORI:   return "ori";
            case Op::ANDI:  return "andi";
            case Op::SLLI:  return "slli";
            case Op::SRLI:  return "srli";
            case Op::SRAI:  return "srai";

            // Loads
            case Op::LB:  return "lb";
            case Op::LH:  return "lh";
            case Op::LW:  return "lw";
            case Op::LBU: return "lbu";
            case Op::LHU: return "lhu";

            // Stores
            case Op::SB:  return "sb";
            case Op::SH:  return "sh";
            case Op::SW:  return "sw";

            // Branches
            case Op::BEQ:  return "beq";
            case Op::BNE:  return "bne";
            case Op::BLT:  return "blt";
            case Op::BGE:  return "bge";
            case Op::BLTU: return "bltu";
            case Op::BGEU: return "bgeu";

            // Jumps
            case Op::JAL:  return "jal";
            case Op::JALR: return "jalr";

            // U-type
            case Op::LUI:   return "lui";
            case Op::AUIPC: return "auipc";

            // System
            case Op::ECALL:  return "ecall";
            case Op::EBREAK: return "ebreak";

            // Fence
            case Op::FENCE:   return "fence";
            case Op::FENCE_I: return "fence.i";

            case Op::ILLEGAL: return "illegal";
            default:          return "unknown";
        }
    }

    static inline std::string to_string(const DecodedInstruction& d) {
        std::ostringstream os;

        // Helpful to always see the raw instruction word
        os << op_mnemonic(d.op);

        switch (d.op) {
            // R-type: rd, rs1, rs2
            case Op::ADD: case Op::SUB: case Op::SLL: case Op::SLT: case Op::SLTU:
            case Op::XOR: case Op::SRL: case Op::SRA: case Op::OR:  case Op::AND:
                os << " " << reg_name(d.rd) << ", " << reg_name(d.rs1) << ", " << reg_name(d.rs2);
                break;

            // OP-IMM: rd, rs1, imm (SLLI/SRLI/SRAI use imm=shamt in your decoder)
            case Op::ADDI: case Op::SLTI: case Op::SLTIU:
            case Op::XORI: case Op::ORI:  case Op::ANDI:
            case Op::SLLI: case Op::SRLI: case Op::SRAI:
                os << " " << reg_name(d.rd) << ", " << reg_name(d.rs1) << ", " << d.imm;
                break;

            // Loads: rd, imm(rs1)
            case Op::LB: case Op::LH: case Op::LW: case Op::LBU: case Op::LHU:
                os << " " << reg_name(d.rd) << ", " << d.imm << "(" << reg_name(d.rs1) << ")";
                break;

            // Stores: rs2, imm(rs1)
            case Op::SB: case Op::SH: case Op::SW:
                os << " " << reg_name(d.rs2) << ", " << d.imm << "(" << reg_name(d.rs1) << ")";
                break;

            // Branches: rs1, rs2, imm (PC-relative offset)
            case Op::BEQ: case Op::BNE: case Op::BLT: case Op::BGE: case Op::BLTU: case Op::BGEU:
                os << " " << reg_name(d.rs1) << ", " << reg_name(d.rs2) << ", " << d.imm;
                break;

            // Jumps
            case Op::JAL:
                os << " " << reg_name(d.rd) << ", " << d.imm;
                break;

            case Op::JALR:
                os << " " << reg_name(d.rd) << ", " << d.imm << "(" << reg_name(d.rs1) << ")";
                break;

            // U-type: rd, imm (already shifted by 12 in your decoder)
            case Op::LUI:
            case Op::AUIPC:
                os << " " << reg_name(d.rd) << ", " << d.imm;
                break;

            // System / Fence: no operands (for your current subset)
            case Op::ECALL:
            case Op::EBREAK:
            case Op::FENCE:
            case Op::FENCE_I:
                break;

            case Op::ILLEGAL:
            default:
                break;
        }

        // Append raw encoding for debugging
        os << "  [raw=0x" << std::hex << std::setw(8) << std::setfill('0') << d.raw << "]";
        return os.str();
    }

};