#include "hardware.h"
#include "utils.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

union Register register_AF;
union Register register_BC;
union Register register_DE;
union Register register_HL;
union Register sp;
WORD pc;

#define SET_FLAG(x) (register_AF.lo |= (x))
#define CLEAR_FLAG(x) (register_AF.lo &= ~(x))

BYTE screen[160][144];

struct Instruction {
    char *mnemonic;
    BYTE bytes;
    BYTE cycles;
    bool immediate;
    void *execute;
};

void not_implemented() {
    LOG_E("This instruction is not yet implemented");
}

BYTE inc(BYTE value){
    if((value & 0x0f) == 0x0f) SET_FLAG(FLAG_H);
    else CLEAR_FLAG(FLAG_H);

    value += 1;

    if(value != 0) CLEAR_FLAG(FLAG_Z);
    else SET_FLAG(FLAG_Z);

    CLEAR_FLAG(FLAG_N);

    return value;
}

BYTE dec(BYTE value){
    if(value & 0x0f) CLEAR_FLAG(FLAG_H);
    else SET_FLAG(FLAG_H);

    value -= 1;

    if(value) CLEAR_FLAG(FLAG_Z);
    else SET_FLAG(FLAG_Z);

    SET_FLAG(FLAG_N);

    return value;
}

// 0x00
void nop(){
    return;
}

// 0x02
void ld_bc_a(){ write_byte(register_BC.full, register_AF.hi); }

// 0x05
void dec_b(){ register_BC.hi = dec(register_BC.hi); }

// 0x06
void ld_b_n8(BYTE operand) { register_BC.hi = operand; pc += 1; }

// 0x0C
void inc_c(){ register_BC.lo = inc(register_BC.lo); }

// 0x0E
void ld_c_n8(BYTE operand){ register_BC.lo = operand; pc += 1; }


// 0x20
void jr_nz_e8(BYTE operand){
    pc += 1;
    BYTE z = register_AF.lo & FLAG_Z;
    if(!z){
        SIGNED_BYTE newOp = (SIGNED_BYTE) operand;
        pc += newOp;
    }
}

// 0x21
void ld_hl_n16(WORD operand){ register_HL.full = operand; pc += 2; }

// 0x32
void ld_hld_a() { write_byte(register_HL.full, register_AF.hi); register_HL.full -= 1; }

// 0xAF
void xor_a_a(){ 
    register_AF.hi = register_AF.hi ^ register_AF.hi; 
    if (register_AF.hi == 0) SET_FLAG(FLAG_Z); 
    CLEAR_FLAG(FLAG_C);
    CLEAR_FLAG(FLAG_H);
    CLEAR_FLAG(FLAG_N);
}

// 0xC3
void jp_a16(WORD operand){ pc = operand; }

struct Instruction unprefixed_instructions[256] = {
    [0x00] = {"NOP", 1, 4, true, nop},
    [0x01] = {"LD BC,n16", 3, 12, true, not_implemented},
    [0x02] = {"LD BC,A", 1, 8, false, ld_bc_a},
    [0x03] = {"INC BC", 1, 8, true, not_implemented},
    [0x04] = {"INC B", 1, 4, true, not_implemented},
    [0x05] = {"DEC B", 1, 4, true, dec_b},
    [0x06] = {"LD B,n8", 2, 8, true, ld_b_n8},
    [0x07] = {"RLCA", 1, 4, true, not_implemented},
    [0x08] = {"LD a16,SP", 3, 20, false, not_implemented},
    [0x09] = {"ADD HL,BC", 1, 8, true, not_implemented},
    [0x0A] = {"LD A,BC", 1, 8, false, not_implemented},
    [0x0B] = {"DEC BC", 1, 8, true, not_implemented},
    [0x0C] = {"INC C", 1, 4, true, inc_c},
    [0x0D] = {"DEC C", 1, 4, true, not_implemented},
    [0x0E] = {"LD C,n8", 2, 8, true, ld_c_n8},
    [0x0F] = {"RRCA", 1, 4, true, not_implemented},
    [0x10] = {"STOP n8", 2, 4, true, not_implemented},
    [0x11] = {"LD DE,n16", 3, 12, true, not_implemented},
    [0x12] = {"LD DE,A", 1, 8, false, not_implemented},
    [0x13] = {"INC DE", 1, 8, true, not_implemented},
    [0x14] = {"INC D", 1, 4, true, not_implemented},
    [0x15] = {"DEC D", 1, 4, true, not_implemented},
    [0x16] = {"LD D,n8", 2, 8, true, not_implemented},
    [0x17] = {"RLA", 1, 4, true, not_implemented},
    [0x18] = {"JR e8", 2, 12, true, not_implemented},
    [0x19] = {"ADD HL,DE", 1, 8, true, not_implemented},
    [0x1A] = {"LD A,DE", 1, 8, false, not_implemented},
    [0x1B] = {"DEC DE", 1, 8, true, not_implemented},
    [0x1C] = {"INC E", 1, 4, true, not_implemented},
    [0x1D] = {"DEC E", 1, 4, true, not_implemented},
    [0x1E] = {"LD E,n8", 2, 8, true, not_implemented},
    [0x1F] = {"RRA", 1, 4, true, not_implemented},
    [0x20] = {"JR NZ,e8", 2, 12, true, jr_nz_e8},
    [0x21] = {"LD HL,n16", 3, 12, true, ld_hl_n16},
    [0x22] = {"LD HL,A", 1, 8, false, not_implemented},
    [0x23] = {"INC HL", 1, 8, true, not_implemented},
    [0x24] = {"INC H", 1, 4, true, not_implemented},
    [0x25] = {"DEC H", 1, 4, true, not_implemented},
    [0x26] = {"LD H,n8", 2, 8, true, not_implemented},
    [0x27] = {"DAA", 1, 4, true, not_implemented},
    [0x28] = {"JR Z,e8", 2, 12, true, not_implemented},
    [0x29] = {"ADD HL,HL", 1, 8, true, not_implemented},
    [0x2A] = {"LD A,HL", 1, 8, false, not_implemented},
    [0x2B] = {"DEC HL", 1, 8, true, not_implemented},
    [0x2C] = {"INC L", 1, 4, true, not_implemented},
    [0x2D] = {"DEC L", 1, 4, true, not_implemented},
    [0x2E] = {"LD L,n8", 2, 8, true, not_implemented},
    [0x2F] = {"CPL", 1, 4, true, not_implemented},
    [0x30] = {"JR NC,e8", 2, 12, true, not_implemented},
    [0x31] = {"LD SP,n16", 3, 12, true, not_implemented},
    [0x32] = {"LD HLD,A", 1, 8, false, ld_hld_a},
    [0x33] = {"INC SP", 1, 8, true, not_implemented},
    [0x34] = {"INC HL", 1, 12, false, not_implemented},
    [0x35] = {"DEC HL", 1, 12, false, not_implemented},
    [0x36] = {"LD HL,n8", 2, 12, false, not_implemented},
    [0x37] = {"SCF", 1, 4, true, not_implemented},
    [0x38] = {"JR C,e8", 2, 12, true, not_implemented},
    [0x39] = {"ADD HL,SP", 1, 8, true, not_implemented},
    [0x3A] = {"LD A,HL", 1, 8, false, not_implemented},
    [0x3B] = {"DEC SP", 1, 8, true, not_implemented},
    [0x3C] = {"INC A", 1, 4, true, not_implemented},
    [0x3D] = {"DEC A", 1, 4, true, not_implemented},
    [0x3E] = {"LD A,n8", 2, 8, true, not_implemented},
    [0x3F] = {"CCF", 1, 4, true, not_implemented},
    [0x40] = {"LD B,B", 1, 4, true, not_implemented},
    [0x41] = {"LD B,C", 1, 4, true, not_implemented},
    [0x42] = {"LD B,D", 1, 4, true, not_implemented},
    [0x43] = {"LD B,E", 1, 4, true, not_implemented},
    [0x44] = {"LD B,H", 1, 4, true, not_implemented},
    [0x45] = {"LD B,L", 1, 4, true, not_implemented},
    [0x46] = {"LD B,HL", 1, 8, false, not_implemented},
    [0x47] = {"LD B,A", 1, 4, true, not_implemented},
    [0x48] = {"LD C,B", 1, 4, true, not_implemented},
    [0x49] = {"LD C,C", 1, 4, true, not_implemented},
    [0x4A] = {"LD C,D", 1, 4, true, not_implemented},
    [0x4B] = {"LD C,E", 1, 4, true, not_implemented},
    [0x4C] = {"LD C,H", 1, 4, true, not_implemented},
    [0x4D] = {"LD C,L", 1, 4, true, not_implemented},
    [0x4E] = {"LD C,HL", 1, 8, false, not_implemented},
    [0x4F] = {"LD C,A", 1, 4, true, not_implemented},
    [0x50] = {"LD D,B", 1, 4, true, not_implemented},
    [0x51] = {"LD D,C", 1, 4, true, not_implemented},
    [0x52] = {"LD D,D", 1, 4, true, not_implemented},
    [0x53] = {"LD D,E", 1, 4, true, not_implemented},
    [0x54] = {"LD D,H", 1, 4, true, not_implemented},
    [0x55] = {"LD D,L", 1, 4, true, not_implemented},
    [0x56] = {"LD D,HL", 1, 8, false, not_implemented},
    [0x57] = {"LD D,A", 1, 4, true, not_implemented},
    [0x58] = {"LD E,B", 1, 4, true, not_implemented},
    [0x59] = {"LD E,C", 1, 4, true, not_implemented},
    [0x5A] = {"LD E,D", 1, 4, true, not_implemented},
    [0x5B] = {"LD E,E", 1, 4, true, not_implemented},
    [0x5C] = {"LD E,H", 1, 4, true, not_implemented},
    [0x5D] = {"LD E,L", 1, 4, true, not_implemented},
    [0x5E] = {"LD E,HL", 1, 8, false, not_implemented},
    [0x5F] = {"LD E,A", 1, 4, true, not_implemented},
    [0x60] = {"LD H,B", 1, 4, true, not_implemented},
    [0x61] = {"LD H,C", 1, 4, true, not_implemented},
    [0x62] = {"LD H,D", 1, 4, true, not_implemented},
    [0x63] = {"LD H,E", 1, 4, true, not_implemented},
    [0x64] = {"LD H,H", 1, 4, true, not_implemented},
    [0x65] = {"LD H,L", 1, 4, true, not_implemented},
    [0x66] = {"LD H,HL", 1, 8, false, not_implemented},
    [0x67] = {"LD H,A", 1, 4, true, not_implemented},
    [0x68] = {"LD L,B", 1, 4, true, not_implemented},
    [0x69] = {"LD L,C", 1, 4, true, not_implemented},
    [0x6A] = {"LD L,D", 1, 4, true, not_implemented},
    [0x6B] = {"LD L,E", 1, 4, true, not_implemented},
    [0x6C] = {"LD L,H", 1, 4, true, not_implemented},
    [0x6D] = {"LD L,L", 1, 4, true, not_implemented},
    [0x6E] = {"LD L,HL", 1, 8, false, not_implemented},
    [0x6F] = {"LD L,A", 1, 4, true, not_implemented},
    [0x70] = {"LD HL,B", 1, 8, false, not_implemented},
    [0x71] = {"LD HL,C", 1, 8, false, not_implemented},
    [0x72] = {"LD HL,D", 1, 8, false, not_implemented},
    [0x73] = {"LD HL,E", 1, 8, false, not_implemented},
    [0x74] = {"LD HL,H", 1, 8, false, not_implemented},
    [0x75] = {"LD HL,L", 1, 8, false, not_implemented},
    [0x76] = {"HALT", 1, 4, true, not_implemented},
    [0x77] = {"LD HL,A", 1, 8, false, not_implemented},
    [0x78] = {"LD A,B", 1, 4, true, not_implemented},
    [0x79] = {"LD A,C", 1, 4, true, not_implemented},
    [0x7A] = {"LD A,D", 1, 4, true, not_implemented},
    [0x7B] = {"LD A,E", 1, 4, true, not_implemented},
    [0x7C] = {"LD A,H", 1, 4, true, not_implemented},
    [0x7D] = {"LD A,L", 1, 4, true, not_implemented},
    [0x7E] = {"LD A,HL", 1, 8, false, not_implemented},
    [0x7F] = {"LD A,A", 1, 4, true, not_implemented},
    [0x80] = {"ADD A,B", 1, 4, true, not_implemented},
    [0x81] = {"ADD A,C", 1, 4, true, not_implemented},
    [0x82] = {"ADD A,D", 1, 4, true, not_implemented},
    [0x83] = {"ADD A,E", 1, 4, true, not_implemented},
    [0x84] = {"ADD A,H", 1, 4, true, not_implemented},
    [0x85] = {"ADD A,L", 1, 4, true, not_implemented},
    [0x86] = {"ADD A,HL", 1, 8, false, not_implemented},
    [0x87] = {"ADD A,A", 1, 4, true, not_implemented},
    [0x88] = {"ADC A,B", 1, 4, true, not_implemented},
    [0x89] = {"ADC A,C", 1, 4, true, not_implemented},
    [0x8A] = {"ADC A,D", 1, 4, true, not_implemented},
    [0x8B] = {"ADC A,E", 1, 4, true, not_implemented},
    [0x8C] = {"ADC A,H", 1, 4, true, not_implemented},
    [0x8D] = {"ADC A,L", 1, 4, true, not_implemented},
    [0x8E] = {"ADC A,HL", 1, 8, false, not_implemented},
    [0x8F] = {"ADC A,A", 1, 4, true, not_implemented},
    [0x90] = {"SUB A,B", 1, 4, true, not_implemented},
    [0x91] = {"SUB A,C", 1, 4, true, not_implemented},
    [0x92] = {"SUB A,D", 1, 4, true, not_implemented},
    [0x93] = {"SUB A,E", 1, 4, true, not_implemented},
    [0x94] = {"SUB A,H", 1, 4, true, not_implemented},
    [0x95] = {"SUB A,L", 1, 4, true, not_implemented},
    [0x96] = {"SUB A,HL", 1, 8, false, not_implemented},
    [0x97] = {"SUB A,A", 1, 4, true, not_implemented},
    [0x98] = {"SBC A,B", 1, 4, true, not_implemented},
    [0x99] = {"SBC A,C", 1, 4, true, not_implemented},
    [0x9A] = {"SBC A,D", 1, 4, true, not_implemented},
    [0x9B] = {"SBC A,E", 1, 4, true, not_implemented},
    [0x9C] = {"SBC A,H", 1, 4, true, not_implemented},
    [0x9D] = {"SBC A,L", 1, 4, true, not_implemented},
    [0x9E] = {"SBC A,HL", 1, 8, false, not_implemented},
    [0x9F] = {"SBC A,A", 1, 4, true, not_implemented},
    [0xA0] = {"AND A,B", 1, 4, true, not_implemented},
    [0xA1] = {"AND A,C", 1, 4, true, not_implemented},
    [0xA2] = {"AND A,D", 1, 4, true, not_implemented},
    [0xA3] = {"AND A,E", 1, 4, true, not_implemented},
    [0xA4] = {"AND A,H", 1, 4, true, not_implemented},
    [0xA5] = {"AND A,L", 1, 4, true, not_implemented},
    [0xA6] = {"AND A,HL", 1, 8, false, not_implemented},
    [0xA7] = {"AND A,A", 1, 4, true, not_implemented},
    [0xA8] = {"XOR A,B", 1, 4, true, not_implemented},
    [0xA9] = {"XOR A,C", 1, 4, true, not_implemented},
    [0xAA] = {"XOR A,D", 1, 4, true, not_implemented},
    [0xAB] = {"XOR A,E", 1, 4, true, not_implemented},
    [0xAC] = {"XOR A,H", 1, 4, true, not_implemented},
    [0xAD] = {"XOR A,L", 1, 4, true, not_implemented},
    [0xAE] = {"XOR A,HL", 1, 8, false, not_implemented},
    [0xAF] = {"XOR A,A", 1, 4, true, xor_a_a},
    [0xB0] = {"OR A,B", 1, 4, true, not_implemented},
    [0xB1] = {"OR A,C", 1, 4, true, not_implemented},
    [0xB2] = {"OR A,D", 1, 4, true, not_implemented},
    [0xB3] = {"OR A,E", 1, 4, true, not_implemented},
    [0xB4] = {"OR A,H", 1, 4, true, not_implemented},
    [0xB5] = {"OR A,L", 1, 4, true, not_implemented},
    [0xB6] = {"OR A,HL", 1, 8, false, not_implemented},
    [0xB7] = {"OR A,A", 1, 4, true, not_implemented},
    [0xB8] = {"CP A,B", 1, 4, true, not_implemented},
    [0xB9] = {"CP A,C", 1, 4, true, not_implemented},
    [0xBA] = {"CP A,D", 1, 4, true, not_implemented},
    [0xBB] = {"CP A,E", 1, 4, true, not_implemented},
    [0xBC] = {"CP A,H", 1, 4, true, not_implemented},
    [0xBD] = {"CP A,L", 1, 4, true, not_implemented},
    [0xBE] = {"CP A,HL", 1, 8, false, not_implemented},
    [0xBF] = {"CP A,A", 1, 4, true, not_implemented},
    [0xC0] = {"RET NZ", 1, 20, true, not_implemented},
    [0xC1] = {"POP BC", 1, 12, true, not_implemented},
    [0xC2] = {"JP NZ,a16", 3, 16, true, not_implemented},
    [0xC3] = {"JP a16", 3, 16, true, jp_a16},
    [0xC4] = {"CALL NZ,a16", 3, 24, true, not_implemented},
    [0xC5] = {"PUSH BC", 1, 16, true, not_implemented},
    [0xC6] = {"ADD A,n8", 2, 8, true, not_implemented},
    [0xC7] = {"RST $00", 1, 16, true, not_implemented},
    [0xC8] = {"RET Z", 1, 20, true, not_implemented},
    [0xC9] = {"RET", 1, 16, true, not_implemented},
    [0xCA] = {"JP Z,a16", 3, 16, true, not_implemented},
    [0xCB] = {"PREFIX", 1, 4, true, not_implemented},
    [0xCC] = {"CALL Z,a16", 3, 24, true, not_implemented},
    [0xCD] = {"CALL a16", 3, 24, true, not_implemented},
    [0xCE] = {"ADC A,n8", 2, 8, true, not_implemented},
    [0xCF] = {"RST $08", 1, 16, true, not_implemented},
    [0xD0] = {"RET NC", 1, 20, true, not_implemented},
    [0xD1] = {"POP DE", 1, 12, true, not_implemented},
    [0xD2] = {"JP NC,a16", 3, 16, true, not_implemented},
    [0xD3] = {"ILLEGAL_D3", 1, 4, true, not_implemented},
    [0xD4] = {"CALL NC,a16", 3, 24, true, not_implemented},
    [0xD5] = {"PUSH DE", 1, 16, true, not_implemented},
    [0xD6] = {"SUB A,n8", 2, 8, true, not_implemented},
    [0xD7] = {"RST $10", 1, 16, true, not_implemented},
    [0xD8] = {"RET C", 1, 20, true, not_implemented},
    [0xD9] = {"RETI", 1, 16, true, not_implemented},
    [0xDA] = {"JP C,a16", 3, 16, true, not_implemented},
    [0xDB] = {"ILLEGAL_DB", 1, 4, true, not_implemented},
    [0xDC] = {"CALL C,a16", 3, 24, true, not_implemented},
    [0xDD] = {"ILLEGAL_DD", 1, 4, true, not_implemented},
    [0xDE] = {"SBC A,n8", 2, 8, true, not_implemented},
    [0xDF] = {"RST $18", 1, 16, true, not_implemented},
    [0xE0] = {"LDH a8,A", 2, 12, false, not_implemented},
    [0xE1] = {"POP HL", 1, 12, true, not_implemented},
    [0xE2] = {"LDH C,A", 1, 8, false, not_implemented},
    [0xE3] = {"ILLEGAL_E3", 1, 4, true, not_implemented},
    [0xE4] = {"ILLEGAL_E4", 1, 4, true, not_implemented},
    [0xE5] = {"PUSH HL", 1, 16, true, not_implemented},
    [0xE6] = {"AND A,n8", 2, 8, true, not_implemented},
    [0xE7] = {"RST $20", 1, 16, true, not_implemented},
    [0xE8] = {"ADD SP,e8", 2, 16, true, not_implemented},
    [0xE9] = {"JP HL", 1, 4, true, not_implemented},
    [0xEA] = {"LD a16,A", 3, 16, false, not_implemented},
    [0xEB] = {"ILLEGAL_EB", 1, 4, true, not_implemented},
    [0xEC] = {"ILLEGAL_EC", 1, 4, true, not_implemented},
    [0xED] = {"ILLEGAL_ED", 1, 4, true, not_implemented},
    [0xEE] = {"XOR A,n8", 2, 8, true, not_implemented},
    [0xEF] = {"RST $28", 1, 16, true, not_implemented},
    [0xF0] = {"LDH A,a8", 2, 12, false, not_implemented},
    [0xF1] = {"POP AF", 1, 12, true, not_implemented},
    [0xF2] = {"LDH A,C", 1, 8, false, not_implemented},
    [0xF3] = {"DI", 1, 4, true, not_implemented},
    [0xF4] = {"ILLEGAL_F4", 1, 4, true, not_implemented},
    [0xF5] = {"PUSH AF", 1, 16, true, not_implemented},
    [0xF6] = {"OR A,n8", 2, 8, true, not_implemented},
    [0xF7] = {"RST $30", 1, 16, true, not_implemented},
    [0xF8] = {"LD HL,SP", 2, 12, true, not_implemented},
    [0xF9] = {"LD SP,HL", 1, 8, true, not_implemented},
    [0xFA] = {"LD A,a16", 3, 16, false, not_implemented},
    [0xFB] = {"EI", 1, 4, true, not_implemented},
    [0xFC] = {"ILLEGAL_FC", 1, 4, true, not_implemented},
    [0xFD] = {"ILLEGAL_FD", 1, 4, true, not_implemented},
    [0xFE] = {"CP A,n8", 2, 8, true, not_implemented},
    [0xFF] = {"RST $38", 1, 16, true, not_implemented},
};

struct Instruction prefixed_instructions[256] = {
    [0x00] = {"RLC B", 2, 8, true, not_implemented},
    [0x01] = {"RLC C", 2, 8, true, not_implemented},
    [0x02] = {"RLC D", 2, 8, true, not_implemented},
    [0x03] = {"RLC E", 2, 8, true, not_implemented},
    [0x04] = {"RLC H", 2, 8, true, not_implemented},
    [0x05] = {"RLC L", 2, 8, true, not_implemented},
    [0x06] = {"RLC HL", 2, 16, false, not_implemented},
    [0x07] = {"RLC A", 2, 8, true, not_implemented},
    [0x08] = {"RRC B", 2, 8, true, not_implemented},
    [0x09] = {"RRC C", 2, 8, true, not_implemented},
    [0x0A] = {"RRC D", 2, 8, true, not_implemented},
    [0x0B] = {"RRC E", 2, 8, true, not_implemented},
    [0x0C] = {"RRC H", 2, 8, true, not_implemented},
    [0x0D] = {"RRC L", 2, 8, true, not_implemented},
    [0x0E] = {"RRC HL", 2, 16, false, not_implemented},
    [0x0F] = {"RRC A", 2, 8, true, not_implemented},
    [0x10] = {"RL B", 2, 8, true, not_implemented},
    [0x11] = {"RL C", 2, 8, true, not_implemented},
    [0x12] = {"RL D", 2, 8, true, not_implemented},
    [0x13] = {"RL E", 2, 8, true, not_implemented},
    [0x14] = {"RL H", 2, 8, true, not_implemented},
    [0x15] = {"RL L", 2, 8, true, not_implemented},
    [0x16] = {"RL HL", 2, 16, false, not_implemented},
    [0x17] = {"RL A", 2, 8, true, not_implemented},
    [0x18] = {"RR B", 2, 8, true, not_implemented},
    [0x19] = {"RR C", 2, 8, true, not_implemented},
    [0x1A] = {"RR D", 2, 8, true, not_implemented},
    [0x1B] = {"RR E", 2, 8, true, not_implemented},
    [0x1C] = {"RR H", 2, 8, true, not_implemented},
    [0x1D] = {"RR L", 2, 8, true, not_implemented},
    [0x1E] = {"RR HL", 2, 16, false, not_implemented},
    [0x1F] = {"RR A", 2, 8, true, not_implemented},
    [0x20] = {"SLA B", 2, 8, true, not_implemented},
    [0x21] = {"SLA C", 2, 8, true, not_implemented},
    [0x22] = {"SLA D", 2, 8, true, not_implemented},
    [0x23] = {"SLA E", 2, 8, true, not_implemented},
    [0x24] = {"SLA H", 2, 8, true, not_implemented},
    [0x25] = {"SLA L", 2, 8, true, not_implemented},
    [0x26] = {"SLA HL", 2, 16, false, not_implemented},
    [0x27] = {"SLA A", 2, 8, true, not_implemented},
    [0x28] = {"SRA B", 2, 8, true, not_implemented},
    [0x29] = {"SRA C", 2, 8, true, not_implemented},
    [0x2A] = {"SRA D", 2, 8, true, not_implemented},
    [0x2B] = {"SRA E", 2, 8, true, not_implemented},
    [0x2C] = {"SRA H", 2, 8, true, not_implemented},
    [0x2D] = {"SRA L", 2, 8, true, not_implemented},
    [0x2E] = {"SRA HL", 2, 16, false, not_implemented},
    [0x2F] = {"SRA A", 2, 8, true, not_implemented},
    [0x30] = {"SWAP B", 2, 8, true, not_implemented},
    [0x31] = {"SWAP C", 2, 8, true, not_implemented},
    [0x32] = {"SWAP D", 2, 8, true, not_implemented},
    [0x33] = {"SWAP E", 2, 8, true, not_implemented},
    [0x34] = {"SWAP H", 2, 8, true, not_implemented},
    [0x35] = {"SWAP L", 2, 8, true, not_implemented},
    [0x36] = {"SWAP HL", 2, 16, false, not_implemented},
    [0x37] = {"SWAP A", 2, 8, true, not_implemented},
    [0x38] = {"SRL B", 2, 8, true, not_implemented},
    [0x39] = {"SRL C", 2, 8, true, not_implemented},
    [0x3A] = {"SRL D", 2, 8, true, not_implemented},
    [0x3B] = {"SRL E", 2, 8, true, not_implemented},
    [0x3C] = {"SRL H", 2, 8, true, not_implemented},
    [0x3D] = {"SRL L", 2, 8, true, not_implemented},
    [0x3E] = {"SRL HL", 2, 16, false, not_implemented},
    [0x3F] = {"SRL A", 2, 8, true, not_implemented},
    [0x40] = {"BIT 0,B", 2, 8, true, not_implemented},
    [0x41] = {"BIT 0,C", 2, 8, true, not_implemented},
    [0x42] = {"BIT 0,D", 2, 8, true, not_implemented},
    [0x43] = {"BIT 0,E", 2, 8, true, not_implemented},
    [0x44] = {"BIT 0,H", 2, 8, true, not_implemented},
    [0x45] = {"BIT 0,L", 2, 8, true, not_implemented},
    [0x46] = {"BIT 0,HL", 2, 12, false, not_implemented},
    [0x47] = {"BIT 0,A", 2, 8, true, not_implemented},
    [0x48] = {"BIT 1,B", 2, 8, true, not_implemented},
    [0x49] = {"BIT 1,C", 2, 8, true, not_implemented},
    [0x4A] = {"BIT 1,D", 2, 8, true, not_implemented},
    [0x4B] = {"BIT 1,E", 2, 8, true, not_implemented},
    [0x4C] = {"BIT 1,H", 2, 8, true, not_implemented},
    [0x4D] = {"BIT 1,L", 2, 8, true, not_implemented},
    [0x4E] = {"BIT 1,HL", 2, 12, false, not_implemented},
    [0x4F] = {"BIT 1,A", 2, 8, true, not_implemented},
    [0x50] = {"BIT 2,B", 2, 8, true, not_implemented},
    [0x51] = {"BIT 2,C", 2, 8, true, not_implemented},
    [0x52] = {"BIT 2,D", 2, 8, true, not_implemented},
    [0x53] = {"BIT 2,E", 2, 8, true, not_implemented},
    [0x54] = {"BIT 2,H", 2, 8, true, not_implemented},
    [0x55] = {"BIT 2,L", 2, 8, true, not_implemented},
    [0x56] = {"BIT 2,HL", 2, 12, false, not_implemented},
    [0x57] = {"BIT 2,A", 2, 8, true, not_implemented},
    [0x58] = {"BIT 3,B", 2, 8, true, not_implemented},
    [0x59] = {"BIT 3,C", 2, 8, true, not_implemented},
    [0x5A] = {"BIT 3,D", 2, 8, true, not_implemented},
    [0x5B] = {"BIT 3,E", 2, 8, true, not_implemented},
    [0x5C] = {"BIT 3,H", 2, 8, true, not_implemented},
    [0x5D] = {"BIT 3,L", 2, 8, true, not_implemented},
    [0x5E] = {"BIT 3,HL", 2, 12, false, not_implemented},
    [0x5F] = {"BIT 3,A", 2, 8, true, not_implemented},
    [0x60] = {"BIT 4,B", 2, 8, true, not_implemented},
    [0x61] = {"BIT 4,C", 2, 8, true, not_implemented},
    [0x62] = {"BIT 4,D", 2, 8, true, not_implemented},
    [0x63] = {"BIT 4,E", 2, 8, true, not_implemented},
    [0x64] = {"BIT 4,H", 2, 8, true, not_implemented},
    [0x65] = {"BIT 4,L", 2, 8, true, not_implemented},
    [0x66] = {"BIT 4,HL", 2, 12, false, not_implemented},
    [0x67] = {"BIT 4,A", 2, 8, true, not_implemented},
    [0x68] = {"BIT 5,B", 2, 8, true, not_implemented},
    [0x69] = {"BIT 5,C", 2, 8, true, not_implemented},
    [0x6A] = {"BIT 5,D", 2, 8, true, not_implemented},
    [0x6B] = {"BIT 5,E", 2, 8, true, not_implemented},
    [0x6C] = {"BIT 5,H", 2, 8, true, not_implemented},
    [0x6D] = {"BIT 5,L", 2, 8, true, not_implemented},
    [0x6E] = {"BIT 5,HL", 2, 12, false, not_implemented},
    [0x6F] = {"BIT 5,A", 2, 8, true, not_implemented},
    [0x70] = {"BIT 6,B", 2, 8, true, not_implemented},
    [0x71] = {"BIT 6,C", 2, 8, true, not_implemented},
    [0x72] = {"BIT 6,D", 2, 8, true, not_implemented},
    [0x73] = {"BIT 6,E", 2, 8, true, not_implemented},
    [0x74] = {"BIT 6,H", 2, 8, true, not_implemented},
    [0x75] = {"BIT 6,L", 2, 8, true, not_implemented},
    [0x76] = {"BIT 6,HL", 2, 12, false, not_implemented},
    [0x77] = {"BIT 6,A", 2, 8, true, not_implemented},
    [0x78] = {"BIT 7,B", 2, 8, true, not_implemented},
    [0x79] = {"BIT 7,C", 2, 8, true, not_implemented},
    [0x7A] = {"BIT 7,D", 2, 8, true, not_implemented},
    [0x7B] = {"BIT 7,E", 2, 8, true, not_implemented},
    [0x7C] = {"BIT 7,H", 2, 8, true, not_implemented},
    [0x7D] = {"BIT 7,L", 2, 8, true, not_implemented},
    [0x7E] = {"BIT 7,HL", 2, 12, false, not_implemented},
    [0x7F] = {"BIT 7,A", 2, 8, true, not_implemented},
    [0x80] = {"RES 0,B", 2, 8, true, not_implemented},
    [0x81] = {"RES 0,C", 2, 8, true, not_implemented},
    [0x82] = {"RES 0,D", 2, 8, true, not_implemented},
    [0x83] = {"RES 0,E", 2, 8, true, not_implemented},
    [0x84] = {"RES 0,H", 2, 8, true, not_implemented},
    [0x85] = {"RES 0,L", 2, 8, true, not_implemented},
    [0x86] = {"RES 0,HL", 2, 16, false, not_implemented},
    [0x87] = {"RES 0,A", 2, 8, true, not_implemented},
    [0x88] = {"RES 1,B", 2, 8, true, not_implemented},
    [0x89] = {"RES 1,C", 2, 8, true, not_implemented},
    [0x8A] = {"RES 1,D", 2, 8, true, not_implemented},
    [0x8B] = {"RES 1,E", 2, 8, true, not_implemented},
    [0x8C] = {"RES 1,H", 2, 8, true, not_implemented},
    [0x8D] = {"RES 1,L", 2, 8, true, not_implemented},
    [0x8E] = {"RES 1,HL", 2, 16, false, not_implemented},
    [0x8F] = {"RES 1,A", 2, 8, true, not_implemented},
    [0x90] = {"RES 2,B", 2, 8, true, not_implemented},
    [0x91] = {"RES 2,C", 2, 8, true, not_implemented},
    [0x92] = {"RES 2,D", 2, 8, true, not_implemented},
    [0x93] = {"RES 2,E", 2, 8, true, not_implemented},
    [0x94] = {"RES 2,H", 2, 8, true, not_implemented},
    [0x95] = {"RES 2,L", 2, 8, true, not_implemented},
    [0x96] = {"RES 2,HL", 2, 16, false, not_implemented},
    [0x97] = {"RES 2,A", 2, 8, true, not_implemented},
    [0x98] = {"RES 3,B", 2, 8, true, not_implemented},
    [0x99] = {"RES 3,C", 2, 8, true, not_implemented},
    [0x9A] = {"RES 3,D", 2, 8, true, not_implemented},
    [0x9B] = {"RES 3,E", 2, 8, true, not_implemented},
    [0x9C] = {"RES 3,H", 2, 8, true, not_implemented},
    [0x9D] = {"RES 3,L", 2, 8, true, not_implemented},
    [0x9E] = {"RES 3,HL", 2, 16, false, not_implemented},
    [0x9F] = {"RES 3,A", 2, 8, true, not_implemented},
    [0xA0] = {"RES 4,B", 2, 8, true, not_implemented},
    [0xA1] = {"RES 4,C", 2, 8, true, not_implemented},
    [0xA2] = {"RES 4,D", 2, 8, true, not_implemented},
    [0xA3] = {"RES 4,E", 2, 8, true, not_implemented},
    [0xA4] = {"RES 4,H", 2, 8, true, not_implemented},
    [0xA5] = {"RES 4,L", 2, 8, true, not_implemented},
    [0xA6] = {"RES 4,HL", 2, 16, false, not_implemented},
    [0xA7] = {"RES 4,A", 2, 8, true, not_implemented},
    [0xA8] = {"RES 5,B", 2, 8, true, not_implemented},
    [0xA9] = {"RES 5,C", 2, 8, true, not_implemented},
    [0xAA] = {"RES 5,D", 2, 8, true, not_implemented},
    [0xAB] = {"RES 5,E", 2, 8, true, not_implemented},
    [0xAC] = {"RES 5,H", 2, 8, true, not_implemented},
    [0xAD] = {"RES 5,L", 2, 8, true, not_implemented},
    [0xAE] = {"RES 5,HL", 2, 16, false, not_implemented},
    [0xAF] = {"RES 5,A", 2, 8, true, not_implemented},
    [0xB0] = {"RES 6,B", 2, 8, true, not_implemented},
    [0xB1] = {"RES 6,C", 2, 8, true, not_implemented},
    [0xB2] = {"RES 6,D", 2, 8, true, not_implemented},
    [0xB3] = {"RES 6,E", 2, 8, true, not_implemented},
    [0xB4] = {"RES 6,H", 2, 8, true, not_implemented},
    [0xB5] = {"RES 6,L", 2, 8, true, not_implemented},
    [0xB6] = {"RES 6,HL", 2, 16, false, not_implemented},
    [0xB7] = {"RES 6,A", 2, 8, true, not_implemented},
    [0xB8] = {"RES 7,B", 2, 8, true, not_implemented},
    [0xB9] = {"RES 7,C", 2, 8, true, not_implemented},
    [0xBA] = {"RES 7,D", 2, 8, true, not_implemented},
    [0xBB] = {"RES 7,E", 2, 8, true, not_implemented},
    [0xBC] = {"RES 7,H", 2, 8, true, not_implemented},
    [0xBD] = {"RES 7,L", 2, 8, true, not_implemented},
    [0xBE] = {"RES 7,HL", 2, 16, false, not_implemented},
    [0xBF] = {"RES 7,A", 2, 8, true, not_implemented},
    [0xC0] = {"SET 0,B", 2, 8, true, not_implemented},
    [0xC1] = {"SET 0,C", 2, 8, true, not_implemented},
    [0xC2] = {"SET 0,D", 2, 8, true, not_implemented},
    [0xC3] = {"SET 0,E", 2, 8, true, not_implemented},
    [0xC4] = {"SET 0,H", 2, 8, true, not_implemented},
    [0xC5] = {"SET 0,L", 2, 8, true, not_implemented},
    [0xC6] = {"SET 0,HL", 2, 16, false, not_implemented},
    [0xC7] = {"SET 0,A", 2, 8, true, not_implemented},
    [0xC8] = {"SET 1,B", 2, 8, true, not_implemented},
    [0xC9] = {"SET 1,C", 2, 8, true, not_implemented},
    [0xCA] = {"SET 1,D", 2, 8, true, not_implemented},
    [0xCB] = {"SET 1,E", 2, 8, true, not_implemented},
    [0xCC] = {"SET 1,H", 2, 8, true, not_implemented},
    [0xCD] = {"SET 1,L", 2, 8, true, not_implemented},
    [0xCE] = {"SET 1,HL", 2, 16, false, not_implemented},
    [0xCF] = {"SET 1,A", 2, 8, true, not_implemented},
    [0xD0] = {"SET 2,B", 2, 8, true, not_implemented},
    [0xD1] = {"SET 2,C", 2, 8, true, not_implemented},
    [0xD2] = {"SET 2,D", 2, 8, true, not_implemented},
    [0xD3] = {"SET 2,E", 2, 8, true, not_implemented},
    [0xD4] = {"SET 2,H", 2, 8, true, not_implemented},
    [0xD5] = {"SET 2,L", 2, 8, true, not_implemented},
    [0xD6] = {"SET 2,HL", 2, 16, false, not_implemented},
    [0xD7] = {"SET 2,A", 2, 8, true, not_implemented},
    [0xD8] = {"SET 3,B", 2, 8, true, not_implemented},
    [0xD9] = {"SET 3,C", 2, 8, true, not_implemented},
    [0xDA] = {"SET 3,D", 2, 8, true, not_implemented},
    [0xDB] = {"SET 3,E", 2, 8, true, not_implemented},
    [0xDC] = {"SET 3,H", 2, 8, true, not_implemented},
    [0xDD] = {"SET 3,L", 2, 8, true, not_implemented},
    [0xDE] = {"SET 3,HL", 2, 16, false, not_implemented},
    [0xDF] = {"SET 3,A", 2, 8, true, not_implemented},
    [0xE0] = {"SET 4,B", 2, 8, true, not_implemented},
    [0xE1] = {"SET 4,C", 2, 8, true, not_implemented},
    [0xE2] = {"SET 4,D", 2, 8, true, not_implemented},
    [0xE3] = {"SET 4,E", 2, 8, true, not_implemented},
    [0xE4] = {"SET 4,H", 2, 8, true, not_implemented},
    [0xE5] = {"SET 4,L", 2, 8, true, not_implemented},
    [0xE6] = {"SET 4,HL", 2, 16, false, not_implemented},
    [0xE7] = {"SET 4,A", 2, 8, true, not_implemented},
    [0xE8] = {"SET 5,B", 2, 8, true, not_implemented},
    [0xE9] = {"SET 5,C", 2, 8, true, not_implemented},
    [0xEA] = {"SET 5,D", 2, 8, true, not_implemented},
    [0xEB] = {"SET 5,E", 2, 8, true, not_implemented},
    [0xEC] = {"SET 5,H", 2, 8, true, not_implemented},
    [0xED] = {"SET 5,L", 2, 8, true, not_implemented},
    [0xEE] = {"SET 5,HL", 2, 16, false, not_implemented},
    [0xEF] = {"SET 5,A", 2, 8, true, not_implemented},
    [0xF0] = {"SET 6,B", 2, 8, true, not_implemented},
    [0xF1] = {"SET 6,C", 2, 8, true, not_implemented},
    [0xF2] = {"SET 6,D", 2, 8, true, not_implemented},
    [0xF3] = {"SET 6,E", 2, 8, true, not_implemented},
    [0xF4] = {"SET 6,H", 2, 8, true, not_implemented},
    [0xF5] = {"SET 6,L", 2, 8, true, not_implemented},
    [0xF6] = {"SET 6,HL", 2, 16, false, not_implemented},
    [0xF7] = {"SET 6,A", 2, 8, true, not_implemented},
    [0xF8] = {"SET 7,B", 2, 8, true, not_implemented},
    [0xF9] = {"SET 7,C", 2, 8, true, not_implemented},
    [0xFA] = {"SET 7,D", 2, 8, true, not_implemented},
    [0xFB] = {"SET 7,E", 2, 8, true, not_implemented},
    [0xFC] = {"SET 7,H", 2, 8, true, not_implemented},
    [0xFD] = {"SET 7,L", 2, 8, true, not_implemented},
    [0xFE] = {"SET 7,HL", 2, 16, false, not_implemented},
    [0xFF] = {"SET 7,A", 2, 8, true, not_implemented},
};

void init_cpu(){
    BYTE Cartridge[0x200000];

    register_AF.full = 0x01B0;
    register_BC.full = 0x0013;
    register_DE.full = 0x00D8;
    register_HL.full = 0x014D;

    sp.full = 0xFFFE;

    pc = 0x100;
 
    Cartridge[0xFF05] = 0x00 ;
    Cartridge[0xFF06] = 0x00 ;
    Cartridge[0xFF07] = 0x00 ;
    Cartridge[0xFF10] = 0x80 ;
    Cartridge[0xFF11] = 0xBF ;
    Cartridge[0xFF12] = 0xF3 ;
    Cartridge[0xFF14] = 0xBF ;
    Cartridge[0xFF16] = 0x3F ;
    Cartridge[0xFF17] = 0x00 ;
    Cartridge[0xFF19] = 0xBF ;
    Cartridge[0xFF1A] = 0x7F ;
    Cartridge[0xFF1B] = 0xFF ;
    Cartridge[0xFF1C] = 0x9F ;
    Cartridge[0xFF1E] = 0xBF ;
    Cartridge[0xFF20] = 0xFF ;
    Cartridge[0xFF21] = 0x00 ;
    Cartridge[0xFF22] = 0x00 ;
    Cartridge[0xFF23] = 0xBF ;
    Cartridge[0xFF24] = 0x77 ;
    Cartridge[0xFF25] = 0xF3 ;
    Cartridge[0xFF26] = 0xF1 ;
    Cartridge[0xFF40] = 0x91 ;
    Cartridge[0xFF42] = 0x00 ;
    Cartridge[0xFF43] = 0x00 ;
    Cartridge[0xFF45] = 0x00 ;
    Cartridge[0xFF47] = 0xFC ;
    Cartridge[0xFF48] = 0xFF ;
    Cartridge[0xFF49] = 0xFF ;
    Cartridge[0xFF4A] = 0x00 ;
    Cartridge[0xFF4B] = 0x00 ;
    Cartridge[0xFFFF] = 0x00 ; 
}

void cpu_step(){
    BYTE instruction = read_byte(pc);
    pc += 1;
    WORD operand = 0;
    int prefixed = 0;
    if (instruction == 0xCB) {
        prefixed = 1;
        pc += 1;
        instruction = read_byte(pc);
    }    
    


    if(!prefixed){
        if(unprefixed_instructions[instruction].bytes == 2){
            operand = read_byte(pc);
            LOG_I("[EXECUTING] $%04X | [0x%04X]  %s $%04X ", pc - 1, instruction, unprefixed_instructions[instruction].mnemonic, operand);
            ((void (*)(BYTE))unprefixed_instructions[instruction].execute)((BYTE) operand);
            return;
        }
        else if(unprefixed_instructions[instruction].bytes == 3){
            operand = read_byte(pc) | read_byte(pc + 1) << 8;
            LOG_I("[EXECUTING] $%04X | [0x%04X]  %s $%04X ", pc - 1, instruction, unprefixed_instructions[instruction].mnemonic, operand);
            ((void (*)(WORD))unprefixed_instructions[instruction].execute)(operand);
            return;
        }
        
        LOG_I("[EXECUTING] $%04X | [0x%04X]  %s ", pc - 1, instruction, unprefixed_instructions[instruction].mnemonic);
        ((void (*)(void))unprefixed_instructions[instruction].execute)();
        return;
        
    }

    if(prefixed){  
        operand = read_byte(pc);
        
        LOG_I("[EXECUTING] CB 0x%04X | %s", instruction, prefixed_instructions[instruction].mnemonic);
        ((void (*)(BYTE))prefixed_instructions[instruction].execute)(operand);
    }
    
    
}






