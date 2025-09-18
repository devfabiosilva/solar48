#ifndef OLED_UTILS_H
 #define OLED_UTILS_H

#include <peripheral/ssd1306/ssd1306.h>

int _oled_printf_panic(const char *, ...);

typedef enum font_size_e {
  FONT__DEFAULT = 1, //FONT__7x10
  FONT_11x18,
  FONT_16x26
} FONT_SIZE;

void set_font_size(FONT_SIZE);
void set_color(SSD1306_COLOR);
int oled_is_initialized();
int init_oled(char *);
int oled_cursor_printf(uint8_t, uint8_t, const char *, ...);
#endif

