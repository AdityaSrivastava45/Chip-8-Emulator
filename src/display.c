/**
 * @file display.c
 * @brief SDL2 display subsystem — window creation, framebuffer upload, render.
 *
 * The DisplayCtx struct is fully opaque to every other translation unit.
 * SDL2 headers are included ONLY here; no other .c file needs them for
 * display purposes.
 *
 * Colour scheme: warm-cream pixels on a deep-navy background — readable
 * and easy on the eyes for long sessions.
 */

#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "display.h"

/* ── Pixel colours (RGBA8888: R<<24 | G<<16 | B<<8 | A) ─────── */
#define PIXEL_ON  0xF0E6D0FFu   /* warm cream  */
#define PIXEL_OFF 0x0D0F1AFFu   /* deep navy   */

/* ── Opaque context definition ───────────────────────────────── */
struct DisplayCtx {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *texture;   /* streaming, CHIP8_DISPLAY_WIDTH × HEIGHT */
    int           scale;
};

/* ── display_init ─────────────────────────────────────────────── */

Chip8Result display_init(DisplayCtx **ctx, int scale)
{
    if (!ctx) return CHIP8_ERR_NULL_PTR;
    if (scale < 1) scale = DISPLAY_DEFAULT_SCALE;

    DisplayCtx *d = calloc(1, sizeof(DisplayCtx));
    if (!d) return CHIP8_ERR_NULL_PTR;
    d->scale = scale;

    int win_w = (int)(CHIP8_DISPLAY_WIDTH  * (unsigned)scale);
    int win_h = (int)(CHIP8_DISPLAY_HEIGHT * (unsigned)scale);

    d->window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        win_w, win_h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!d->window) {
        free(d);
        return CHIP8_ERR_NULL_PTR;
    }

    d->renderer = SDL_CreateRenderer(
        d->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!d->renderer) {
        SDL_DestroyWindow(d->window);
        free(d);
        return CHIP8_ERR_NULL_PTR;
    }

    /* Scale the low-res texture up to the window size. */
    SDL_RenderSetLogicalSize(d->renderer, win_w, win_h);
    SDL_RenderSetIntegerScale(d->renderer, SDL_TRUE);

    /* Streaming texture: one uint32_t per CHIP-8 pixel. */
    d->texture = SDL_CreateTexture(
        d->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        (int)CHIP8_DISPLAY_WIDTH,
        (int)CHIP8_DISPLAY_HEIGHT
    );
    if (!d->texture) {
        SDL_DestroyRenderer(d->renderer);
        SDL_DestroyWindow(d->window);
        free(d);
        return CHIP8_ERR_NULL_PTR;
    }

    *ctx = d;
    return CHIP8_OK;
}

/* ── display_render ───────────────────────────────────────────── */

Chip8Result display_render(DisplayCtx *ctx, const Chip8 *vm)
{
    if (!ctx || !vm) return CHIP8_ERR_NULL_PTR;

    void *pixels = NULL;
    int   pitch  = 0;

    if (SDL_LockTexture(ctx->texture, NULL, &pixels, &pitch) != 0)
        return CHIP8_ERR_NULL_PTR;

    uint32_t *p = (uint32_t *)pixels;
    for (unsigned i = 0u;
         i < CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT;
         ++i)
    {
        p[i] = vm->display[i] ? PIXEL_ON : PIXEL_OFF;
    }

    SDL_UnlockTexture(ctx->texture);

    SDL_SetRenderDrawColor(ctx->renderer, 0x0D, 0x0F, 0x1A, 0xFF);
    SDL_RenderClear(ctx->renderer);
    SDL_RenderCopy(ctx->renderer, ctx->texture, NULL, NULL);
    SDL_RenderPresent(ctx->renderer);

    return CHIP8_OK;
}

/* ── display_clear_vm ─────────────────────────────────────────── */

void display_clear_vm(Chip8 *vm)
{
    if (!vm) return;
    memset(vm->display, 0, sizeof(vm->display));
}

/* ── display_destroy ──────────────────────────────────────────── */

void display_destroy(DisplayCtx *ctx)
{
    if (!ctx) return;
    if (ctx->texture)  SDL_DestroyTexture(ctx->texture);
    if (ctx->renderer) SDL_DestroyRenderer(ctx->renderer);
    if (ctx->window)   SDL_DestroyWindow(ctx->window);
    free(ctx);
}
