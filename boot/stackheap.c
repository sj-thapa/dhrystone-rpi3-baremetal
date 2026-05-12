//--------------------------------------------------------------------------
// Stack and heap memory area for Raspberry Pi 3B+
//
// Creates the memory region used for stack and heap.
// heap_base: start of heap area (heap grows upward)
// stack_top: end of stack area (stack grows downward from here)
//--------------------------------------------------------------------------

// Stack definitions (must match stackheap.hs)
#define CPU_STACK_SIZE  4096
#define STACK_SIZE      (CPU_STACK_SIZE * 4)  // 16384
#define HEAP_SIZE       (4096 * 8)             // 32768

// Allocate the full stack+heap memory region in the "stackheap" section.
// This section is placed at 0x8000 by the linker (between .got.plt and .bss).
// The 4KB alignment matches the original .align 12 in stackheap.s.
__attribute__((section("stackheap"), aligned(4096)))
char stack_heap_area[HEAP_SIZE + STACK_SIZE];

// Export symbols for assembly code (init.s) and C code (retarget.c).
// heap_base points to the start of the memory region.
// stack_top points to the last 4 bytes of the region.
__asm__(".globl heap_base\n"
        "heap_base = stack_heap_area\n");
__asm__(".globl stack_top\n"
        "stack_top = stack_heap_area + 49148\n");  // HEAP_SIZE + STACK_SIZE - 4
