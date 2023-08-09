/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/ifetch.h>

#define _R(i) gpr(i)
#define _CR(i) cgpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
    TYPE_I,
    TYPE_SI,
    TYPE_U,
    TYPE_S,
    TYPE_R,
    TYPE_B,
    TYPE_J,
    TYPE_N,// none
};

#define src1R()          \
    do {                 \
        *src1 = _R(rs1); \
    } while (0)
#define src2R()          \
    do {                 \
        *src2 = _R(rs2); \
    } while (0)
#define immI()                            \
    do {                                  \
        *imm = SEXT(BITS(i, 31, 20), 12); \
    } while (0)
#define immSI()                          \
    do {                                 \
        *imm = SEXT(BITS(i, 24, 20), 5); \
    } while (0)
#define immU()                                  \
    do {                                        \
        *imm = SEXT(BITS(i, 31, 12), 20) << 12; \
    } while (0)
#define immS()                                                   \
    do {                                                         \
        *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); \
    } while (0)
#define immJ()                                                                                         \
    do {                                                                                               \
        uint64_t tmp = SEXT(BITS(i, 31, 0), 32);                                                       \
        *imm = (SEXT(BITS(tmp, 63, 52), 12) << 20) | (BITS(i, 19, 12) << 12) | (BITS(i, 30, 21) << 1); \
        if (((i & 0x00100000) >> 20) == 1) {                                                           \
            *imm |= 2048;                                                                              \
        }                                                                                              \
    } while (0)

#define immB()                                                                                        \
    do {                                                                                              \
        uint64_t tmp = SEXT(BITS(i, 31, 0), 32);                                                      \
        *imm |= (SEXT(BITS(tmp, 63, 44), 20) << 12) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1); \
        if (((i & 0x00000080) >> 7) == 1)                                                             \
            *imm |= 0x00000800;                                                                       \
    } while (0)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
    uint32_t i = s->isa.inst.val;
    int rs1 = BITS(i, 19, 15);
    int rs2 = BITS(i, 24, 20);
    *rd = BITS(i, 11, 7);
    switch (type) {
        case TYPE_I:
            src1R();
            immI();
            break;
        case TYPE_SI:
            src1R();
            immSI();
            break;
        case TYPE_U:
            immU();
            break;
        case TYPE_S:
            src1R();
            src2R();
            immS();
            break;
        case TYPE_J:
            immJ();
            break;
        case TYPE_B:
            src1R();
            src2R();
            immB();
            break;
        case TYPE_R:
            src1R();
            src2R();
            break;
    }
}

static int decode_exec(Decode *s) {
    int rd = 0;
    word_t src1 = 0, src2 = 0, imm = 0;
    s->dnpc = s->snpc;
#define STR(n) (#n)
#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_NAME(s) (((Decode *) (s))->name)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */)             \
    {                                                                    \
        decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
        strcpy(INSTPAT_NAME(s), STR(name));                              \
        __VA_ARGS__;                                                     \
    }

    INSTPAT_START();
    INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal, J, _R(rd) = s->snpc; s->dnpc = imm + s->pc);
    INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui, U, _R(rd) = imm);
    INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc, U, _R(rd) = imm + s->pc);
    INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi, I, _R(rd) = imm + src1);
    INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi, I, _R(rd) = imm & src1);
    INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori, I, _R(rd) = imm ^ src1);
    INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori, I, _R(rd) = imm | src1);
    INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw, I, _R(rd) = Mr(src1 + imm, 4));
    INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh, I, _R(rd) = SEXT(Mr(src1 + imm, 2), 16));
    INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu, I, _R(rd) = Mr(src1 + imm, 2));
    INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu, I, _R(rd) = Mr(src1 + imm, 1));
    INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb, I, _R(rd) = SEXT(Mr(src1 + imm, 1), 8));
    INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr, I, _R(rd) = s->snpc; s->dnpc = src1 + imm);
    INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu, I, _R(rd) = 0; if (src1 < imm) _R(rd) = 1);
    INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti, I, _R(rd) = 0; if ((int) src1 < (int) imm) _R(rd) = 1);
    INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw, I, _R(rd) = _CR(imm); _CR(imm) = src1);
    INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall, I, s->dnpc = isa_raise_intr(0, 0));
    INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw, S, Mw(src1 + imm, 4, src2));
    INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb, S, Mw(src1 + imm, 1, src2));
    INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh, S, Mw(src1 + imm, 2, src2));
    INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq, B, if (src1 == src2) s->dnpc = imm + s->pc);
    INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge, B, if ((signed) src1 >= (signed) src2) s->dnpc = imm + s->pc);
    INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu, B, if (src1 >= src2) s->dnpc = imm + s->pc);
    INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu, B, if (src1 < src2) s->dnpc = imm + s->pc);
    INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt, B, if ((signed) src1 < (signed) src2) s->dnpc = imm + s->pc);
    INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne, B, if (src1 != src2) s->dnpc = imm + s->pc);
    INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem, R, _R(rd) = src1 % src2);
    INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu, R, _R(rd) = src1 % src2);
    INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor, R, _R(rd) = src1 ^ src2);
    INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu, R, _R(rd) = 0; if (src1 < src2) _R(rd) = 1);
    INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt, R, _R(rd) = 0; if ((signed) src1 < (signed) src2) _R(rd) = 1);
    INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or, R, _R(rd) = src1 | src2);
    INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and, R, _R(rd) = src1 & src2);
    INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add, R, _R(rd) = src1 + src2);
    INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div, R, _R(rd) = src1 / src2);
    INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu, R, _R(rd) = src1 / src2);
    INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul, R, _R(rd) = src1 * src2);
    INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu, R, uint64_t a1 = 0, a2 = 0;
            a1 |= src1; a2 |= src2; _R(rd) = BITS((uint64_t) (a1 * a2), 63, 32));
    INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh, R, uint64_t s1 = SEXT(src1, 32); uint64_t s2 = SEXT(src2, 32); _R(rd) = BITS((uint64_t) (s1 * s2), 63, 32));
    INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub, R, _R(rd) = src1 - src2);
    INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra, R, _R(rd) = (signed) src1 >> src2);
    INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl, R, _R(rd) = src1 >> src2);
    INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll, R, _R(rd) = src1 << src2);
    INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli, SI, _R(rd) = src1 << imm);
    INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli, SI, _R(rd) = src1 >> imm);
    INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai, SI, _R(rd) = (signed) src1 >> imm);
    INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak, N, NEMUTRAP(s->pc, _R(10)));// R(10) is $a0
    INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv, N, INV(s->pc));
    INSTPAT_END();

    _R(0) = 0;// reset $zero to 0

    return 0;
}

int isa_exec_once(Decode *s) {
    s->isa.inst.val = inst_fetch(&s->snpc, 4);
    return decode_exec(s);
}
