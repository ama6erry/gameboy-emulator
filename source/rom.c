#include "rom.h"
#include "hardware.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

BYTE rom[0x10000];

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
    fread(rom, 0x180, 1, file);

    char title[17];

    memset(title, '\0', 17); //Tile is max 16 characters

    for(int i = 0; i < 16; i++){
        if(rom[i + 0x134] == 0x80 || rom[0x134 + i] == 0xc0) title[i] = '\0';
        else title[i] = rom[i + 0x134];
    }

    LOG_I("ROM title : %s", title);
    fclose(file);
    return;
};