/**
 * @file memory.h
 * @brief Bounds-checked memory access and font initialisation.
 *
 * All reads and writes go through these functions so that out-of-range
 * accesses return CHIP8_ERR_INVALID_ADDR rather than causing UB.
 */

#ifndef CHIP8_MEMORY_H
#define CHIP8_MEMORY_H

#include <stdint.h>
#include "chip8.h"

/* ── Font data (public so main can query its size) ───────────── */
/**
 * Standard CHIP-8 font sprites: glyphs 0x0–0xF, 5 bytes each.
 * Loaded into memory[CHIP8_FONT_START .. CHIP8_FONT_END].
 */
extern const uint8_t CHIP8_FONT_DATA[CHIP8_NUM_GLYPHS * CHIP8_FONT_SPRITE_BYTES];

/* ── Initialisation ───────────────────────────────────────────── */

/**
 * @brief Zero memory and copy font sprites to 0x050.
 *
 * Called by chip8_init(); safe to call again on reset.
 */
Chip8Result memory_init(Chip8 *vm);

/* ── Reads ────────────────────────────────────────────────────── */

/**
 * @brief Read one byte from @p addr.
 * @param vm    Non-NULL VM state.
 * @param addr  Address in [0, CHIP8_MEMORY_SIZE).
 * @param out   Receives the byte value.
 */
Chip8Result memory_read8(const Chip8 *vm, uint16_t addr, uint8_t *out);

/**
 * @brief Read a big-endian 16-bit word from @p addr and @p addr+1.
 * @param vm    Non-NULL VM state.
 * @param addr  Address in [0, CHIP8_MEMORY_SIZE - 1).
 * @param out   Receives the 16-bit word (high byte first).
 */
Chip8Result memory_read16(const Chip8 *vm, uint16_t addr, uint16_t *out);

/* ── Writes ───────────────────────────────────────────────────── */

/**
 * @brief Write one byte to @p addr.
 */
Chip8Result memory_write8(Chip8 *vm, uint16_t addr, uint8_t value);

/**
 * @brief Write a big-endian 16-bit word to @p addr and @p addr+1.
 */
Chip8Result memory_write16(Chip8 *vm, uint16_t addr, uint16_t value);

#endif /* CHIP8_MEMORY_H */
