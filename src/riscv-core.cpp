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

inline uint32_t RiscVCore::bits(uint32_t x, int hi, int lo) {
    return (x >> lo) & ((1u << (hi - lo + 1)) - 1);
}

inline int32_t RiscVCore::sext(uint32_t x, int width) {
    uint32_t m = 1u << (width - 1);
    return (int32_t)((x ^ m) - m);
}

void RiscVCore::decode() {
    uint32_t opcode = bits(instruction, 6, 0);
    uint32_t rd     = bits(instruction, 11, 7);
    uint32_t funct3 = bits(instruction, 14, 12);
    uint32_t rs1    = bits(instruction, 19, 15);
    uint32_t rs2    = bits(instruction, 24, 20);
    uint32_t funct7 = bits(instruction, 31, 25);

    // Immediates
    int32_t imm_i = sext(bits(instruction, 31, 20), 12);

    uint32_t imm_s_raw = (bits(instruction, 31, 25) << 5) | bits(instruction, 11, 7);
    int32_t imm_s = sext(imm_s_raw, 12);

    uint32_t imm_b_raw =
        (bits(instruction, 31, 31) << 12) |
        (bits(instruction, 7, 7)   << 11) |
        (bits(instruction, 30, 25) << 5)  |
        (bits(instruction, 11, 8)  << 1);
    int32_t imm_b = sext(imm_b_raw, 13);

    int32_t imm_u = (int32_t)(instruction & 0xFFFFF000u);

    uint32_t imm_j_raw =
        (bits(instruction, 31, 31) << 20) |
        (bits(instruction, 19, 12) << 12) |
        (bits(instruction, 20, 20) << 11) |
        (bits(instruction, 30, 21) << 1);
    int32_t imm_j = sext(imm_j_raw, 21);

    // OP-IMM shift fields
    uint32_t shamt      = bits(instruction, 24, 20);  // imm[4:0]
    uint32_t imm_i_11_5 = bits(instruction, 31, 25);  // imm[11:5]

    auto illegal = [&]() {
        // TODO: set trap / exception / flag
        // e.g. raise_illegal_instruction(instruction);
    };

    switch (opcode) {

        // =========================================================
        // OP (R-Type): ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND
        // =========================================================
        case 0b0110011: {
            switch (funct3) {
                case 0x0: // ADD/SUB
                    if (funct7 == 0x00) { /* ADD  */ }
                    else if (funct7 == 0x20) { /* SUB */ }
                    else illegal();
                    break;

                case 0x1: // SLL
                    if (funct7 == 0x00) { /* SLL */ } else illegal();
                    break;

                case 0x2: // SLT
                    if (funct7 == 0x00) { /* SLT */ } else illegal();
                    break;

                case 0x3: // SLTU
                    if (funct7 == 0x00) { /* SLTU */ } else illegal();
                    break;

                case 0x4: // XOR
                    if (funct7 == 0x00) { /* XOR */ } else illegal();
                    break;

                case 0x5: // SRL/SRA
                    if (funct7 == 0x00) { /* SRL */ }
                    else if (funct7 == 0x20) { /* SRA */ }
                    else illegal();
                    break;

                case 0x6: // OR
                    if (funct7 == 0x00) { /* OR */ } else illegal();
                    break;

                case 0x7: // AND
                    if (funct7 == 0x00) { /* AND */ } else illegal();
                    break;

                default:
                    illegal();
                    break;
            }
            break;
        }

        // =========================================================
        // OP-IMM (I-Type): ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI
        // =========================================================
        case 0b0010011: {
            switch (funct3) {
                case 0x0: /* ADDI  (rd, rs1, imm_i) */ break;
                case 0x2: /* SLTI  (rd, rs1, imm_i) */ break;
                case 0x3: /* SLTIU (rd, rs1, imm_i) */ break;
                case 0x4: /* XORI  (rd, rs1, imm_i) */ break;
                case 0x6: /* ORI   (rd, rs1, imm_i) */ break;
                case 0x7: /* ANDI  (rd, rs1, imm_i) */ break;

                case 0x1: // SLLI
                    // RV32I: imm[11:5] must be 0x00
                    if (imm_i_11_5 == 0x00) { /* SLLI (rd, rs1, shamt) */ }
                    else illegal();
                    break;

                case 0x5: // SRLI / SRAI
                    // RV32I: imm[11:5] = 0x00 -> SRLI, = 0x20 -> SRAI
                    if (imm_i_11_5 == 0x00) { /* SRLI (rd, rs1, shamt) */ }
                    else if (imm_i_11_5 == 0x20) { /* SRAI (rd, rs1, shamt) */ }
                    else illegal();
                    break;

                default:
                    illegal();
                    break;
            }
            break;
        }

        // =========================================================
        // LOAD (I-Type): LB, LH, LW, LBU, LHU
        // =========================================================
        case 0b0000011: {
            switch (funct3) {
                case 0x0: /* LB  (rd, rs1, imm_i) */ break;
                case 0x1: /* LH  (rd, rs1, imm_i) */ break;
                case 0x2: /* LW  (rd, rs1, imm_i) */ break;
                case 0x4: /* LBU (rd, rs1, imm_i) */ break;
                case 0x5: /* LHU (rd, rs1, imm_i) */ break;
                default:  illegal(); break;
            }
            break;
        }

        // =========================================================
        // STORE (S-Type): SB, SH, SW
        // =========================================================
        case 0b0100011: {
            switch (funct3) {
                case 0x0: /* SB (rs2, rs1, imm_s) */ break;
                case 0x1: /* SH (rs2, rs1, imm_s) */ break;
                case 0x2: /* SW (rs2, rs1, imm_s) */ break;
                default:  illegal(); break;
            }
            break;
        }

        // =========================================================
        // BRANCH (B-Type): BEQ, BNE, BLT, BGE, BLTU, BGEU
        // =========================================================
        case 0b1100011: {
            switch (funct3) {
                case 0x0: /* BEQ  (rs1, rs2, imm_b) */ break;
                case 0x1: /* BNE  (rs1, rs2, imm_b) */ break;
                case 0x4: /* BLT  (rs1, rs2, imm_b) */ break;
                case 0x5: /* BGE  (rs1, rs2, imm_b) */ break;
                case 0x6: /* BLTU (rs1, rs2, imm_b) */ break;
                case 0x7: /* BGEU (rs1, rs2, imm_b) */ break;
                default:  illegal(); break;
            }
            break;
        }

        // =========================================================
        // JAL (J-Type)
        // =========================================================
        case 0b1101111: {
            /* JAL (rd, imm_j) */
            break;
        }

        // =========================================================
        // JALR (I-Type)
        // =========================================================
        case 0b1100111: {
            if (funct3 == 0x0) {
                /* JALR (rd, rs1, imm_i) */
            } else {
                illegal();
            }
            break;
        }

        // =========================================================
        // LUI / AUIPC (U-Type)
        // =========================================================
        case 0b0110111: {
            /* LUI (rd, imm_u) */
            break;
        }

        case 0b0010111: {
            /* AUIPC (rd, imm_u) */
            break;
        }

        // =========================================================
        // SYSTEM (I-Type): ECALL / EBREAK (CSR später)
        // =========================================================
        case 0b1110011: {
            if (funct3 == 0x0) {
                // imm_i enthält hier die 12-bit "SYSTEM immediate"
                if (imm_i == 0x0) { /* ECALL  */ }
                else if (imm_i == 0x1) { /* EBREAK */ }
                else illegal();
            } else {
                // CSR instructions kommen hier rein (CSRRW/CSRRS/CSRRC + immediate variants)
                illegal(); // vorerst, bis du CSR implementierst
            }
            break;
        }

        // =========================================================
        // FENCE (auch RV32I)
        // =========================================================
        case 0b0001111: {
            // FENCE / FENCE.I sind hier (funct3 unterscheidet)
            // Viele Simulatoren behandeln das als NOP, aber decoden solltest du es können.
            // funct3: 0 -> FENCE, 1 -> FENCE.I
            if (funct3 == 0x0) { /* FENCE */ }
            else if (funct3 == 0x1) { /* FENCE.I */ }
            else illegal();
            break;
        }

        default:
            illegal();
            break;
    }
}