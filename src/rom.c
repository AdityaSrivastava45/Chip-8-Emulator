/**
 * @file rom.c
 * @brief ROM file loader — reads a binary image into program memory at 0x200.
 */

#include <stdio.h>
#include "rom.h"

/* ── rom_load ─────────────────────────────────────────────────── */

Chip8Result rom_load(Chip8 *vm, const char *path, size_t *bytes_loaded)
{
    if (!vm || !path) return CHIP8_ERR_NULL_PTR;

    FILE *fp = fopen(path, "rb");
    if (!fp) return CHIP8_ERR_ROM_NOT_FOUND;

    /* ── Determine file size ──────────────────────────────────── */
    if (fseek(fp, 0L, SEEK_END) != 0) {
        fclose(fp);
        return CHIP8_ERR_ROM_READ_FAIL;
    }

    long raw_size = ftell(fp);
    if (raw_size < 0L) {
        fclose(fp);
        return CHIP8_ERR_ROM_READ_FAIL;
    }

    size_t file_size = (size_t)raw_size;

    if (file_size > CHIP8_MAX_ROM_SIZE) {
        fclose(fp);
        return CHIP8_ERR_ROM_TOO_LARGE;
    }

    rewind(fp);

    /* ── Load directly into program memory ───────────────────── */
    size_t n = fread(&vm->memory[CHIP8_PROGRAM_START], 1u, file_size, fp);
    fclose(fp);

    if (n != file_size) return CHIP8_ERR_ROM_READ_FAIL;

    if (bytes_loaded) *bytes_loaded = n;
    return CHIP8_OK;
}
