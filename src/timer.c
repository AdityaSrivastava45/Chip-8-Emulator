/**
 * @file timer.c
 * @brief CHIP-8 delay and sound timer management.
 */

#include "timer.h"

/* ── timer_tick ───────────────────────────────────────────────── */

void timer_tick(Chip8 *vm) {
    if (!vm) return;
    
    if (vm->delay_timer > 0) {
        vm->delay_timer--;
    }
    
    if (vm->sound_timer > 0) {
        vm->sound_timer--;
    }
}

/* ── timer_sound_active ───────────────────────────────────────── */

bool timer_sound_active(const Chip8 *vm) {
    if (!vm) return false;
    
    return (vm->sound_timer > 0);
}
