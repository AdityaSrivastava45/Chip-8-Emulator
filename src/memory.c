/**
 * @file memory.c
 * @brief CHIP-8 memory subsystem — initialisation, font loading, R/W ops.
 */

#include <string.h>
#include "memory.h"

/* ── Font data ────────────────────────────────────────────────── */
/*
 * Standard CHIP-8 font: 16 glyphs (0x0–0xF), each 4 pixels wide × 5 rows.
 * Each byte encodes one row; the high nibble is the pixel data
 * (low nibble is always 0 and never drawn).
 *
 * Loaded at CHIP8_FONT_START (0x050) during memory_init().
 */
const uint8_t CHIP8_FONT_DATA[CHIP8_NUM_GLYPHS * CHIP8_FONT_SPRITE_BYTES] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  /* 0 */
    0x20, 0x60, 0x20, 0x20, 0x70,  /* 1 */
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  /* 2 */
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  /* 3 */
    0x90, 0x90, 0xF0, 0x10, 0x10,  /* 4 */
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  /* 5 */
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  /* 6 */
    0xF0, 0x10, 0x20, 0x40, 0x40,  /* 7 */
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  /* 8 */
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  /* 9 */
    0xF0, 0x90, 0xF0, 0x90, 0x90,  /* A */
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  /* B */
    0xF0, 0x80, 0x80, 0x80, 0xF0,  /* C */
    0xE0, 0x90, 0x90, 0x90, 0xE0,  /* D */
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  /* E */
    0xF0, 0x80, 0xF0, 0x80, 0x80,  /* F */
};

/* ── memory_init ──────────────────────────────────────────────── */

Chip8Result memory_init(Chip8 *vm)
{
    if (!vm) return CHIP8_ERR_NULL_PTR;

    memset(vm->memory, 0, CHIP8_MEMORY_SIZE);
    memcpy(&vm->memory[CHIP8_FONT_START],
           CHIP8_FONT_DATA,
           sizeof(CHIP8_FONT_DATA));

    return CHIP8_OK;
}

/* ── Reads ────────────────────────────────────────────────────── */

Chip8Result memory_read8(const Chip8 *vm, uint16_t addr, uint8_t *out)
{
    if (!vm || !out)               return CHIP8_ERR_NULL_PTR;
    if (addr >= CHIP8_MEMORY_SIZE) return CHIP8_ERR_INVALID_ADDR;

    *out = vm->memory[addr];
    return CHIP8_OK;
}

Chip8Result memory_read16(const Chip8 *vm, uint16_t addr, uint16_t *out)
{
    if (!vm || !out)                         return CHIP8_ERR_NULL_PTR;
    /* Need addr and addr+1 both in range. */
    if ((uint32_t)addr + 1u >= CHIP8_MEMORY_SIZE) return CHIP8_ERR_INVALID_ADDR;

    *out = (uint16_t)(((uint16_t)vm->memory[addr]     << 8u) |
                       (uint16_t)vm->memory[addr + 1u]);
    return CHIP8_OK;
}

/* ── Writes ───────────────────────────────────────────────────── */

Chip8Result memory_write8(Chip8 *vm, uint16_t addr, uint8_t value)
{
    if (!vm)                       return CHIP8_ERR_NULL_PTR;
    if (addr >= CHIP8_MEMORY_SIZE) return CHIP8_ERR_INVALID_ADDR;

    vm->memory[addr] = value;
    return CHIP8_OK;
}

Chip8Result memory_write16(Chip8 *vm, uint16_t addr, uint16_t value)
{
    if (!vm)                                        return CHIP8_ERR_NULL_PTR;
    if ((uint32_t)addr + 1u >= CHIP8_MEMORY_SIZE)  return CHIP8_ERR_INVALID_ADDR;

    vm->memory[addr]      = (uint8_t)(value >> 8u);
    vm->memory[addr + 1u] = (uint8_t)(value & 0xFFu);
    return CHIP8_OK;
}
