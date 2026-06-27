/**
 * @file cpu.c
 * @brief CHIP-8 CPU — full fetch / decode / execute pipeline.
 *
 * All 35 standard CHIP-8 instructions are implemented here.
 *
 * Compatibility choices (matches the majority of published ROMs):
 *   • 8XY6 / 8XYE  — shift VX in-place; do NOT copy VY first.
 *   • FX55 / FX65  — I is NOT modified after the store/load.
 *   • BNNN         — jump to NNN + V0 (not BXNN + VX).
 *   • 8XY1/2/3     — VF is NOT reset (modern behaviour).
 *   • VF flag      — written AFTER the destination register so
 *                    "8FF_" instructions give the flag, not the result.
 */

#include <stdlib.h>    /* rand()  */
#include <string.h>    /* memset  */
#include "cpu.h"
#include "memory.h"

/* ── cpu_fetch ────────────────────────────────────────────────── */

Chip8Result cpu_fetch(Chip8 *vm)
{
    if (!vm) return CHIP8_ERR_NULL_PTR;

    Chip8Result r = memory_read16(vm, vm->pc, &vm->opcode);
    if (r != CHIP8_OK) return r;

    vm->pc = (uint16_t)(vm->pc + 2u);
    return CHIP8_OK;
}

/* ── cpu_decode_execute ───────────────────────────────────────── */

Chip8Result cpu_decode_execute(Chip8 *vm)
{
    if (!vm) return CHIP8_ERR_NULL_PTR;

    /* Extract common fields once. */
    const uint8_t  x   = (uint8_t) ((vm->opcode >> 8u) & 0x0Fu);
    const uint8_t  y   = (uint8_t) ((vm->opcode >> 4u) & 0x0Fu);
    const uint8_t  n   = (uint8_t) ( vm->opcode        & 0x0Fu);
    const uint8_t  kk  = (uint8_t) ( vm->opcode        & 0x00FFu);
    const uint16_t nnn = (uint16_t)( vm->opcode         & 0x0FFFu);

    switch (vm->opcode & 0xF000u) {

    /* ── Group 0x0 ──────────────────────────────────────────── */
    case 0x0000:
        switch (vm->opcode) {

        case 0x00E0: /* CLS — clear the display buffer */
            memset(vm->display, 0, sizeof(vm->display));
            break;

        case 0x00EE: /* RET — return from subroutine */
            if (vm->sp == 0u) return CHIP8_ERR_STACK_UNDERFLOW;
            vm->sp--;
            vm->pc = vm->stack[vm->sp];
            break;

        default:
            /* 0NNN: machine-code call — silently ignored by all modern
             * interpreters; real COSMAC VIP ROMs never emit these.       */
            break;
        }
        break;

    /* ── 1NNN: JP addr ──────────────────────────────────────── */
    case 0x1000:
        vm->pc = nnn;
        break;

    /* ── 2NNN: CALL addr ────────────────────────────────────── */
    case 0x2000:
        if (vm->sp >= CHIP8_STACK_DEPTH) return CHIP8_ERR_STACK_OVERFLOW;
        vm->stack[vm->sp] = vm->pc;
        vm->sp++;
        vm->pc = nnn;
        break;

    /* ── 3XKK: SE Vx, byte ──────────────────────────────────── */
    case 0x3000:
        if (vm->V[x] == kk) vm->pc = (uint16_t)(vm->pc + 2u);
        break;

    /* ── 4XKK: SNE Vx, byte ─────────────────────────────────── */
    case 0x4000:
        if (vm->V[x] != kk) vm->pc = (uint16_t)(vm->pc + 2u);
        break;

    /* ── 5XY0: SE Vx, Vy ────────────────────────────────────── */
    case 0x5000:
        if (n != 0u) return CHIP8_ERR_UNKNOWN_OPCODE;
        if (vm->V[x] == vm->V[y]) vm->pc = (uint16_t)(vm->pc + 2u);
        break;

    /* ── 6XKK: LD Vx, byte ──────────────────────────────────── */
    case 0x6000:
        vm->V[x] = kk;
        break;

    /* ── 7XKK: ADD Vx, byte (no carry flag) ─────────────────── */
    case 0x7000:
        vm->V[x] = (uint8_t)(vm->V[x] + kk);
        break;

    /* ── Group 0x8: register-to-register ALU ────────────────── */
    case 0x8000:
        switch (n) {

        case 0x0: /* 8XY0 LD  Vx, Vy */
            vm->V[x] = vm->V[y];
            break;

        case 0x1: /* 8XY1 OR  Vx, Vy */
            vm->V[x] |= vm->V[y];
            break;

        case 0x2: /* 8XY2 AND Vx, Vy */
            vm->V[x] &= vm->V[y];
            break;

        case 0x3: /* 8XY3 XOR Vx, Vy */
            vm->V[x] ^= vm->V[y];
            break;

        case 0x4: { /* 8XY4 ADD Vx, Vy  (VF = carry) */
            uint16_t sum = (uint16_t)vm->V[x] + (uint16_t)vm->V[y];
            vm->V[x]   = (uint8_t)(sum & 0xFFu);
            vm->V[0xF] = (sum > 0xFFu) ? 1u : 0u;
            break;
        }

        case 0x5: { /* 8XY5 SUB Vx, Vy  (VF = NOT borrow) */
            uint8_t not_borrow = (vm->V[x] >= vm->V[y]) ? 1u : 0u;
            vm->V[x]   = (uint8_t)(vm->V[x] - vm->V[y]);
            vm->V[0xF] = not_borrow;
            break;
        }

        case 0x6: { /* 8XY6 SHR Vx  (VF = shifted-out bit) */
            uint8_t lsb = vm->V[x] & 0x01u;
            vm->V[x]   = (uint8_t)(vm->V[x] >> 1u);
            vm->V[0xF] = lsb;
            break;
        }

        case 0x7: { /* 8XY7 SUBN Vx, Vy  (VF = NOT borrow) */
            uint8_t not_borrow = (vm->V[y] >= vm->V[x]) ? 1u : 0u;
            vm->V[x]   = (uint8_t)(vm->V[y] - vm->V[x]);
            vm->V[0xF] = not_borrow;
            break;
        }

        case 0xE: { /* 8XYE SHL Vx  (VF = shifted-out bit) */
            uint8_t msb = (vm->V[x] >> 7u) & 0x01u;
            vm->V[x]   = (uint8_t)(vm->V[x] << 1u);
            vm->V[0xF] = msb;
            break;
        }

        default:
            return CHIP8_ERR_UNKNOWN_OPCODE;
        }
        break;

    /* ── 9XY0: SNE Vx, Vy ───────────────────────────────────── */
    case 0x9000:
        if (n != 0u) return CHIP8_ERR_UNKNOWN_OPCODE;
        if (vm->V[x] != vm->V[y]) vm->pc = (uint16_t)(vm->pc + 2u);
        break;

    /* ── ANNN: LD I, addr ───────────────────────────────────── */
    case 0xA000:
        vm->I = nnn;
        break;

    /* ── BNNN: JP V0, addr ──────────────────────────────────── */
    case 0xB000:
        vm->pc = (uint16_t)(nnn + (uint16_t)vm->V[0]);
        break;

    /* ── CXKK: RND Vx, byte ─────────────────────────────────── */
    case 0xC000:
        vm->V[x] = (uint8_t)((uint8_t)(rand() & 0xFFu) & kk);
        break;

    /* ── DXYN: DRW Vx, Vy, nibble ───────────────────────────── */
    case 0xD000: {
        /*
         * Draw an N-byte sprite from memory[I] at screen position
         * (Vx % 64, Vy % 32).  Pixels are XOR'd; if any lit pixel
         * is turned off, set VF = 1 (collision), else VF = 0.
         * Sprites wrap around the display edges.
         */
        const uint8_t px = vm->V[x] % (uint8_t)CHIP8_DISPLAY_WIDTH;
        const uint8_t py = vm->V[y] % (uint8_t)CHIP8_DISPLAY_HEIGHT;
        vm->V[0xF] = 0u;

        for (uint8_t row = 0u; row < n; ++row) {
            uint8_t sprite_byte = 0u;
            Chip8Result mr = memory_read8(vm,
                                          (uint16_t)(vm->I + (uint16_t)row),
                                          &sprite_byte);
            if (mr != CHIP8_OK) break; /* out-of-range sprite data: stop */

            for (uint8_t col = 0u; col < 8u; ++col) {
                if (!(sprite_byte & (0x80u >> col))) continue;

                unsigned dx  = ((unsigned)px + col) % CHIP8_DISPLAY_WIDTH;
                unsigned dy  = ((unsigned)py + row) % CHIP8_DISPLAY_HEIGHT;
                unsigned idx = dy * CHIP8_DISPLAY_WIDTH + dx;

                if (vm->display[idx]) vm->V[0xF] = 1u; /* collision */
                vm->display[idx] ^= 1u;
            }
        }
        break;
    }

    /* ── Group 0xE: key-skip instructions ───────────────────── */
    case 0xE000:
        switch (kk) {

        case 0x9E: /* EX9E SKP  Vx — skip if key[Vx] pressed */
            if (vm->keypad[vm->V[x] & 0x0Fu])
                vm->pc = (uint16_t)(vm->pc + 2u);
            break;

        case 0xA1: /* EXA1 SKNP Vx — skip if key[Vx] NOT pressed */
            if (!vm->keypad[vm->V[x] & 0x0Fu])
                vm->pc = (uint16_t)(vm->pc + 2u);
            break;

        default:
            return CHIP8_ERR_UNKNOWN_OPCODE;
        }
        break;

    /* ── Group 0xF: misc / I / timers ───────────────────────── */
    case 0xF000:
        switch (kk) {

        case 0x07: /* FX07 LD Vx, DT */
            vm->V[x] = vm->delay_timer;
            break;

        case 0x0A: {
            /* FX0A LD Vx, K — block until any key pressed.
             * Implementation: if no key is currently down, rewind PC
             * by 2 so this instruction is re-fetched next cycle.      */
            bool key_found = false;
            for (uint8_t k = 0u; k < CHIP8_NUM_KEYS; ++k) {
                if (vm->keypad[k]) {
                    vm->V[x]   = k;
                    key_found  = true;
                    break;
                }
            }
            if (!key_found) {
                vm->pc = (uint16_t)(vm->pc - 2u); /* re-execute next cycle */
            }
            break;
        }

        case 0x15: /* FX15 LD DT, Vx */
            vm->delay_timer = vm->V[x];
            break;

        case 0x18: /* FX18 LD ST, Vx */
            vm->sound_timer = vm->V[x];
            break;

        case 0x1E: /* FX1E ADD I, Vx */
            vm->I = (uint16_t)(vm->I + (uint16_t)vm->V[x]);
            break;

        case 0x29: /* FX29 LD F, Vx — point I at font sprite for digit Vx */
            vm->I = (uint16_t)(CHIP8_FONT_START +
                    (uint16_t)((vm->V[x] & 0x0Fu) * CHIP8_FONT_SPRITE_BYTES));
            break;

        case 0x33: { /* FX33 LD B, Vx — store BCD of Vx at I, I+1, I+2 */
            Chip8Result mr;
            mr = memory_write8(vm, vm->I,                          vm->V[x] / 100u);
            if (mr != CHIP8_OK) return mr;
            mr = memory_write8(vm, (uint16_t)(vm->I + 1u),  (vm->V[x] / 10u) % 10u);
            if (mr != CHIP8_OK) return mr;
            mr = memory_write8(vm, (uint16_t)(vm->I + 2u),        vm->V[x] % 10u);
            if (mr != CHIP8_OK) return mr;
            break;
        }

        case 0x55: { /* FX55 LD [I], Vx — store V0..Vx into memory[I..] */
            for (uint8_t i = 0u; i <= x; ++i) {
                Chip8Result mr = memory_write8(vm,
                                               (uint16_t)(vm->I + (uint16_t)i),
                                               vm->V[i]);
                if (mr != CHIP8_OK) return mr;
            }
            break;
        }

        case 0x65: { /* FX65 LD Vx, [I] — load V0..Vx from memory[I..] */
            for (uint8_t i = 0u; i <= x; ++i) {
                Chip8Result mr = memory_read8(vm,
                                              (uint16_t)(vm->I + (uint16_t)i),
                                              &vm->V[i]);
                if (mr != CHIP8_OK) return mr;
            }
            break;
        }

        default:
            return CHIP8_ERR_UNKNOWN_OPCODE;
        }
        break;

    default:
        return CHIP8_ERR_UNKNOWN_OPCODE;
    }

    return CHIP8_OK;
}
