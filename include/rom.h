/**
 * @file rom.h
 * @brief ROM file loading into CHIP-8 program memory.
 *
 * ROMs are always loaded at address 0x200 (CHIP8_PROGRAM_START).
 * The maximum ROM size is CHIP8_MAX_ROM_SIZE (3584 bytes).
 */

#ifndef CHIP8_ROM_H
#define CHIP8_ROM_H

#include <stddef.h>
#include "chip8.h"

/**
 * @brief Load a ROM file into vm->memory starting at 0x200.
 *
 * @param vm           Non-NULL VM state (must be initialised first).
 * @param path         Null-terminated path to the ROM file.
 * @param bytes_loaded If non-NULL, receives the number of bytes written.
 *
 * @return CHIP8_OK on success, or:
 *         CHIP8_ERR_NULL_PTR      — vm or path is NULL
 *         CHIP8_ERR_ROM_NOT_FOUND — file could not be opened
 *         CHIP8_ERR_ROM_TOO_LARGE — file > CHIP8_MAX_ROM_SIZE
 *         CHIP8_ERR_ROM_READ_FAIL — fread returned wrong byte count
 */
Chip8Result rom_load(Chip8 *vm, const char *path, size_t *bytes_loaded);

#endif /* CHIP8_ROM_H */
