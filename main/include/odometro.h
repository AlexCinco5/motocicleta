#ifndef ODOMETRO_H
#define ODOMETRO_H

#include "estado.h"

void odometro_init(EstadoECU *est);
void odometro_actualizar(EstadoECU *est, float dt_seg);
void odometro_guardar_nvs(const EstadoECU *est);

#endif