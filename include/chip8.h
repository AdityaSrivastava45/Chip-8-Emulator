/**
 * @file chip8.h
 * @brief CHIP-8 virtual machine state and public lifecycle API.
 *
 * This header is the single canonical definition of the VM state.
 * Every subsystem receives a pointer to `Chip8`; no global instances exist.
 */

#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H

#include <stdint.h>
#include <stdbool.h>

/* ── Compile-time constants ───────────────────────────────────── */
#define CHIP8_MEMORY_SIZE       4096u
#define CHIP8_DISPLAY_WIDTH       64u
#define CHIP8_DISPLAY_HEIGHT      32u
#define CHIP8_NUM_REGISTERS       16u
#define CHIP8_STACK_DEPTH         16u
#define CHIP8_NUM_KEYS            16u

#define CHIP8_FONT_START        0x050u
#define CHIP8_FONT_END          0x09Fu
#define CHIP8_FONT_SPRITE_BYTES    5u   /* each glyph is 5 bytes tall          */
#define CHIP8_NUM_GLYPHS          16u   /* 0–F                                  */

#define CHIP8_PROGRAM_START     0x200u
#define CHIP8_MAX_ROM_SIZE      (CHIP8_MEMORY_SIZE - CHIP8_PROGRAM_START)

/* ── Result codes ─────────────────────────────────────────────── */
/**
 * All subsystem functions return one of these values.
 * CHIP8_OK == 0 so callers can test `if (r)` for any error.
 */
typedef enum {
    CHIP8_OK                 = 0,
    CHIP8_ERR_NULL_PTR,          /**< A required pointer argument was NULL       */
    CHIP8_ERR_ROM_NOT_FOUND,     /**< ROM file could not be opened               */
    CHIP8_ERR_ROM_TOO_LARGE,     /**< ROM exceeds available program memory       */
    CHIP8_ERR_ROM_READ_FAIL,     /**< fread returned fewer bytes than expected   */
    CHIP8_ERR_INVALID_ADDR,      /**< Memory address out of range [0, 4095]      */
    CHIP8_ERR_STACK_OVERFLOW,    /**< CALL with sp already at max depth          */
    CHIP8_ERR_STACK_UNDERFLOW,   /**< RET with sp already at zero                */
    CHIP8_ERR_UNKNOWN_OPCODE,    /**< Fetched opcode does not match any handler  */
} Chip8Result;

/* ── Virtual machine state ────────────────────────────────────── */
/**
 * Complete CHIP-8 virtual machine state.
 *
 * Layout follows the specification in §6 of the PRD.
 * All subsystems operate through a pointer to this struct; there is no
 * global instance.
 */
typedef struct {
    /* ── Memory ────────────────────────────────────────────────── */
    uint8_t  memory[CHIP8_MEMORY_SIZE];     /**< Flat 4 KiB address space       */

    /* ── Registers ─────────────────────────────────────────────── */
    uint8_t  V[CHIP8_NUM_REGISTERS];        /**< General-purpose V0–VF           */
    uint16_t I;                             /**< Index register                  */
    uint16_t pc;                            /**< Program counter (starts 0x200)  */

    /* ── Call stack ─────────────────────────────────────────────── */
    uint16_t stack[CHIP8_STACK_DEPTH];      /**< Return address stack            */
    uint8_t  sp;                            /**< Stack pointer (next free slot)  */

    /* ── Timers ─────────────────────────────────────────────────── */
    uint8_t  delay_timer;                   /**< Decrements at 60 Hz             */
    uint8_t  sound_timer;                   /**< Decrements at 60 Hz; beep > 0  */

    /* ── I/O ────────────────────────────────────────────────────── */
    uint8_t  keypad[CHIP8_NUM_KEYS];        /**< Key state: 0 = up, 1 = down    */
    uint8_t  display[CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT];
                                            /**< Framebuffer: 0 = off, 1 = on   */

    /* ── Decoder scratchpad ─────────────────────────────────────── */
    uint16_t opcode;                        /**< Currently decoded opcode word   */
} Chip8;

/* ── Lifecycle API ────────────────────────────────────────────── */

/**
 * @brief Zero the VM state, load the font, and set PC to 0x200.
 * @param vm  Non-NULL pointer to an uninitialized or recycled Chip8 struct.
 * @return CHIP8_OK or CHIP8_ERR_NULL_PTR.
 */
Chip8Result chip8_init(Chip8 *vm);

/**
 * @brief Reset the VM to its post-init state (identical to chip8_init).
 *
 * Convenience wrapper for re-loading a ROM into an already-running emulator.
 */
void chip8_reset(Chip8 *vm);

/**
 * @brief Return a human-readable string for a result code.
 * @param result  Any Chip8Result value.
 * @return Pointer to a static string; never NULL.
 */
const char *chip8_result_str(Chip8Result result);

#endif /* CHIP8_CHIP8_H */
