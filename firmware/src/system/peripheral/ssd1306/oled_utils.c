#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <hal_i2c.h>
#include <errors.h>
#include <solar48_config.h>
#include <stdbool.h>
#include <time.h>
#include <sys_queue.h>
#include <stdlib.h>
#include <system.h>
#include <peripheral/ssd1306/ssd1306.h>
#include <peripheral/ssd1306/fonts.h>
#include <peripheral/ssd1306/oled_utils.h>

extern SSD1306_t SSD1306;
extern void app_panic(const char *);

static char oled_buffer[OLED_BUF_MAX_SIZE];
static FontDef *current_font = &Font_7x10;
static SSD1306_COLOR current_color = White;

volatile bool oled_util_lock = false;
extern volatile bool oled_buffer_lock;

static SYS_QUEUE oled_print_queue = {0};

void _run_oled_process()
{
  run_queue_run(&oled_print_queue);
}

int init_oled(char *msg)
{
  if (ssd1306_Init())
    return E_OLED_INIT_FAILED;

  sys_queue_init(&oled_print_queue, OLED_PRINTIF_QUEUE_SIZE, OLED_PRINTF_ENABLE_GLOBAL_ERROR); // If any sys_queue_init fails it runs PANIC

  set_font_size(FONT_11x18);
  oled_cursor_printf(0, 0, msg);
  set_font_size(FONT__DEFAULT);

  return 0;
}

#define OLED_PRINTF_INIT \
  if (SSD1306.Initialized == 0)\
    return E_OLED_NOT_INITIALIZED;\
\
  _ssize_t len; \
  va_list arg; \
\
  va_start(arg, fmt); \
  len = vsnprintf(oled_buffer, OLED_BUF_MAX_SIZE, fmt, arg); \
  va_end(arg);


// Used only on panic Error Handler
int _oled_printf_panic(const char *fmt, ...)
{
  OLED_PRINTF_INIT

  if (len >= OLED_BUF_MAX_SIZE) {
    len = OLED_BUF_MAX_SIZE - 1;
    oled_buffer[len] = 0;
  } else if (len == 0) return 0;
  else if (len < 0) return E_INVALID_OLED_PRINTF_BUF_SIZE;

  uint16_t y = ssd1306_GetCursorY();

  for (char *p = oled_buffer; *p; ++p) {
    if (*p != '\n')
      ssd1306_WriteChar(*p, *current_font, current_color);
    else {
      y += current_font->FontHeight + 1;
      ssd1306_SetCursor(0, y);
    }
  }

  return ssd1306_UpdateScreen();
}

typedef struct oled_cursor_printf_t {
  uint8_t x, y;
  _ssize_t size;
  char *text;
  FontDef *font;
  SSD1306_COLOR color;
} OLED_CURSOR_PRINTF_TYP;

static int _oled_cursor_printf(void *ctx)
{

  OLED_CURSOR_PRINTF_TYP *print = (void *)ctx;

  if (print) {
    if (!print->text) {
      app_panic("_CURPRN:ERR2");
      return -2; // Never reaaches here
    }

    if (oled_buffer_flush_busy_ret_hold()) {
      free((void *)print->text);
      free((void *)print);
      return E_OLED_PRINTF_SET_CURSOR_FLUSH_BUSY;
    }

    // NOTE: sys queue guarantees that if flush is not busy then only this event is executed with exclusion. So only this event is accessing screen buffer

    TIMEOUT_MS timeout_ms;
    if (!sys_try_lock(&oled_buffer_lock, &timeout_ms, OLED_BUFFER_TIMEOUT_MS, NULL)) {
      free((void *)print->text);
      free((void *)print);
      return E_OLED_CURSOR_FLUSH_UPDATE_TIMEOUT;
    }

    // NOTE: sys queue guarantees that if buffer is not busy then only this event is executed with exclusion to update the buffer

    char *p = print->text;
    int16_t fill_with_spaces = (int16_t)(print->x < SSD1306_WIDTH)?print->font->char_per_line:0,
            space_count = fill_with_spaces;

    _ssize_t n = print->size;

    ssd1306_SetCursor(print->x, print->y);

    uint8_t y = print->y;
    FontDef *font = print->font;

    while (n > 0) {
      if (*p != '\n') {
        ssd1306_WriteChar(*p, *font, print->color);
        --space_count;
      } else {
        while (space_count > 0) {
          ssd1306_WriteChar(' ', *font, print->color);
          --space_count;
        }
        space_count = fill_with_spaces;
        y += font->FontHeight + 1;
        ssd1306_SetCursor(0, y);
      }
      ++p;
      --n;
    }

    while (space_count > 0) {
      ssd1306_WriteChar(' ', *font, print->color);
      --space_count;
    }

    free((void *)print->text);
    free((void *)print);

    sys_unlock(&oled_buffer_lock);

    return ssd1306_UpdateScreen_ret_hold();
  }

  app_panic("_CURPRN:ERR1");
  return -1; // Never reaaches here
}

int oled_cursor_printf(uint8_t x, uint8_t y, const char *fmt, ...)
{

  OLED_PRINTF_INIT

  TIMEOUT_MS timeout_ms;
  if (!sys_try_lock(&oled_util_lock, &timeout_ms, OLED_PRINTIF_TIMEOUT_MS, NULL))
    return E_OLED_CURSOR_PRINTF_LOCK_TIMEOUT;

  if (len > OLED_BUF_MAX_SIZE)
    len = OLED_BUF_MAX_SIZE;
  else if (len == 0) {
    sys_unlock(&oled_util_lock);
    return 0;
  } else if (len < 0) {
    sys_unlock(&oled_util_lock);
    return E_INVALID_OLED_CURSOR_PRINTF_BUF_SIZE;
  }

  OLED_CURSOR_PRINTF_TYP *oled_cursor_typ = (OLED_CURSOR_PRINTF_TYP *)malloc(sizeof(OLED_CURSOR_PRINTF_TYP));

  if (!oled_cursor_typ) {
    sys_unlock(&oled_util_lock);
    app_panic("CURPRN:ERR1");
    return -1; // Never reaches here
  }

  oled_cursor_typ->x = x;
  oled_cursor_typ->y = y;
  oled_cursor_typ->size = len; // Is size because it has no zero char at the end
  oled_cursor_typ->font = current_font;
  oled_cursor_typ->color = current_color;

  if (!(oled_cursor_typ->text = (char *)malloc((size_t)len))) {
    free((void *)oled_cursor_typ);
    sys_unlock(&oled_util_lock);
    app_panic("CURPRN:ERR2");
    return -1; // Never reaches here
  }

  memcpy(oled_cursor_typ->text, oled_buffer, (size_t)len);

  int err = sys_queue(&oled_print_queue, _oled_cursor_printf, (void *)oled_cursor_typ, NULL);

  if (err) {
    free((void *)oled_cursor_typ->text);
    free((void *)oled_cursor_typ);
  }

  sys_unlock(&oled_util_lock);

  return err;
}

void set_font_size(FONT_SIZE font_size)
{
  switch (font_size) {
    case FONT_11x18:
      current_font = &Font_11x18;
      break;
    case FONT_16x26:
      current_font = &Font_16x26;
      break;
    default:
      current_font = &Font_7x10;
  }
}

inline void set_color(SSD1306_COLOR color)
{
  current_color = color;
}

inline int oled_is_initialized()
{
  return SSD1306.Initialized;
}

