#ifndef SERVOS_H
#define SERVOS_H

#include "estado.h"

void servos_init(void);
void servos_actualizar(const EstadoECU *est);

#endif