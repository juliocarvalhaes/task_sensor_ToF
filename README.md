# task_sensor_ToF - Firmware e Simulador para Sensor ToF VL53L8CH com ESP32

## 1. Visão Geral

Este projeto contém o desenvolvimento de um firmware para o microcontrolador ESP32, projetado para interagir com o sensor Time-of-Flight (ToF) multizona VL53L8CH. Adicionalmente, inclui um simulador em C para PC que permite o desenvolvimento e teste da lógica de processamento de dados sem a necessidade do hardware físico.

O objetivo principal do firmware é executar uma tarefa RTOS (FreeRTOS) que realiza as seguintes operações:
1.  Lê os dados de distância (8x8 zonas) e de status do sensor VL53L8CH.
2.  Imprime os dados brutos em formato hexadecimal via UART (porta serial) para fins de depuração.
3.  Processa os dados, filtrando medições válidas (status 5 ou 9) e as salva em um arquivo `.csv` em um cartão SD.

## Estrutura do Projeto

O projeto é organizado em duas partes principais: o firmware para o ESP32 e o simulador para PC (parte extra que criei para testar).

```
/
|-- firmware/
|   |-- CMakeLists.txt
|   |-- main/
|   |   |-- main.c
|   |   |-- sensor_code.c
|   |   `-- sensor_code.h
|   `-- components/
|       `-- vl53l8ch_driver/
|           |-- CMakeLists.txt
|           `-- (Arquivos do driver do sensor aqui)
|
|-- simulation/
|   |-- main.c
|   |-- sensor_code.c
|   |-- sensor_code.h
|   `-- device-monitor-250706-173207.log
|
`-- README.md
```

-   **`/firmware`**: Contém o projeto ESP-IDF para ser compilado e gravado no microcontrolador.
-   **`/simulation`**: Contém uma versão do código em C padrão, adaptada para ser compilada e executada em um PC, usando o arquivo `.log` como entrada de dados.




### Parte do Simulador 
É gerado um arquivo `tof_log.csv` (Cartão SD / Simulação).
As medições de distância consideradas válidas (status 5 ou 9) são salvas em formato CSV.

**Formato:** `timestamp_ms,zone_id,distance_mm,status`

**Exemplo:**
```csv
1254,4,26,5
1254,18,6,9
1254,34,26,5
...
```
