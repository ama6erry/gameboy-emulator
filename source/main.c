#include "utils.h"
#include "hardware.h"
#include "memory.h"
#include "CPU.h"
#include "display.h"
#include <SDL.h>

int running;
int awaiting;
int stepEnabled;

int main(int argc, char **argv){
    char *fileDest = "test_roms/tetris.gb";
    running = 1;
    awaiting = 1;
    stepEnabled = 0;
    SDL_KeyboardEvent key;
    
    init_cpu();
    load_rom(fileDest);
    init_debug_display();

    SDL_Event event;
    while(running){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            else if (event.type == SDL_KEYDOWN) {
                key = event.key;
                switch(key.keysym.scancode){
                    case SDL_SCANCODE_S:
                        stepEnabled = !stepEnabled;
                        break;
                    default:
                        break;
                }
            }
        }
        cpu_step();
        update_debug_window();
        if(stepEnabled){
            while(awaiting){
                if (SDL_PollEvent(&event)){
                    if(event.type == SDL_QUIT) {awaiting = 0; running = 0;}
                    else if (event.type == SDL_KEYDOWN) {awaiting = 0;}
                }
            }
            awaiting = 1;
        }
    }

    destroy_debug_display();
}