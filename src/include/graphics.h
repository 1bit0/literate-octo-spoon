#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include "multiboot.h"

/* Simple color definitions (ARGB8888) */
#define COLOR_BLACK 0xFF000000
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_RED   0xFFFF0000
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE  0xFF0000FF
#define COLOR_WIN_BLUE 0xFF0078D7
#define COLOR_TASKBAR 0xFF202020
#define COLOR_BSOD 0xFF0000AA

typedef struct {
    uint32_t* framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch; // in bytes
    uint8_t bpp;
} graphics_context_t;

void init_graphics(multiboot_info_t* mb_info);
void putpixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_char(int x, int y, char c, uint32_t color);
void draw_string(int x, int y, const char* str, uint32_t color);
void clear_screen(uint32_t color);
uint32_t get_screen_width();
uint32_t get_screen_height();

#endif
