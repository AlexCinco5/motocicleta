#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "estado.h"
#include "entradas.h"
#include "servos.h"
#include "luces.h"
#include "odometro.h"
#include "lcd_16x2.h"

static const char *TAG = "ECU_MOTO";

void app_main(void) {
    EstadoECU ecu = {0};

    entradas_init();
    servos_init();
    luces_init();
    odometro_init(&ecu);
    lcd_init();

   int pct_acel = ((int)ecu.acelerador_raw * 100) / 4095;
    ESP_LOGI(TAG, "BAT: %.2fV (Nv %d) | IGN: %d | ACEL: %d%% | VEL: %.1f km/h | KM: %.2f",
         ecu.voltaje_bateria,
         (int)ecu.nivel_bateria,
         (int)ecu.ignicion_activa,
         pct_acel,
         ecu.velocidad_kmh,
         ecu.km_recorridos);

    uint32_t contador_ciclos = 0;
    bool prev_ignicion = false;

    for (;;) {
        // 1. Leer sensores, botones y batería
        entradas_leer(&ecu);

        // Guardar km en NVS al apagar la ignición
        if (prev_ignicion && !ecu.ignicion_activa) {
            odometro_guardar_nvs(&ecu);
        }
        prev_ignicion = ecu.ignicion_activa;

        // 2. Actualizar velocidad y kilometraje (ciclo de 20 ms = 0.02 s)
        odometro_actualizar(&ecu, 0.02f);

        // 3. Reloj de parpadeo 1 Hz (cambia cada 25 ciclos = 500 ms)
        if (contador_ciclos % 25 == 0) {
            ESP_LOGI(TAG, "BAT: %.2fV (Nv %d) | IGN: %d | ACEL: %d | VEL: %.1f | KM: %.2f",
                     ecu.voltaje_bateria, ecu.nivel_bateria, ecu.ignicion_activa,
                      ecu.velocidad_kmh, ecu.km_recorridos);
        }

        // 4. Actualizar actuadores
        servos_actualizar(&ecu);
        luces_actualizar(&ecu);

        // 5. Refrescar LCD cada 200 ms (10 ciclos)
        if (contador_ciclos % 10 == 0) {
            lcd_actualizar(&ecu);
        }

        contador_ciclos++;
        vTaskDelay(pdMS_TO_TICKS(20)); // Siempre cede el CPU a IDLE0
    }
}