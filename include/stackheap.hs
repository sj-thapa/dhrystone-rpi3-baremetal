// Stack definitions. Each CPU gets a chunk of the global stack space.
.equ CPU_STACK_SIZE, 4096
.equ STACK_SIZE, CPU_STACK_SIZE*4
.equ HEAP_SIZE, 4096*8
