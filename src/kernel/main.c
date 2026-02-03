/* src/kernel/main.c */

typedef unsigned long long size_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define VGA_ADDRESS 0xb8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

enum VgaColor {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_LIGHT_BROWN = 14,
    COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum VgaColor fg, enum VgaColor bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

uint16_t* terminal_buffer;
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(COLOR_WHITE, COLOR_BLUE);
    terminal_buffer = (uint16_t*) VGA_ADDRESS;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_row++;
        terminal_column = 0;
        return;
    }
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_row = 0;
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

// Pseudo-random number generator
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

void bsod() {
    terminal_setcolor(vga_entry_color(COLOR_WHITE, COLOR_BLUE));
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_putentryat(' ', terminal_color, x, y);
        }
    }
    terminal_row = 2;
    terminal_column = 5;
    terminal_writestring(":(");
    terminal_row = 5;
    terminal_column = 5;
    terminal_writestring("Your PC ran into a problem and needs to restart.");
    terminal_row = 6;
    terminal_column = 5;
    terminal_writestring("We're just collecting some error info, and then we'll restart for you.");

    terminal_row = 8;
    terminal_column = 5;
    terminal_writestring("0% complete");

    sleep(100000000);
    terminal_row = 8;
    terminal_column = 5;
    terminal_writestring("20% complete");
    sleep(100000000);
    terminal_row = 8;
    terminal_column = 5;
    terminal_writestring("100% complete");

    sleep(50000000);

    terminal_row = 12;
    terminal_column = 5;
    terminal_writestring("Stop Code: CRITICAL_PROCESS_DIED_OF_BOREDOM");

    while(1) { __asm__("hlt"); }
}

void kernel_main(void) {
    terminal_initialize();

    // Simulate "Starting Windows" - Black screen
    terminal_setcolor(vga_entry_color(COLOR_WHITE, COLOR_BLACK));
     for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_putentryat(' ', terminal_color, x, y);
        }
    }

    terminal_row = 10;
    terminal_column = 30;
    terminal_writestring("Windows 11 (Bad Edition)");

    terminal_row = 12;
    terminal_column = 35;
    char spinner[] = "|/-\\";
    for (int i = 0; i < 50; i++) {
        terminal_putentryat(spinner[i % 4], terminal_color, 38, 12);
        sleep(5000000);
    }

    // Windows Update annoyance - Blue screen
    terminal_setcolor(vga_entry_color(COLOR_WHITE, COLOR_BLUE));
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_putentryat(' ', terminal_color, x, y);
        }
    }

    terminal_row = 10;
    terminal_column = 10;
    terminal_writestring("Working on updates. 1% complete.");
    terminal_row = 11;
    terminal_column = 10;
    terminal_writestring("Don't turn off your computer.");

    int progress = 1;
    while (progress < 100) {
        sleep(10000000);

        // Randomly reset progress or stall
        int r = rand() % 100;
        if (r < 5) {
            progress = 0; // Reset
            terminal_row = 10;
            terminal_column = 10;
            terminal_writestring("Working on updates. 0% complete. (Whoops)   ");
        } else if (r < 20) {
            // Stall, do nothing
        } else {
            progress++;
            terminal_row = 10;
            terminal_column = 28;
            // Hacky number printing
            char buf[4];
            if (progress < 10) {
                buf[0] = progress + '0';
                buf[1] = '%';
                buf[2] = ' ';
                buf[3] = 0;
            } else {
                buf[0] = (progress / 10) + '0';
                buf[1] = (progress % 10) + '0';
                buf[2] = '%';
                buf[3] = 0;
            }
            terminal_writestring(buf);
        }

        // Random BSOD
        if (rand() % 300 == 0) {
            bsod();
        }
    }

    // If we actually finish updates (unlikely)
    bsod();
}
