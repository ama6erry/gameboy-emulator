#pragma once

typedef unsigned char BYTE;
typedef char SIGNED_BYTE;
typedef unsigned short WORD;
typedef short SIGNED_WORD;

//flag masks
#define FLAG_Z 0b10000000
#define FLAG_N 0b01000000
#define FLAG_H 0b00100000
#define FLAG_C 0b00010000

#define BLACK 0b000000001
#define DARK_GREY 0b00000010
#define LIGHT_GREY 0b00000100
#define WHITE 0b00001000

union Register{
    WORD full;
    struct 
    {
        BYTE lo;
        BYTE hi;
    };
};

extern BYTE Cartridge[0x8000];






