/**
 * @file input.h
 * @brief SDL2 keyboard input — event processing and CHIP-8 key mapping.
 *
 * Implemented in Milestone 4.
 *
 * Physical key layout (QWERTY → CHIP-8):
 *
 *   Physical:  1  2  3  4       CHIP-8:  1  2  3  C
 *              Q  W  E  R                4  5  6  D
 *              A  S  D  F                7  8  9  E
 *              Z  X  C  V                A  0  B  F
 */

#ifndef CHIP8_INPUT_H
#define CHIP8_INPUT_H

#include <stdbool.h>
#include "chip8.h"

/**
 * @brief Drain the SDL2 event queue and update vm->keypad[].
 *
 * Also detects the window-close / Escape event.
 *
 * @param vm   Non-NULL VM state; vm->keypad[] is updated in place.
 * @param quit Set to true if the user requested application exit.
 * @return CHIP8_OK always (errors are logged, not fatal).
 */
Chip8Result input_process(Chip8 *vm, bool *quit);

/**
 * @brief Blocking wait: spin until any CHIP-8 key is pressed.
 *
 * Used by opcode FX0A.  Stores the key index into *key_out.
 *
 * @param vm      Non-NULL VM state.
 * @param key_out Receives the CHIP-8 key index [0x0, 0xF].
 * @param quit    Set to true if window close was received while waiting.
 * @return CHIP8_OK on success.
 */
Chip8Result input_wait_for_key(Chip8 *vm, uint8_t *key_out, bool *quit);

#endif /* CHIP8_INPUT_H */
