/**
 * @file timer.h
 * @brief CHIP-8 delay and sound timer management.
 *
 * Implemented in Milestone 5.
 *
 * Both timers decrement at exactly 60 Hz, independent of the CPU clock.
 * The emulator runtime is responsible for calling timer_tick() at the
 * correct wall-clock interval.
 */

#ifndef CHIP8_TIMER_H
#define CHIP8_TIMER_H

#include <stdbool.h>
#include "chip8.h"

/**
 * @brief Decrement delay_timer and sound_timer by 1 (floor 0).
 *
 * Call this once per 60 Hz tick, not once per CPU cycle.
 *
 * @param vm  Non-NULL VM state; timers are modified in place.
 */
void timer_tick(Chip8 *vm);

/**
 * @brief Return true if the sound timer is active (sound_timer > 0).
 *
 * The audio subsystem polls this to decide whether to emit a beep.
 */
bool timer_sound_active(const Chip8 *vm);

#endif /* CHIP8_TIMER_H */
