#include "hardware.h"
#include "CPU.h"

#define VBLANK 0b00000001
#define LCD 0b00000010
#define TIMER 0b00000100
#define SERIAL 0b00001000
#define JOYPAD 0b00010000


BYTE IE;
BYTE IF;

void handle_interrupt(){
    if ((IE & IF) != 0){
        if((IF & IE) & VBLANK){
            return;
        } else if ((IF & IE) & LCD){
            return;
        } else if ((IF & IE) & TIMER){
            return;
        } else if ((IF & IE) & SERIAL){
            return;
        } else if ((IF & IE) & JOYPAD){
            return;
        }
    } 
}