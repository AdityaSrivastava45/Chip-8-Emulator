/**
 * @file chip8.c
 * @brief CHIP-8 VM lifecycle — init, reset, and error string table.
 */

#include <string.h>
#include "chip8.h"
#include "memory.h"

/* ── chip8_init ───────────────────────────────────────────────── */

Chip8Result chip8_init(Chip8 *vm)
{
    if (!vm) return CHIP8_ERR_NULL_PTR;

    /*
     * Zero the entire struct first so that every field (V[], stack[],
     * keypad[], display[], I, sp, timers) starts at a known-good state
     * before subsystem init functions run.
     */
    memset(vm, 0, sizeof(Chip8));

    /* Populate the font region at 0x050–0x09F. */
    Chip8Result r = memory_init(vm);
    if (r != CHIP8_OK) return r;

    /* Programs always begin at 0x200. */
    vm->pc = CHIP8_PROGRAM_START;

    return CHIP8_OK;
}

/* ── chip8_reset ──────────────────────────────────────────────── */

void chip8_reset(Chip8 *vm)
{
    if (!vm) return;
    /*
     * Re-initialise the full state.  Any loaded ROM is wiped; the caller
     * must call rom_load() again after reset if re-running the same ROM.
     */
    chip8_init(vm);
}

/* ── chip8_result_str ─────────────────────────────────────────── */

const char *chip8_result_str(Chip8Result result)
{
    switch (result) {
        case CHIP8_OK:                return "OK";
        case CHIP8_ERR_NULL_PTR:      return "null pointer argument";
        case CHIP8_ERR_ROM_NOT_FOUND: return "ROM file not found";
        case CHIP8_ERR_ROM_TOO_LARGE: return "ROM file exceeds available program memory";
        case CHIP8_ERR_ROM_READ_FAIL: return "ROM read error (short read or seek failure)";
        case CHIP8_ERR_INVALID_ADDR:  return "memory address out of range [0, 4095]";
        case CHIP8_ERR_STACK_OVERFLOW:  return "call stack overflow (max depth 16)";
        case CHIP8_ERR_STACK_UNDERFLOW: return "call stack underflow (RET with empty stack)";
        case CHIP8_ERR_UNKNOWN_OPCODE:  return "unknown or unimplemented opcode";
        default:                      return "(unrecognised error code)";
    }
}
