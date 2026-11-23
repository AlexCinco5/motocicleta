#ifndef ESTADO_H
#define ESTADO_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    // Entradas procesadas
    bool ignicion_activa;      // Estado ON/OFF de la moto
    bool bateria_conectada;    // false si se desconecta la pila (< 1.5 V)
    float voltaje_bateria;     // Voltaje real estimado de la pila de 9 V
    uint8_t nivel_bateria;     // 0 = vacía, 1 = baja, 2 = media, 3 = llena
    uint16_t acelerador_raw;   // 0 a 4095

    // Señales de luces
    bool dir_izq_activa;
    bool dir_der_activa;
    bool intermitentes_activas;
    bool pulso_parpadeo_1hz;   // Alterna cada 500 ms

    // Cinemática aproximada
    float velocidad_kmh;       // 0.0 a 120.0 km/h aprox.
    float km_recorridos;       // Acumulado en km
} EstadoECU;

#endif