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
    //std::cout << "single step" << std::endl;
    fetch();
    std::cout << decode() << std::endl;
    writePC(readPC()+1);
}

void RiscVCore::fetch(){
    instruction = mem_.readMem(program_mem_base_offset + pc);
}

std::string RiscVCore::decode() {
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

    // OP-IMM shift fields (SLLI/SRLI/SRAI)
    uint32_t shamt      = bits(instruction, 24, 20);  // imm[4:0]
    uint32_t imm_i_11_5 = bits(instruction, 31, 25);  // imm[11:5]

    auto illegal = [&]() {
        // Reset decoded_ and mark instruction as illegal
        decoded_ = DecodedInstruction{};
        decoded_.op = Op::ILLEGAL;
        decoded_.raw = instruction;
    };

    // Reset decoded_ state before decoding
    decoded_.op = Op::ILLEGAL;
    decoded_.rd = decoded_.rs1 = decoded_.rs2 = 0;
    decoded_.imm = 0;
    decoded_.imm_kind = ImmKind::None;
    decoded_.raw = instruction;

    switch (opcode) {

        // =========================================================
        // OP (R-Type): ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND
        // =========================================================
        case 0b0110011: {
            switch (funct3) {
                case 0x0: // ADD/SUB
                    if (funct7 == 0x00) { // ADD
                        decoded_.op = Op::ADD;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.rs2 = rs2;
                        decoded_.imm_kind = ImmKind::None;
                    }
                    else if (funct7 == 0x20) { // SUB
                        decoded_.op = Op::SUB;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.rs2 = rs2;
                        decoded_.imm_kind = ImmKind::None;
                    }
                    else illegal();
                    break;

                case 0x1: // SLL
                    if (funct7 == 0x00) {
                        decoded_.op = Op::SLL;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.rs2 = rs2;
                        decoded_.imm_kind = ImmKind::None;
                    } else illegal();
                    break;

                case 0x2: // SLT
                    if (funct7 == 0x00) {
                        decoded_.op = Op::SLT;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.rs2 = rs2;
                        decoded_.imm_kind = ImmKind::None;
                    } else illegal();
                    break;

                case 0x3: // SLTU
                    if (funct7 == 0x00) {
                        decoded_.op = Op::SLTU;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.rs2 = rs2;
                        decoded_.imm_kind = ImmKind::None;
                    } else illegal();
                    break;

                case 0x4: // XOR
                    if (funct7 == 0x00) {
                        decoded_.op = Op::XOR;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.rs2 = rs2;
                        decoded_.imm_kind = ImmKind::None;
                    } else illegal();
                    break;

                case 0x5: // SRL/SRA
                    if (funct7 == 0x00) { // SRL
                        decoded_.op = Op::SRL;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.rs2 = rs2;
                        decoded_.imm_kind = ImmKind::None;
                    }
                    else if (funct7 == 0x20) { // SRA
                        decoded_.op = Op::SRA;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.rs2 = rs2;
                        decoded_.imm_kind = ImmKind::None;
                    }
                    else illegal();
                    break;

                case 0x6: // OR
                    if (funct7 == 0x00) {
                        decoded_.op = Op::OR;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.rs2 = rs2;
                        decoded_.imm_kind = ImmKind::None;
                    } else illegal();
                    break;

                case 0x7: // AND
                    if (funct7 == 0x00) {
                        decoded_.op = Op::AND;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.rs2 = rs2;
                        decoded_.imm_kind = ImmKind::None;
                    } else illegal();
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
                case 0x0: // ADDI (rd, rs1, imm_i)
                    decoded_.op = Op::ADDI;
                    decoded_.rd = rd;
                    decoded_.rs1 = rs1;
                    decoded_.imm = imm_i;
                    decoded_.imm_kind = ImmKind::I;
                    break;

                case 0x2: // SLTI (rd, rs1, imm_i)
                    decoded_.op = Op::SLTI;
                    decoded_.rd = rd;
                    decoded_.rs1 = rs1;
                    decoded_.imm = imm_i;
                    decoded_.imm_kind = ImmKind::I;
                    break;

                case 0x3: // SLTIU (rd, rs1, imm_i)
                    decoded_.op = Op::SLTIU;
                    decoded_.rd = rd;
                    decoded_.rs1 = rs1;
                    decoded_.imm = imm_i;
                    decoded_.imm_kind = ImmKind::I;
                    break;

                case 0x4: // XORI (rd, rs1, imm_i)
                    decoded_.op = Op::XORI;
                    decoded_.rd = rd;
                    decoded_.rs1 = rs1;
                    decoded_.imm = imm_i;
                    decoded_.imm_kind = ImmKind::I;
                    break;

                case 0x6: // ORI (rd, rs1, imm_i)
                    decoded_.op = Op::ORI;
                    decoded_.rd = rd;
                    decoded_.rs1 = rs1;
                    decoded_.imm = imm_i;
                    decoded_.imm_kind = ImmKind::I;
                    break;

                case 0x7: // ANDI (rd, rs1, imm_i)
                    decoded_.op = Op::ANDI;
                    decoded_.rd = rd;
                    decoded_.rs1 = rs1;
                    decoded_.imm = imm_i;
                    decoded_.imm_kind = ImmKind::I;
                    break;

                case 0x1: // SLLI
                    // RV32I: imm[11:5] must be 0x00
                    if (imm_i_11_5 == 0x00) {
                        decoded_.op = Op::SLLI;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.imm = (int32_t)shamt; // For shifts, imm stores shamt
                        decoded_.imm_kind = ImmKind::I;
                    }
                    else illegal();
                    break;

                case 0x5: // SRLI / SRAI
                    // RV32I: imm[11:5] = 0x00 -> SRLI, = 0x20 -> SRAI
                    if (imm_i_11_5 == 0x00) {
                        decoded_.op = Op::SRLI;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.imm = (int32_t)shamt; // For shifts, imm stores shamt
                        decoded_.imm_kind = ImmKind::I;
                    }
                    else if (imm_i_11_5 == 0x20) {
                        decoded_.op = Op::SRAI;
                        decoded_.rd = rd;
                        decoded_.rs1 = rs1;
                        decoded_.imm = (int32_t)shamt; // For shifts, imm stores shamt
                        decoded_.imm_kind = ImmKind::I;
                    }
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
                case 0x0: // LB (rd, rs1, imm_i)
                    decoded_.op = Op::LB;
                    decoded_.rd = rd;
                    decoded_.rs1 = rs1;
                    decoded_.imm = imm_i;
                    decoded_.imm_kind = ImmKind::I;
                    break;

                case 0x1: // LH (rd, rs1, imm_i)
                    decoded_.op = Op::LH;
                    decoded_.rd = rd;
                    decoded_.rs1 = rs1;
                    decoded_.imm = imm_i;
                    decoded_.imm_kind = ImmKind::I;
                    break;

                case 0x2: // LW (rd, rs1, imm_i)
                    decoded_.op = Op::LW;
                    decoded_.rd = rd;
                    decoded_.rs1 = rs1;
                    decoded_.imm = imm_i;
                    decoded_.imm_kind = ImmKind::I;
                    break;

                case 0x4: // LBU (rd, rs1, imm_i)
                    decoded_.op = Op::LBU;
                    decoded_.rd = rd;
                    decoded_.rs1 = rs1;
                    decoded_.imm = imm_i;
                    decoded_.imm_kind = ImmKind::I;
                    break;

                case 0x5: // LHU (rd, rs1, imm_i)
                    decoded_.op = Op::LHU;
                    decoded_.rd = rd;
                    decoded_.rs1 = rs1;
                    decoded_.imm = imm_i;
                    decoded_.imm_kind = ImmKind::I;
                    break;

                default:
                    illegal();
                    break;
            }
            break;
        }

        // =========================================================
        // STORE (S-Type): SB, SH, SW
        // =========================================================
        case 0b0100011: {
            switch (funct3) {
                case 0x0: // SB (rs2, rs1, imm_s)
                    decoded_.op = Op::SB;
                    decoded_.rs1 = rs1;
                    decoded_.rs2 = rs2;
                    decoded_.imm = imm_s;
                    decoded_.imm_kind = ImmKind::S;
                    break;

                case 0x1: // SH (rs2, rs1, imm_s)
                    decoded_.op = Op::SH;
                    decoded_.rs1 = rs1;
                    decoded_.rs2 = rs2;
                    decoded_.imm = imm_s;
                    decoded_.imm_kind = ImmKind::S;
                    break;

                case 0x2: // SW (rs2, rs1, imm_s)
                    decoded_.op = Op::SW;
                    decoded_.rs1 = rs1;
                    decoded_.rs2 = rs2;
                    decoded_.imm = imm_s;
                    decoded_.imm_kind = ImmKind::S;
                    break;

                default:
                    illegal();
                    break;
            }
            break;
        }

        // =========================================================
        // BRANCH (B-Type): BEQ, BNE, BLT, BGE, BLTU, BGEU
        // =========================================================
        case 0b1100011: {
            switch (funct3) {
                case 0x0: // BEQ (rs1, rs2, imm_b)
                    decoded_.op = Op::BEQ;
                    decoded_.rs1 = rs1;
                    decoded_.rs2 = rs2;
                    decoded_.imm = imm_b;
                    decoded_.imm_kind = ImmKind::B;
                    break;

                case 0x1: // BNE (rs1, rs2, imm_b)
                    decoded_.op = Op::BNE;
                    decoded_.rs1 = rs1;
                    decoded_.rs2 = rs2;
                    decoded_.imm = imm_b;
                    decoded_.imm_kind = ImmKind::B;
                    break;

                case 0x4: // BLT (rs1, rs2, imm_b)
                    decoded_.op = Op::BLT;
                    decoded_.rs1 = rs1;
                    decoded_.rs2 = rs2;
                    decoded_.imm = imm_b;
                    decoded_.imm_kind = ImmKind::B;
                    break;

                case 0x5: // BGE (rs1, rs2, imm_b)
                    decoded_.op = Op::BGE;
                    decoded_.rs1 = rs1;
                    decoded_.rs2 = rs2;
                    decoded_.imm = imm_b;
                    decoded_.imm_kind = ImmKind::B;
                    break;

                case 0x6: // BLTU (rs1, rs2, imm_b)
                    decoded_.op = Op::BLTU;
                    decoded_.rs1 = rs1;
                    decoded_.rs2 = rs2;
                    decoded_.imm = imm_b;
                    decoded_.imm_kind = ImmKind::B;
                    break;

                case 0x7: // BGEU (rs1, rs2, imm_b)
                    decoded_.op = Op::BGEU;
                    decoded_.rs1 = rs1;
                    decoded_.rs2 = rs2;
                    decoded_.imm = imm_b;
                    decoded_.imm_kind = ImmKind::B;
                    break;

                default:
                    illegal();
                    break;
            }
            break;
        }

        // =========================================================
        // JAL (J-Type)
        // =========================================================
        case 0b1101111: {
            // JAL (rd, imm_j)
            decoded_.op = Op::JAL;
            decoded_.rd = rd;
            decoded_.imm = imm_j;
            decoded_.imm_kind = ImmKind::J;
            break;
        }

        // =========================================================
        // JALR (I-Type)
        // =========================================================
        case 0b1100111: {
            if (funct3 == 0x0) {
                // JALR (rd, rs1, imm_i)
                decoded_.op = Op::JALR;
                decoded_.rd = rd;
                decoded_.rs1 = rs1;
                decoded_.imm = imm_i;
                decoded_.imm_kind = ImmKind::I;
            } else {
                illegal();
            }
            break;
        }

        // =========================================================
        // LUI / AUIPC (U-Type)
        // =========================================================
        case 0b0110111: {
            // LUI (rd, imm_u)
            decoded_.op = Op::LUI;
            decoded_.rd = rd;
            decoded_.imm = imm_u;
            decoded_.imm_kind = ImmKind::U;
            break;
        }

        case 0b0010111: {
            // AUIPC (rd, imm_u)
            decoded_.op = Op::AUIPC;
            decoded_.rd = rd;
            decoded_.imm = imm_u;
            decoded_.imm_kind = ImmKind::U;
            break;
        }

        // =========================================================
        // SYSTEM (I-Type): ECALL / EBREAK (CSR later)
        // =========================================================
        case 0b1110011: {
            if (funct3 == 0x0) {
                // imm_i holds the 12-bit SYSTEM immediate
                if (imm_i == 0x0) { // ECALL
                    decoded_.op = Op::ECALL;
                    decoded_.imm_kind = ImmKind::None;
                }
                else if (imm_i == 0x1) { // EBREAK
                    decoded_.op = Op::EBREAK;
                    decoded_.imm_kind = ImmKind::None;
                }
                else illegal();
            } else {
                // CSR instructions go here (CSRRW/CSRRS/CSRRC + immediate variants)
                illegal(); // not implemented yet
            }
            break;
        }

        // =========================================================
        // FENCE (part of RV32I)
        // =========================================================
        case 0b0001111: {
            // FENCE / FENCE.I are encoded here (distinguished by funct3)
            // funct3: 0 -> FENCE, 1 -> FENCE.I
            if (funct3 == 0x0) { // FENCE
                decoded_.op = Op::FENCE;
            }
            else if (funct3 == 0x1) { // FENCE.I
                decoded_.op = Op::FENCE_I;
            }
            else illegal();

            decoded_.imm_kind = ImmKind::None;
            break;
        }

        default:
            illegal();
            break;
    }

    return to_string(decoded_);
}