#include "entradas.h"
#include "pines.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

// Factor del divisor 100k / 22k: V_bat = V_adc * (122 / 22) = V_adc * 5.545
#define DIVISOR_FACTOR          5.545f
#define VOLTAJE_DESCONECTADA    1.5f   // Debajo de esto se considera sin batería
#define VOLTAJE_VACIA           6.5f   // 0 LEDs
#define VOLTAJE_MEDIA           7.4f   // 2 LEDs
#define VOLTAJE_LLENA           8.2f   // 3 LEDs

static adc_oneshot_unit_handle_t adc1_handle;
static bool prev_btn_ign = true;
static bool prev_btn_int = true;

void entradas_init(void) {
    // Botones con pull-up interno
    gpio_config_t btn_cfg = {
        .pin_bit_mask = (1ULL << PIN_BTN_IGNICION) | (1ULL << PIN_BTN_INTERMIT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&btn_cfg);

    // Switches de direccionales en pines solo-entrada (requieren pull-up externo 10k)
    gpio_config_t sw_cfg = {
        .pin_bit_mask = (1ULL << PIN_SW_DIR_IZQ) | (1ULL << PIN_SW_DIR_DER),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&sw_cfg);

    // Configuración ADC1
    adc_oneshot_unit_init_cfg_t unit_cfg = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&unit_cfg, &adc1_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CH_ACELERADOR, &chan_cfg);
    adc_oneshot_config_channel(adc1_handle, ADC_CH_BATERIA, &chan_cfg);
}

void entradas_leer(EstadoECU *est) {
    // 1. Leer batería (promedio de 4 muestras para estabilidad)
    int suma_bat = 0, raw_tmp = 0;
    for (int i = 0; i < 4; i++) {
        adc_oneshot_read(adc1_handle, ADC_CH_BATERIA, &raw_tmp);
        suma_bat += raw_tmp;
    }
    float v_pin = ((suma_bat / 4.0f) / 4095.0f) * 3.3f;
    est->voltaje_bateria = v_pin * DIVISOR_FACTOR;

    // Si se desconecta la batería, apagar todo el sistema
    if (est->voltaje_bateria < VOLTAJE_DESCONECTADA) {
        est->bateria_conectada = false;
        est->ignicion_activa = false;
        est->intermitentes_activas = false;
        est->dir_izq_activa = false;
        est->dir_der_activa = false;
        est->nivel_bateria = 0;
        est->acelerador_raw = 0;
        return;
    }
    est->bateria_conectada = true;

    // Nivel de batería (0 a 3 LEDs)
    if (est->voltaje_bateria < VOLTAJE_VACIA) {
        est->nivel_bateria = 0;
    } else if (est->voltaje_bateria < VOLTAJE_MEDIA) {
        est->nivel_bateria = 1;
    } else if (est->voltaje_bateria < VOLTAJE_LLENA) {
        est->nivel_bateria = 2;
    } else {
        est->nivel_bateria = 3;
    }

    // 2. Botón de ignición (flanco de bajada -> alterna ON/OFF)
    bool btn_ign = gpio_get_level(PIN_BTN_IGNICION);
    if (prev_btn_ign && !btn_ign) {
        est->ignicion_activa = !est->ignicion_activa;
    }
    prev_btn_ign = btn_ign;

    // 3. Botón de intermitentes (flanco de bajada -> alterna ON/OFF)
    bool btn_int = gpio_get_level(PIN_BTN_INTERMIT);
    if (prev_btn_int && !btn_int) {
        est->intermitentes_activas = !est->intermitentes_activas;
    }
    prev_btn_int = btn_int;

    // 4. Switches de direccionales (activos en LOW por pull-up)
    est->dir_izq_activa = (gpio_get_level(PIN_SW_DIR_IZQ) == 0);
    est->dir_der_activa = (gpio_get_level(PIN_SW_DIR_DER) == 0);

    // 5. Potenciómetro acelerador
    int raw_acel = 0;
    adc_oneshot_read(adc1_handle, ADC_CH_ACELERADOR, &raw_acel);
    // Zona muerta pequeña abajo para asegurar detención limpia
    est->acelerador_raw = (raw_acel < 150) ? 0 : (uint16_t)raw_acel;

}

