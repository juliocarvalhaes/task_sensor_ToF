/**
 * @file sensor_code.h
 * @brief Interface pública para o componente do sensor de ToF VL53L8CH.
 *
 * Este cabeçalho expõe as funções necessárias para inicializar e gerenciar a tarefa
 * de aquisição de dados do sensor Time-of-Flight.
 */

#ifndef SENSOR_CODE_H
#define SENSOR_CODE_H

/**
 * @brief Inicializa e cria a tarefa RTOS para o sensor VL53L8CH.
 *
 * Esta função aloca os recursos e inicia uma nova tarefa FreeRTOS dedicada
 * ao gerenciamento do sensor. A tarefa é responsável por todo o ciclo de vida
 * da aquisição de dados, incluindo inicialização do hardware, medições periódicas,
 * logging para depuração via UART e persistência dos dados em um cartão SD.
 *
 * @param None
 * @return None
 */
void start_tof_sensor_task(void);

#endif // TOF_SENSOR_H