/* src/kernel/graphics.c */
#include "../include/graphics.h"
#include "../include/font.h"

static graphics_context_t ctx;

void init_graphics(multiboot_info_t* mb_info) {
    if (mb_info->flags & MULTIBOOT_FLAG_FB) {
        ctx.framebuffer = (uint32_t*)(uint64_t)mb_info->framebuffer_addr;
        ctx.width = mb_info->framebuffer_width;
        ctx.height = mb_info->framebuffer_height;
        ctx.pitch = mb_info->framebuffer_pitch;
        ctx.bpp = mb_info->framebuffer_bpp;
    } else if (mb_info->flags & MULTIBOOT_FLAG_VBE) {
        /* Fallback if FB flag not set but VBE is (older GRUB) */
        /* Currently assuming linear FB is available via standard FB field */
        /* This path is just a placeholder for robustness later */
    }
}

void putpixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= (int)ctx.width || y < 0 || y >= (int)ctx.height) return;

    /* Assuming 32 bpp (ARGB/XRGB) */
    /* Pitch is in bytes, so we need to calculate offset carefully */

    // uint32_t* pixel_addr = (uint32_t*)((uint8_t*)ctx.framebuffer + y * ctx.pitch + x * 4);
    // *pixel_addr = color;

    /* Optimized for 32-bit aligned buffers (pitch usually width * 4) */
    ctx.framebuffer[y * (ctx.pitch / 4) + x] = color;
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int j = y; j < y + h; j++) {
        for (int i = x; i < x + w; i++) {
            putpixel(i, j, color);
        }
    }
}

void draw_char(int x, int y, char c, uint32_t color) {
    unsigned char uc = (unsigned char)c;
    if (uc > 127) return;
    const uint8_t* glyph = font8x8_basic[uc];

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (glyph[row] & (1 << (7 - col))) {
                /* Scale by 2 for visibility */
                draw_rect(x + col * 2, y + row * 2, 2, 2, color);
            }
        }
    }
}

void draw_string(int x, int y, const char* str, uint32_t color) {
    int cur_x = x;
    int cur_y = y;
    while (*str) {
        if (*str == '\n') {
            cur_x = x;
            cur_y += 16; /* 8x8 font scaled by 2 */
        } else {
            draw_char(cur_x, cur_y, *str, color);
            cur_x += 16;
        }
        str++;
    }
}

void clear_screen(uint32_t color) {
    draw_rect(0, 0, ctx.width, ctx.height, color);
}

uint32_t get_screen_width() {
    return ctx.width;
}

uint32_t get_screen_height() {
    return ctx.height;
}
