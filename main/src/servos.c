#include "servos.h"
#include "pines.h"
#include "driver/ledc.h"

// PWM 50 Hz, 14 bits (0..16383). Periodo = 20,000 us.
// Calibra NEUTRO_IZQ_US y NEUTRO_DER_US si algún servo se mueve estando detenido.
#define SERVO_NEUTRO_IZQ_US     1500
#define SERVO_NEUTRO_DER_US     1500
#define SERVO_MAX_DELTA_US      450   // 1500 +/- 450 us para velocidad máxima

static uint32_t us_a_duty(int us) {
    return (uint32_t)((us * 16384) / 20000);
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
        .duty = us_a_duty(SERVO_NEUTRO_IZQ_US),
        .hpoint = 0
    };
    ledc_channel_config(&ch_izq);

    ledc_channel_config_t ch_der = {
        .gpio_num = PIN_SERVO_DER,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_0,
        .duty = us_a_duty(SERVO_NEUTRO_DER_US),
        .hpoint = 0
    };
    ledc_channel_config(&ch_der);
}

void servos_actualizar(const EstadoECU *est) {
    static int ultimo_delta = -1;
    int delta_us = 0;

    // Solo giran si hay batería conectada, ignición encendida y batería no vacía
    if (est->bateria_conectada && est->ignicion_activa && est->nivel_bateria > 0) {
        delta_us = (est->acelerador_raw * SERVO_MAX_DELTA_US) / 4095;
    }

    if (delta_us == ultimo_delta) {
        return; // No hubo cambio, evita saturar el hardware LEDC
    }
    ultimo_delta = delta_us;

    // Sentidos opuestos para avanzar hacia adelante
    int pulso_izq = SERVO_NEUTRO_IZQ_US + delta_us;
    int pulso_der = SERVO_NEUTRO_DER_US - delta_us;

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, us_a_duty(pulso_izq));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, us_a_duty(pulso_der));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}