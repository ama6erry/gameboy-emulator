#include "utils.h"
#include "hardware.h"
#include "rom.h"
#include "CPU.h"

int main(int argc, char **argv){
    char *fileDest = "test_roms/tetris.gb";
    
    init_cpu();
    load_rom(fileDest);

    while(1){
        cpu_step();
    }
}