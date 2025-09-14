#include <peripheral/peripherals.h>
#include <peripheral/ssd1306/ssd1306.h>
#include <system.h>
#include <watchdog.h>
#include <hal_i2c.h>
#include <errors.h>
#include <solar48_config.h>
#include <time.h>
#include <stdbool.h>

// Added to avoid async access. TRUE if no process using transfer buffer memory to OLED driver
volatile bool update_screen_idle = true;

// Added to avoid async access. TRUE if no process is updating oled buffer
volatile bool oled_buffer_idle = true;

// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// Screen object
static SSD1306_t SSD1306;

//
//  Send a byte to the command register
//

static int ssd1306_WriteCommand(uint8_t command)
{
    if (SSD1306.Initialized)
      return hal_i2c1_write(SSD1306_I2C_ADDR, 0x00, &command, 1, 10);
 
    return 1;
}

//
//  Initialize the oled screen
//
int ssd1306_Init()
{
    // Wait for the screen to boot

    SSD1306.Initialized = 1;

    TIMEOUT_MS timeout;
    init_timeout_ms(&timeout, OLED_INITIALIZE_TIME_MS);

    while (!is_timeout_ms(&timeout));

    int status = 0;

    // Init LCD
    status += ssd1306_WriteCommand(0xAE);   // Display off
    status += ssd1306_WriteCommand(0x20);   // Set Memory Addressing Mode
    status += ssd1306_WriteCommand(0x10);   // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    status += ssd1306_WriteCommand(0xB0);   // Set Page Start Address for Page Addressing Mode,0-7
    status += ssd1306_WriteCommand(0xC8);   // Set COM Output Scan Direction
    status += ssd1306_WriteCommand(0x00);   // Set low column address
    status += ssd1306_WriteCommand(0x10);   // Set high column address
    status += ssd1306_WriteCommand(0x40);   // Set start line address
    status += ssd1306_WriteCommand(0x81);   // set contrast control register
    status += ssd1306_WriteCommand(0xFF);
    status += ssd1306_WriteCommand(0xA1);   // Set segment re-map 0 to 127
    status += ssd1306_WriteCommand(0xA6);   // Set normal display

    status += ssd1306_WriteCommand(0xA8);   // Set multiplex ratio(1 to 64)
    status += ssd1306_WriteCommand(SSD1306_HEIGHT - 1);

    status += ssd1306_WriteCommand(0xA4);   // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    status += ssd1306_WriteCommand(0xD3);   // Set display offset
    status += ssd1306_WriteCommand(0x00);   // No offset
    status += ssd1306_WriteCommand(0xD5);   // Set display clock divide ratio/oscillator frequency
    status += ssd1306_WriteCommand(0xF0);   // Set divide ratio
    status += ssd1306_WriteCommand(0xD9);   // Set pre-charge period
    status += ssd1306_WriteCommand(0x22);

    status += ssd1306_WriteCommand(0xDA);   // Set com pins hardware configuration
    status += ssd1306_WriteCommand(SSD1306_COM_LR_REMAP << 5 | SSD1306_COM_ALTERNATIVE_PIN_CONFIG << 4 | 0x02);   

    status += ssd1306_WriteCommand(0xDB);   // Set vcomh
    status += ssd1306_WriteCommand(0x20);   // 0x20,0.77xVcc
    status += ssd1306_WriteCommand(0x8D);   // Set DC-DC enable
    status += ssd1306_WriteCommand(0x14);   //
    status += ssd1306_WriteCommand(0xAF);   // Turn on SSD1306 panel

    if (status != 0) {
        SSD1306.Initialized = 0;

        return status;
    }

    // Clear screen
    ssd1306_Fill(Black);

    // Flush buffer to screen
    ssd1306_UpdateScreen();

    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

    return 0;
}

//
//  Fill the whole screen with the given color
//
void ssd1306_Fill(SSD1306_COLOR color)
{
    // Fill screenbuffer with a constant value (color)
    uint32_t i;

    for(i = 0; i < sizeof(SSD1306_Buffer); i++)
    {
        SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

//
//  Write the screenbuffer with changed to the screen
//

int ssd1306_UpdateScreen()
{
  uint8_t i;

  int status  = 0; // Added. status avoids long delay and does not starts watchdog.

  for (i = 0; i < 8; i++) {
    iwd_refresh();
    status = ssd1306_WriteCommand(0xB0 + i);
    status += ssd1306_WriteCommand(0x00);
    status += ssd1306_WriteCommand(0x10);

    status += hal_i2c1_write(SSD1306_I2C_ADDR, 0x40, &SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH, 100);

    if (status)
      break;
  }

  return status;
}

int ssd1306_UpdateScreen_ret()
{
  if (update_screen_idle) {
    update_screen_idle = false;

    int status = ssd1306_UpdateScreen();

    update_screen_idle = true;

    return status;
  }

  return E_OLED_BUFFER_TRANSFER_BUSY;
}

// Added to avoid async access. Check if BUSY. If so, try with timeout
int ssd1306_UpdateScreen_ret_hold()
{
  int status = ssd1306_UpdateScreen_ret();

  // try again with timeout
  if (status == E_OLED_BUFFER_TRANSFER_BUSY) {
    TIMEOUT_MS timeout;
    init_timeout_ms(&timeout, OLED_BUFFER_TRANSFER_TIMEOUT_MS);

    while ((status = ssd1306_UpdateScreen_ret()) == E_OLED_BUFFER_TRANSFER_BUSY) {
      if (!is_timeout_ms(&timeout)) {
        iwd_refresh(); // Reset watchdog
        continue;
      }

      return E_OLED_BUFFER_TRANSFER_BUSY_TIMEOUT;
    }
  }

  return status;
}

//
//  Draw one pixel in the screenbuffer
//  X => X Coordinate
//  Y => Y Coordinate
//  color => Pixel color
//
static void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    {
        // Don't write outside the buffer
        return;
    }

    // Check if pixel should be inverted
    if (SSD1306.Inverted)
    {
        color = (SSD1306_COLOR)!color;
    }

    // Draw in the correct color
    if (color == White)
    {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    }
    else
    {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}


//
//  Draw 1 char to the screen buffer
//  ch      => Character to write
//  Font    => Font to use
//  color   => Black or White
//
char ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color)
{
    uint32_t i, b, j;

    // Check remaining space on current line
    if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font.FontWidth) ||
        SSD1306_HEIGHT <= (SSD1306.CurrentY + Font.FontHeight))
    {
        // Not enough space on current line
        return 0;
    }

    // Translate font to screenbuffer
    for (i = 0; i < Font.FontHeight; i++)
    {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for (j = 0; j < Font.FontWidth; j++)
        {
            if ((b << j) & 0x8000)
            {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
            }
            else
            {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    SSD1306.CurrentX += Font.FontWidth;

    // Return written char for validation
    return ch;
}

//
//  Write full string to screenbuffer
//
/*
//TODO Remove it. use oled_printf instead
static char ssd1306_WriteString(const char* str, FontDef Font, SSD1306_COLOR color)
{
    // Write until null-byte
    while (*str)
    {
        if (ssd1306_WriteChar(*str, Font, color) != *str)
        {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}
*/
//
//  Invert background/foreground colors
//
void ssd1306_InvertColors(void)
{
    SSD1306.Inverted = !SSD1306.Inverted;
}

inline void ssd1306_SetCursor(uint8_t x, uint8_t y)
{
   SSD1306.CurrentX = x;
   SSD1306.CurrentY = y;
}

#define SSD1306_SET_CURSOR_BUILD \
  oled_buffer_idle = false; \
  SSD1306.CurrentX = x; \
  SSD1306.CurrentY = y; \
  oled_buffer_idle = true;

bool ssd1306_SetCursor_ret(uint8_t x, uint8_t y)
{
  if (oled_buffer_idle) {
    SSD1306_SET_CURSOR_BUILD
  }

  return oled_buffer_idle;
}

int ssd1306_SetCursor_ret_hold(uint8_t x, uint8_t y)
{
  if (oled_buffer_idle) {

    SSD1306_SET_CURSOR_BUILD

    return 0;
  }

  TIMEOUT_MS timeout;
  init_timeout_ms(&timeout, OLED_BUFFER_TIMEOUT_MS);

  while (!oled_buffer_idle) {
    if (!is_timeout_ms(&timeout))
      continue;

    return E_OLED_BUFFER_BUSY_TIMEOUT;
  }

  SSD1306_SET_CURSOR_BUILD

  return 0;
}

inline uint16_t ssd1306_GetCursorX()
{
  return SSD1306.CurrentX;
}

inline uint16_t ssd1306_GetCursorY()
{
  return SSD1306.CurrentY;
}

