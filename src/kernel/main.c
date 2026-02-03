/* src/kernel/main.c */
#include "../include/multiboot.h"
#include "../include/graphics.h"

/* Standard Lib Helpers */
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

uint32_t rand_state = 123456789;
uint32_t rand() {
    rand_state = rand_state * 1103515245 + 12345;
    return (rand_state / 65536) % 32768;
}

void sleep(uint64_t count) {
    for (volatile uint64_t i = 0; i < count; i++) {
        __asm__("nop");
    }
}

void itoa(int n, char* buffer) {
    int i = 0;
    if (n == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }
    while (n > 0) {
        buffer[i++] = (n % 10) + '0';
        n /= 10;
    }
    buffer[i] = '\0';
    /* Reverse */
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

/* UI Elements */

void draw_bsod() {
    clear_screen(COLOR_BSOD);

    int x = 100;
    int y = 100;

    // Huge :(
    // Manually drawing a crude :( face using rects
    draw_rect(x, y, 20, 100, COLOR_WHITE); // Left eye
    draw_rect(x + 50, y, 20, 100, COLOR_WHITE); // Right eye

    // Mouth
    draw_rect(x, y + 150, 100, 20, COLOR_WHITE); // Top bar of frown
    draw_rect(x, y + 150, 20, 50, COLOR_WHITE); // Left down
    draw_rect(x + 80, y + 150, 20, 50, COLOR_WHITE); // Right down

    y += 250;

    draw_string(x, y, "Your PC ran into a problem and needs to restart.", COLOR_WHITE);
    draw_string(x, y + 30, "We're just collecting some error info, and then we'll restart for you.", COLOR_WHITE);

    draw_string(x, y + 80, "0% complete", COLOR_WHITE);

    sleep(100000000);
    draw_rect(x, y + 80, 200, 30, COLOR_BSOD); // Clear %
    draw_string(x, y + 80, "20% complete", COLOR_WHITE);

    sleep(100000000);
    draw_rect(x, y + 80, 200, 30, COLOR_BSOD);
    draw_string(x, y + 80, "100% complete", COLOR_WHITE);

    sleep(50000000);

    draw_string(x, y + 150, "Stop Code: CRITICAL_PROCESS_DIED_OF_BOREDOM", COLOR_WHITE);

    while(1) { __asm__("hlt"); }
}

void draw_window(int x, int y, int w, int h, const char* title, const char* message) {
    // Drop shadow
    draw_rect(x + 10, y + 10, w, h, 0xFF000000); // Shadow

    // Border
    draw_rect(x - 2, y - 2, w + 4, h + 4, 0xFF888888);

    // Background
    draw_rect(x, y, w, h, COLOR_WHITE);

    // Title bar
    draw_rect(x, y, w, 30, COLOR_WHITE);
    draw_string(x + 10, y + 5, title, COLOR_BLACK);

    // Close button
    draw_rect(x + w - 30, y, 30, 30, COLOR_RED);
    draw_string(x + w - 22, y + 5, "X", COLOR_WHITE);

    // Content
    draw_string(x + 20, y + 50, message, COLOR_BLACK);

    // OK Button
    int btn_x = x + w / 2 - 40;
    int btn_y = y + h - 40;
    draw_rect(btn_x, btn_y, 80, 30, 0xFFCCCCCC);
    draw_rect(btn_x + 1, btn_y + 1, 78, 28, 0xFFEEEEEE);
    draw_string(btn_x + 25, btn_y + 8, "OK", COLOR_BLACK);
}

void draw_taskbar() {
    int w = get_screen_width();
    int h = get_screen_height();

    draw_rect(0, h - 48, w, 48, 0xFFEEEEEE); // Taskbar bg

    // Start button (centered)
    int start_x = w / 2 - 20;
    int start_y = h - 40;
    // Windows Logo (4 blue squares)
    uint32_t logo_blue = 0xFF00ADEF;
    draw_rect(start_x, start_y, 14, 14, logo_blue);
    draw_rect(start_x + 16, start_y, 14, 14, logo_blue);
    draw_rect(start_x, start_y + 16, 14, 14, logo_blue);
    draw_rect(start_x + 16, start_y + 16, 14, 14, logo_blue);
}

void draw_update_screen() {
    clear_screen(COLOR_BLACK);
    int w = get_screen_width();
    int h = get_screen_height();

    int cx = w / 2 - 150;
    int cy = h / 2 - 50;

    draw_string(cx, cy, "Working on updates", COLOR_WHITE);
    draw_string(cx, cy + 30, "Don't turn off your computer", COLOR_WHITE);
    draw_string(cx, cy + 60, "This will take a while", COLOR_WHITE);

    int progress = 0;
    char buf[16];

    while (progress < 100) {
        sleep(5000000);

        // Random BSOD chance
        if (rand() % 500 == 0) {
            draw_bsod();
        }

        // Random reset
        if (rand() % 100 < 2) {
            progress = 0;
            draw_rect(cx, cy + 90, 300, 30, COLOR_BLACK); // Clear text
        } else {
            if (rand() % 10 > 2) progress++;
        }

        itoa(progress, buf);
        int len = strlen(buf);
        buf[len] = '%';
        buf[len+1] = 0;

        draw_rect(cx, cy + 90, 300, 30, COLOR_BLACK); // Clear text
        draw_string(cx, cy + 90, buf, COLOR_WHITE);
    }

    // "Finished" -> BSOD
    draw_bsod();
}

void kernel_main(multiboot_info_t* mb_info) {
    init_graphics(mb_info);

    // 1. Boot Logo
    clear_screen(COLOR_BLACK);
    int w = get_screen_width();
    int h = get_screen_height();
    int cx = w / 2 - 40;
    int cy = h / 2 - 40;

    // Cyan Windows Logo
    uint32_t logo_cyan = 0xFF00ADEF;
    draw_rect(cx, cy, 35, 35, logo_cyan);
    draw_rect(cx + 40, cy, 35, 35, logo_cyan);
    draw_rect(cx, cy + 40, 35, 35, logo_cyan);
    draw_rect(cx + 40, cy + 40, 35, 35, logo_cyan);

    // Spinner
    cy += 100;
    for (int i = 0; i < 20; i++) {
        draw_rect(cx, cy, 80, 10, COLOR_BLACK); // Clear
        draw_string(cx + (i % 4) * 20, cy, ".", COLOR_WHITE);
        sleep(10000000);
    }

    // 2. Desktop
    clear_screen(COLOR_WIN_BLUE); // Background
    draw_taskbar();

    // Draw "Recycle Bin" icon (just a grey rect)
    draw_rect(20, 20, 40, 50, 0xFFCCCCCC);
    draw_string(10, 80, "Recycle Bin", COLOR_WHITE);

    // Draw "Edge" icon
    draw_rect(20, 120, 40, 40, 0xFF00ADEF);
    draw_string(25, 130, "e", COLOR_WHITE);
    draw_string(20, 170, "Edge", COLOR_WHITE);

    // Annoying Popup 1
    sleep(20000000);
    draw_window(w / 2 - 200, h / 2 - 100, 400, 200, "Microsoft Edge", "Did you know Edge is strictly better than\nChrome? Please use it.");

    // Annoying Popup 2: Activate Windows
    sleep(20000000);
    draw_string(w - 300, h - 100, "Activate Windows", 0x88FFFFFF);
    draw_string(w - 300, h - 80, "Go to Settings to activate Windows.", 0x88FFFFFF);

    // Annoying Popup 3: Error
    sleep(20000000);
    draw_window(w / 2 - 150, h / 2, 300, 150, "System Error", "Task failed successfully.");

    // 3. Update Screen
    sleep(40000000);
    draw_update_screen();

    // Should not be reached
    while(1) { __asm__("hlt"); }
}
