# Dhrystone on Raspberry Pi 3B+ (AArch64 Bare-Metal)

Bare-metal AArch64 port of the [Dhrystone 2.1](https://en.wikipedia.org/wiki/Dhrystone) benchmark running on a Raspberry Pi 3B+ (Cortex-A53) at EL3, with MMU, caches, and NEON enabled.

## Results

| Metric | Value |
|--------|-------|
| Dhrystones/sec | **210,526.3** |
| Microseconds per run | 4.7 |
| Iterations | 100,000,000 |
| Compiler | GCC 13.2.1 (aarch64-none-elf) |
| Optimization | `-O1` (no inlining, no vectorization, no loop unrolling) |
| Register attribute | Not used |

## Project Structure

```
boot/           Boot initialization (entry point, stack setup, MMU/cache enable)
drivers/uart/   PL011 UART driver for serial output
mmu/            AArch64 page table definitions (EL3, 4KB granule)
lib/            C runtime startup, clock, and retarget stubs
src/            Dhrystone source (dhry_1.c, dhry_2.c)
include/        Headers and board definitions
linker/         Linker script
```

## Prerequisites

- `aarch64-none-elf` GCC toolchain (Arm GNU Toolchain)
- JTAG debugger (e.g., Adafruit FT232H) with OpenOCD for loading onto hardware
- USB-to-UART TTL serial adapter (e.g., CP2102) for reading UART output
- Raspberry Pi 3B+ with an SD card

## SD Card Setup

Copy the contents of `sdcard/` to a FAT32-formatted SD card and insert it into the Raspberry Pi 3B+:

```bash
cp sdcard/* /path/to/sdcard/
```

## Building

```bash
make build
```

To change the iteration count:

```bash
make build ITERATIONS=200000000
```

Clean build artifacts:

```bash
make clean
```

The output ELF is placed at `build/dhrystone.elf`.

## JTAG Wiring (FT232H to Raspberry Pi 3B+)

| Function | FT232H Pin | Pi Pin | Pi GPIO |
|----------|-----------|--------|---------|
| TCK | D0 | 22 | 25 |
| TDI | D1 | 37 | 26 |
| TDO | D2 | 18 | 24 |
| TMS | D3 | 13 | 27 |
| TRST | D4 | 15 | 22 |
| SRST | D5 | Not connected | Not connected |
| RTCK | D7 | 16 | 23 |
| GND | GND | 39 | GND |

## Running on Hardware

Connect via JTAG and use the provided GDB script:

```bash
aarch64-none-elf-gdb -x ConnectJTAG.gdb build/dhrystone.elf
```

Output is printed over UART (PL011 at the BCM2837 base address).

## License

Dhrystone 2.1 is in the public domain. The bare-metal platform code has no additional license restrictions.
