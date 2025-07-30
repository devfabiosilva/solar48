#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <peripheral/ssd1306/ssd1306.h>
#include <peripheral/ssd1306/fonts.h>

static char oled_buffer[64];
static FontDef *current_font = &Font_7x10;

#define OLED_BUF_MAX_SIZE sizeof(oled_buffer)

int oled_printf(const char *fmt, ...)
{
  _ssize_t len;
  va_list arg;

  va_start(arg, fmt);
  len = vsnprintf(oled_buffer, OLED_BUF_MAX_SIZE, fmt, arg);
  va_end(arg);

  if (len >= OLED_BUF_MAX_SIZE) {
    len = OLED_BUF_MAX_SIZE - 1;
    oled_buffer[len] = 0;
  } else if (len == 0) return 0;
  else if (len < 0) return -2;

  uint16_t y = ssd1306_GetCursorY();

  for (char *p = oled_buffer; *p; ++p) {
    if (*p != '\n')
      ssd1306_WriteChar(*p, *current_font, White);
    else {
      y += current_font->FontHeight + 1;
      ssd1306_SetCursor(0, y);
    }
  }

  return ssd1306_UpdateScreen_ret();
}

