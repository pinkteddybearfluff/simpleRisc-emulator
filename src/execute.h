#ifndef SIMPRISC_EXECUTE_H
#define SIMPRISC_EXECUTE_H

#include "../src/cpu.h"
#include "../src/memory.h"
#include "../src/decode.h"

bool execute(CPU* cpu, Memory* mem, DecodedInstr instr);

#endif //SIMPRISC_EXECUTE_H
