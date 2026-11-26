#include "luces.h"
#include "pines.h"
#include "driver/gpio.h"

void luces_init(void) {
    gpio_config_t out_cfg = {
        .pin_bit_mask = (1ULL << PIN_LED_BAT_ROJO) |
                        (1ULL << PIN_LED_BAT_AMAR) |
                        (1ULL << PIN_LED_BAT_VERDE) |
                        (1ULL << PIN_LED_DIR_IZQ) |
                        (1ULL << PIN_LED_DIR_DER),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&out_cfg);

    gpio_set_level(PIN_LED_BAT_ROJO, 0);
    gpio_set_level(PIN_LED_BAT_AMAR, 0);
    gpio_set_level(PIN_LED_BAT_VERDE, 0);
    gpio_set_level(PIN_LED_DIR_IZQ, 0);
    gpio_set_level(PIN_LED_DIR_DER, 0);
}

void luces_actualizar(const EstadoECU *est) {
    // Si se desconecta la batería, apagar todos los LEDs
    if (!est->bateria_conectada) {
        gpio_set_level(PIN_LED_BAT_ROJO, 0);
        gpio_set_level(PIN_LED_BAT_AMAR, 0);
        gpio_set_level(PIN_LED_BAT_VERDE, 0);
        gpio_set_level(PIN_LED_DIR_IZQ, 0);
        gpio_set_level(PIN_LED_DIR_DER, 0);
        return;
    }

    // 3 LEDs de batería: 0=apagados, 1=rojo, 2=rojo+amarillo, 3=todos prendidos
    gpio_set_level(PIN_LED_BAT_ROJO,  (est->nivel_bateria >= 1) ? 1 : 0);
    gpio_set_level(PIN_LED_BAT_AMAR,  (est->nivel_bateria >= 2) ? 1 : 0);
    gpio_set_level(PIN_LED_BAT_VERDE, (est->nivel_bateria >= 3) ? 1 : 0);

    // Direccionales e intermitentes (parpadeo a 1 Hz)
    bool izq = false;
    bool der = false;

    if (est->intermitentes_activas) {
        izq = est->pulso_parpadeo_1hz;
        der = est->pulso_parpadeo_1hz;
    } else if (est->ignicion_activa) {
        if (est->dir_izq_activa) izq = est->pulso_parpadeo_1hz;
        if (est->dir_der_activa) der = est->pulso_parpadeo_1hz;
    }

    gpio_set_level(PIN_LED_DIR_IZQ, izq ? 1 : 0);
    gpio_set_level(PIN_LED_DIR_DER, der ? 1 : 0);
}