/**
 * @file sensor_code.h
 * @brief Interface para o componente de simulação do sensor ToF.
 */

#ifndef SENSOR_CODE_H
#define SENSOR_CODE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Inicia a simulação do firmware do sensor ToF.
 * @param log_filename O nome do arquivo de log de entrada a ser lido.
 * @return 0 em caso de sucesso, -1 em caso de falha.
 */
int run_sensor_simulation(const char* log_filename);

#endif // SENSOR_CODE_H
