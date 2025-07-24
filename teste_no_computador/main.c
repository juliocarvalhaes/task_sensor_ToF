#include "sensor_code.h"
#include <stdio.h>

/**
 * @file main.c
 * @brief Ponto de entrada para a simulação do sensor ToF no PC.
 */

int main() {
    // Nome do arquivo de log que você forneceu.
    // Ele DEVE estar na mesma pasta que o executável.
    const char* log_file = "device-monitor-250706-173207.log";
    
    printf("Iniciando simulador...\n");

    // Inicia a simulação
    run_sensor_simulation(log_file);

    return 0; // Retorna 0 ao final
}
