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

volatile bool oled_util_lock = false; // TODO REMOVE

static SYS_QUEUE oled_print_queue = {0};

int init_oled(char *msg)
{
  if (ssd1306_Init())
    return E_OLED_INIT_FAILED;

  set_font_size(FONT_11x18);
  oled_printf(msg);
  set_font_size(FONT__DEFAULT);

  sys_queue_init(&oled_print_queue, -1, true);
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

int oled_printf(const char *fmt, ...)
{

  if (oled_util_lock)
    return -200; // TODO remove and refactor

  oled_util_lock = true;

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

  //return ssd1306_UpdateScreen_ret();
  int status = ssd1306_UpdateScreen_ret_hold();
  oled_util_lock = false; // TODO refactor this
  return status;
}

typedef struct oled_cursor_printf_t {
  uint8_t x, y;
  _ssize_t size;
  char *text;
} OLED_CURSOR_PRINTF_TYP;

static int _oled_cursor_printf(void *ctx)
{

  OLED_CURSOR_PRINTF_TYP *print = (void *)ctx;

  if (print) {
    if (!print->text) {
      app_panic("_CURPRN:ERR2");
      return -2; // Never reaaches here
    }

    if (flush_busy_ret()) {
      free((void *)print->text);
      free((void *)print);
      return E_OLED_PRINTF_SET_CURSOR_FLUSH_BUSY;
    }

    char *p = print->text;
    _ssize_t n = print->size;
    // NOTE: sys queue guarantees that if flush is not busy then only this event is executed with exclusion. So only this event is accessing screen buffer
    ssd1306_SetCursor(print->x, print->y);

    uint8_t y = print->y;

    while (n > 0) {
      if (*p != '\n')
        ssd1306_WriteChar(*p, *current_font, current_color);
      else {
        y += current_font->FontHeight + 1;
        ssd1306_SetCursor(0, y);
      }
      ++p;
      --n;
    }

    free((void *)print->text);
    free((void *)print);

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

void _run_oled_process()
{
  run_queue_run(&oled_print_queue);
}

