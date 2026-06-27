/**
 * @file display.h
 * @brief SDL2 display subsystem — window, renderer, and framebuffer update.
 *
 * Implemented in Milestone 3.  The DisplayCtx struct is opaque to all other
 * subsystems; only a pointer is passed around, keeping SDL2 headers out of
 * non-display translation units.
 */

#ifndef CHIP8_DISPLAY_H
#define CHIP8_DISPLAY_H

#include <stdint.h>
#include "chip8.h"

/* ── Opaque context ───────────────────────────────────────────── */

/**
 * Opaque SDL2 window/renderer/texture bundle.
 * Allocated by display_init(); freed by display_destroy().
 * No other subsystem may dereference this pointer directly.
 */
typedef struct DisplayCtx DisplayCtx;

/* ── Configuration ────────────────────────────────────────────── */

/** Default pixel scale factor (each CHIP-8 pixel → 10×10 screen pixels). */
#define DISPLAY_DEFAULT_SCALE 10

/** Resulting window dimensions at default scale. */
#define DISPLAY_WINDOW_W (CHIP8_DISPLAY_WIDTH  * DISPLAY_DEFAULT_SCALE)
#define DISPLAY_WINDOW_H (CHIP8_DISPLAY_HEIGHT * DISPLAY_DEFAULT_SCALE)

/* ── Lifecycle ────────────────────────────────────────────────── */

/**
 * @brief Create the SDL2 window, renderer, and streaming texture.
 *
 * @param ctx   Receives a heap-allocated DisplayCtx on success.
 * @param scale Pixel scale factor (e.g. 10 → 640×320 window).
 * @return CHIP8_OK on success.
 */
Chip8Result display_init(DisplayCtx **ctx, int scale);

/**
 * @brief Upload vm->display[] to the GPU texture and present the frame.
 *
 * @param ctx  Non-NULL context from display_init().
 * @param vm   Non-NULL VM state providing the framebuffer.
 */
Chip8Result display_render(DisplayCtx *ctx, const Chip8 *vm);

/**
 * @brief Zero every pixel in vm->display[] (does NOT redraw the window).
 *
 * Used by opcode 00E0 (CLS).
 */
void display_clear_vm(Chip8 *vm);

/**
 * @brief Destroy the SDL2 window and free the DisplayCtx.
 *
 * Safe to call with ctx == NULL.
 */
void display_destroy(DisplayCtx *ctx);

#endif /* CHIP8_DISPLAY_H */
