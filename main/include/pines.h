#ifndef PINES_H
#define PINES_H

// Servos (PWM 50 Hz)
#define PIN_SERVO_IZQ       25
#define PIN_SERVO_DER       26

// Entradas analógicas (ADC1)
#define ADC_CH_ACELERADOR   ADC_CHANNEL_4  // GPIO 32
#define ADC_CH_BATERIA      ADC_CHANNEL_6  // GPIO 34

// Entradas digitales
#define PIN_BTN_IGNICION    33  // Pull-up interno
#define PIN_BTN_INTERMIT    27  // Pull-up interno
#define PIN_SW_DIR_IZQ      35  // Solo entrada -> Pull-up externo 10 kΩ
#define PIN_SW_DIR_DER      36  // Solo entrada -> Pull-up externo 10 kΩ

// Salidas LEDs de batería (3 colores)
#define PIN_LED_BAT_ROJO    16
#define PIN_LED_BAT_AMAR    17
#define PIN_LED_BAT_VERDE   4

// Salidas LEDs direccionales
#define PIN_LED_DIR_IZQ     19
#define PIN_LED_DIR_DER     21

// Pantalla LCD 16x2 (modo 4 bits)
#define PIN_LCD_RS          23
#define PIN_LCD_E           22
#define PIN_LCD_D4          18
#define PIN_LCD_D5          5
#define PIN_LCD_D6          13
#define PIN_LCD_D7          14

#endif