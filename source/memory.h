#include "hardware.h"

void load_rom(const char *name);
BYTE read_byte(WORD address);
void write_byte(WORD address, BYTE value);

extern BYTE Cartridge[0x8000];