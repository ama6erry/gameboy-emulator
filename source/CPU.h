#include "hardware.h"

void cpu_step();
void init_cpu();

extern union Register register_AF;
extern union Register register_BC;
extern union Register register_DE;
extern union Register register_HL;
extern union Register sp;
extern WORD pc;