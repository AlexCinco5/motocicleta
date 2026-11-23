#include "lcd_16x2.h"
#include "pines.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static void lcd_pulso_e(void) {
    gpio_set_level(PIN_LCD_E, 1);
    ets_delay_us(2);
    gpio_set_level(PIN_LCD_E, 0);
    ets_delay_us(50);
}

static void lcd_enviar_nibble(uint8_t nibble) {
    gpio_set_level(PIN_LCD_D4, (nibble >> 0) & 1);
    gpio_set_level(PIN_LCD_D5, (nibble >> 1) & 1);
    gpio_set_level(PIN_LCD_D6, (nibble >> 2) & 1);
    gpio_set_level(PIN_LCD_D7, (nibble >> 3) & 1);
    lcd_pulso_e();
}

static void lcd_enviar_byte(uint8_t val, bool es_dato) {
    gpio_set_level(PIN_LCD_RS, es_dato ? 1 : 0);
    lcd_enviar_nibble(val >> 4);
    lcd_enviar_nibble(val & 0x0F);
}

static void lcd_cmd(uint8_t cmd) {
    lcd_enviar_byte(cmd, false);
    if (cmd == 0x01 || cmd == 0x02) vTaskDelay(pdMS_TO_TICKS(2));
}

static void lcd_escribir_texto(const char *str) {
    while (*str) {
        lcd_enviar_byte((uint8_t)*str++, true);
    }
}

static void lcd_cursor(uint8_t col, uint8_t fila) {
    uint8_t addr = (fila == 0) ? (0x80 + col) : (0xC0 + col);
    lcd_cmd(addr);
}

void lcd_init(void) {
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << PIN_LCD_RS) | (1ULL << PIN_LCD_E) |
                        (1ULL << PIN_LCD_D4) | (1ULL << PIN_LCD_D5) |
                        (1ULL << PIN_LCD_D6) | (1ULL << PIN_LCD_D7),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&cfg);

    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(PIN_LCD_RS, 0);
    lcd_enviar_nibble(0x03); vTaskDelay(pdMS_TO_TICKS(5));
    lcd_enviar_nibble(0x03); ets_delay_us(150);
    lcd_enviar_nibble(0x03);
    lcd_enviar_nibble(0x02); // Modo 4 bits

    lcd_cmd(0x28); // 4 bits, 2 líneas, fuente 5x8
    lcd_cmd(0x0C); // Display ON, cursor OFF
    lcd_cmd(0x06); // Incremento automático
    lcd_cmd(0x01); // Limpiar
}

void lcd_limpiar(void) {
    lcd_cmd(0x01);
}

void lcd_actualizar(const EstadoECU *est) {
    char linea1[17];
    char linea2[17];

    if (!est->bateria_conectada) {
        // Sistema apagado por falta de batería
        lcd_cursor(0, 0);
        lcd_escribir_texto("  SIN BATERIA   ");
        lcd_cursor(0, 1);
        lcd_escribir_texto("SISTEMA APAGADO ");
        return;
    }

    if (!est->ignicion_activa) {
        snprintf(linea1, sizeof(linea1), "  MOTO APAGADA  ");
        snprintf(linea2, sizeof(linea2), "KM: %06.2f      ", est->km_recorridos);
    } else {
        snprintf(linea1, sizeof(linea1), "VEL: %5.1f km/h ", est->velocidad_kmh);
        snprintf(linea2, sizeof(linea2), "KM:  %06.2f km  ", est->km_recorridos);
    }

    lcd_cursor(0, 0);
    lcd_escribir_texto(linea1);
    lcd_cursor(0, 1);
    lcd_escribir_texto(linea2);
}