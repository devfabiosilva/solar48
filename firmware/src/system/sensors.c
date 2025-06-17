#include <registers.h>
#include <time.h>
#include <watchdog.h>
// Refs Page 58 from STM32F103X6.pdf
// Page 236 rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf

#define MEASEURE_MEAN (int)4
void init_sensors()
{
//ADC clock = 12 MHz page 112
  RCC_CFGR |= ADCPRE(PCLK2_divided_by_6);
  RCC_APB2ENR |= ADC1EN;
}

static float read_vref_util()
{
  ADC1_CR2 |= TSVREFE; // TSVREFE: Enables sensor and Vrefint
  ADC1_SQR3 = 17;// Channel 17 = Vrefint

  ADC1_CR2 |= ADON;
  //delay(1);
  delay_5us();

  ADC1_CR2 |= ADON; // Begin conversion
  while ((ADC1_SR & EOC) == 0);

  uint16_t vrefint_adc = ADC1_DR;

  return (1.20f * 4096.0f) / (float)vrefint_adc;
}

float read_vref()
{
  float vref = 0;

  for (int i = 0; i < MEASEURE_MEAN; i++)
    vref += read_vref_util();

  return vref /= (float)MEASEURE_MEAN;
}

static float read_internal_temp_sensor_util()
{
  ADC1_CR2 |= TSVREFE; // TSVREFE: Enables sensor and Vrefint
  //delay(1);            // Waits for stabilization
  delay_5us();
  ADC1_SQR3 = 16;      // Channel 16 = Internal temperature sensor

  ADC1_CR2 |= ADON;      // Turns on ADC
  //delay(1);
  delay_5us();
  ADC1_CR2 |= ADON;      // Starts convertion

  while (!(ADC1_SR & EOC));  // Wait conversion

  uint16_t result = ADC1_DR;

  float voltage = (result * read_vref()) / 4096.0f;
  float temperature = ((1.4f - voltage) / 0.004478f) + 25.0f;

  return temperature;
}

float read_internal_temp_sensor()
{
  float internal_temp_sensor = 0;

  for (int i = 0; i < MEASEURE_MEAN; i++)
    internal_temp_sensor += read_internal_temp_sensor_util();

  return internal_temp_sensor /= (float)MEASEURE_MEAN;
}

