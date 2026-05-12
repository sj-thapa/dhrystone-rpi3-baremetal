target extended-remote :3333
load
set extended-prompt (gdb) 
set pagination off
set logging enabled on
set trace-commands on
layout asm
layout regs
