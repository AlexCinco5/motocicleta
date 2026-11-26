#include "servos.h"
#include "pines.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// PWM 50 Hz, 14 bits (0..16383). Periodo = 20,000 us.
#define SERVO_NEUTRO_US         1500
#define SERVO_RANGO_OSC_US      400   // Oscila entre 1100 us y 1900 us (~ +/- 36 deg)
#define PASO_US                 25    // Incremento de pulso por cada paso de oscilación

static uint32_t us_a_duty(int us) {
    return (uint32_t)((us * 16384) / 20000);
}

static void aplicar_pulsos(int pulso_izq, int pulso_der) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, us_a_duty(pulso_izq));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, us_a_duty(pulso_der));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}

void servos_init(void) {
    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_14_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 50,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_cfg);

    ledc_channel_config_t ch_izq = {
        .gpio_num = PIN_SERVO_IZQ,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = us_a_duty(SERVO_NEUTRO_US),
        .hpoint = 0
    };
    ledc_channel_config(&ch_izq);

    ledc_channel_config_t ch_der = {
        .gpio_num = PIN_SERVO_DER,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_0,
        .duty = us_a_duty(SERVO_NEUTRO_US),
        .hpoint = 0
    };
    ledc_channel_config(&ch_der);
}

void servos_actualizar(const EstadoECU *est) {
    static int offset_us = 0;
    static int direccion = 1; // 1 sube hacia +RANGO, -1 baja hacia -RANGO
    static TickType_t ultimo_paso = 0;

    bool activo = est->bateria_conectada && est->ignicion_activa && (est->nivel_bateria > 0);
    int acel = est->acelerador_raw;
    if (acel < 0) acel = 0;
    if (acel > 4095) acel = 4095;

    // Si está apagado o el acelerador está prácticamente en 0, regresa a neutro
    if (!activo || acel < 80) {
        if (offset_us != 0) {
            offset_us = 0;
            direccion = 1;
            aplicar_pulsos(SERVO_NEUTRO_US, SERVO_NEUTRO_US);
        }
        return;
    }

    // A mayor acelerador, menor tiempo entre pasos (de ~80 ms lento a ~15 ms rápido)
    int intervalo_ms = 80 - ((acel * 65) / 4095);
    if (intervalo_ms < 15) intervalo_ms = 15;

    TickType_t ahora = xTaskGetTickCount();
    if ((ahora - ultimo_paso) < pdMS_TO_TICKS(intervalo_ms)) {
        return;
    }
    ultimo_paso = ahora;

    offset_us += direccion * PASO_US;
    if (offset_us >= SERVO_RANGO_OSC_US) {
        offset_us = SERVO_RANGO_OSC_US;
        direccion = -1;
    } else if (offset_us <= -SERVO_RANGO_OSC_US) {
        offset_us = -SERVO_RANGO_OSC_US;
        direccion = 1;
    }

    // Sentidos opuestos para simular giro acoplado
    int pulso_izq = SERVO_NEUTRO_US + offset_us;
    int pulso_der = SERVO_NEUTRO_US - offset_us;
    aplicar_pulsos(pulso_izq, pulso_der);
}