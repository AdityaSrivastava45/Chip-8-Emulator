/**
 * @file cpu.h
 * @brief CHIP-8 CPU — fetch, decode, and execute pipeline.
 *
 * Implemented in Milestone 2.  Forward-declared here so other subsystems
 * can reference the types without a compile dependency on the full CPU.
 */

#ifndef CHIP8_CPU_H
#define CHIP8_CPU_H

#include "chip8.h"

/* ── Execution cycle stages ───────────────────────────────────── */

/**
 * @brief Fetch the next 16-bit opcode from memory[pc] and advance pc by 2.
 *
 * Reads two consecutive bytes in big-endian order and stores the result
 * in vm->opcode.
 *
 * @return CHIP8_OK or CHIP8_ERR_INVALID_ADDR if pc is out of range.
 */
Chip8Result cpu_fetch(Chip8 *vm);

/**
 * @brief Decode vm->opcode and dispatch to the appropriate handler.
 *
 * All 35 standard CHIP-8 instructions are handled here.
 *
 * @return CHIP8_OK or CHIP8_ERR_UNKNOWN_OPCODE / stack errors.
 */
Chip8Result cpu_decode_execute(Chip8 *vm);

#endif /* CHIP8_CPU_H */
