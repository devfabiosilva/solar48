//#include "ssd1306.h"
#include <peripheral/ssd1306/ssd1306.h>
#include <system.h>

// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// Screen object
static SSD1306_t SSD1306;

extern uint64_t milliseconds();

//
//  Send a byte to the command register
//

static int ssd1306_WriteCommand(uint8_t command)
{
    //return HAL_I2C_Mem_Write(hi2c, SSD1306_I2C_ADDR, 0x00, 1, &command, 1, 10);
    if (SSD1306.Initialized)
      return hal_i2c1_write(SSD1306_I2C_ADDR, 0x00, &command, 1, 10);
 
    return 1;
}

//TODO remove it
//https://stackoverflow.com/questions/59143031/i2c-oled-will-not-turn-on-or-display
//https://github.com/adafruit/Adafruit_SSD1306
//https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf

//TODO remove (testing only)
/*
static int ssd1306_WriteCommand(uint8_t command)
{
    //return HAL_I2C_Mem_Write(hi2c, SSD1306_I2C_ADDR, 0x00, 1, &command, 1, 10);

    if (SSD1306.Initialized) {
      int err = hal_i2c1_write(SSD1306_I2C_ADDR, 0x00, &command, 1, 100);

      if (!err)
        return 0;

      if (!SSD1306.first_error)
        SSD1306.first_error = err;
      else
        SSD1306.last_error = err;
    }
 
    return 1;
}
*/
//Other example: https://github.com/raspberrypi/pico-examples/blob/master/i2c/ssd1306_i2c/ssd1306_i2c.c

//https://gist.github.com/pulsar256/564fda3b9e8fc6b06b89
/**
 * according to http://www.adafruit.com/datasheets/UG-2864HSWEG01.pdf Chapter 4.4 Page 15
 */

//
//  Initialize the oled screen
//
int ssd1306_Init()
{
    // Wait for the screen to boot

    SSD1306.Initialized = 1;

#ifdef RTOS_SOLAR48
  END_SETUP // Enable interrupt
#endif

    uint64_t delay_ms = milliseconds() + 100; // 100ms

    while (delay_ms > milliseconds());

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

#ifdef RTOS_SOLAR48
  DISABLE_SETUP // Disable interrupt
#endif

        return status;
    }

    // Clear screen
    ssd1306_Fill(Black);

    // Flush buffer to screen
    ssd1306_UpdateScreen();

    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

#ifdef RTOS_SOLAR48
  DISABLE_SETUP // Disable interrupt
#endif

//    SSD1306.Initialized = 1;

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
void ssd1306_UpdateScreen()
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        ssd1306_WriteCommand(0xB0 + i);
        ssd1306_WriteCommand(0x00);
        ssd1306_WriteCommand(0x10);

        hal_i2c1_write(SSD1306_I2C_ADDR, 0x40, &SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH, 100);
    }
}

//
//  Draw one pixel in the screenbuffer
//  X => X Coordinate
//  Y => Y Coordinate
//  color => Pixel color
//
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
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
char ssd1306_WriteString(const char* str, FontDef Font, SSD1306_COLOR color)
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

//
//  Invert background/foreground colors
//
void ssd1306_InvertColors(void)
{
    SSD1306.Inverted = !SSD1306.Inverted;
}

//
//  Set cursor position
//
void ssd1306_SetCursor(uint8_t x, uint8_t y)
{
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}
