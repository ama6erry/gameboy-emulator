#include "memory.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

BYTE Cartridge[0x8000];
BYTE vram[0x9fff - 0x8000];
BYTE eram[0xbfff - 0xa000];
BYTE wram[0xdfff - 0xc000];
BYTE oam[0xfe9f - 0xfe00];
BYTE io[0xff7f - 0xff00];
BYTE hram[0xfffe - 0xff80];
BYTE ie;

void load_rom(const char *name){
    FILE* file;
    file = fopen(name, "rb");
    LOG_I("Opening ROM at : %s", name);


    if(file == NULL){
        LOG_E("ROM file not found");
        fclose(file);
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    if(length < 0x180){
        LOG_E("ROM file is too small to be a valid ROM");
        fclose(file);
        return;
    }

    rewind(file);
    fread(Cartridge, 0x8000, 1, file);

    char title[17];

    memset(title, '\0', 17); //Tile is max 17 characters

    for(int i = 0; i < 16; i++){
        if(Cartridge[i + 0x134] == 0x80 || Cartridge[0x134 + i] == 0xc0) title[i] = '\0';
        else title[i] = Cartridge[i + 0x134];
    }

    LOG_I("ROM title : %s", title);
    LOG_I("MBC MODE : %d", Cartridge[0x147]);
    fclose(file);
    return;
};

BYTE read_byte(WORD address){
    if(address < 0x8000){
        return Cartridge[address];
    }
    else if(address < 0xa000){
        return vram[address - 0x8000];
    } else if(address < 0xc000){
        return eram[address - 0xa000];
    } else if(address < 0xe000){
        return wram[address - 0xc000];
    } else if(address < 0xfea0){
        return oam[address - 0xfe00];
    } else if(address < 0xff80){
        return io[address - 0xff00];
    } else if(address <= 0xffff){
        return hram[address - 0xff80];
    }
}

void write_byte(WORD address, BYTE value){
    if(address >= 0xa000 && address <= 0xbfff){
        eram[address - 0xa000] = value;
    } else if(address >= 0x8000 && address <= 0x9fff){
        vram[address - 0x8000] = value;
    }

    if(address >= 0xc000 && address <= 0xdfff)
		wram[address - 0xc000] = value;
	
	else if(address >= 0xe000 && address <= 0xfdff)
		wram[address - 0xe000] = value;

    else if(address >= 0xfe00 && address <= 0xfeff)
		oam[address - 0xfe00] = value;
	
	else if(address >= 0xff80 && address <= 0xfffe)
		hram[address - 0xff80] = value;

    else {
        LOG_I("Address written to unsupported area : 0x%x", address);
        exit(0);
    }
}