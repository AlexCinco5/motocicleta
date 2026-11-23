#include "odometro.h"
#include "nvs_flash.h"
#include "nvs.h"

#define VELOCIDAD_MAX_KMH   120.0f
// Factor de escala para demo (10.0 hace que los km avancen visiblemente en clase)
#define FACTOR_ESCALA_DEMO  10.0f

void odometro_init(EstadoECU *est) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    nvs_handle_t h;
    est->km_recorridos = 0.0f;
    if (nvs_open("ecu_moto", NVS_READONLY, &h) == ESP_OK) {
        uint32_t km_x100 = 0;
        if (nvs_get_u32(h, "km_x100", &km_x100) == ESP_OK) {
            est->km_recorridos = km_x100 / 100.0f;
        }
        nvs_close(h);
    }
}

void odometro_actualizar(EstadoECU *est, float dt_seg) {
    if (est->bateria_conectada && est->ignicion_activa && est->nivel_bateria > 0) {
        est->velocidad_kmh = ((float)est->acelerador_raw / 4095.0f) * VELOCIDAD_MAX_KMH;
        // distancia = velocidad (km/h) * tiempo (h) * factor_demo
        est->km_recorridos += (est->velocidad_kmh * (dt_seg / 3600.0f)) * FACTOR_ESCALA_DEMO;
    } else {
        est->velocidad_kmh = 0.0f;
    }
}

void odometro_guardar_nvs(const EstadoECU *est) {
    nvs_handle_t h;
    if (nvs_open("ecu_moto", NVS_READWRITE, &h) == ESP_OK) {
        uint32_t km_x100 = (uint32_t)(est->km_recorridos * 100.0f);
        nvs_set_u32(h, "km_x100", km_x100);
        nvs_commit(h);
        nvs_close(h);
    }
}