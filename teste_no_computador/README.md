# Simulador de Firmware para Sensor ToF VL53L8CH usando um modelo de ESP32

## 1. Visão Geral

Este projeto contém um programa em linguagem C projetado para simular a lógica de um firmware embarcado para o sensor Time-of-Flight (ToF) multizona VL53L8CH.

O objetivo principal desta simulação é validar e depurar o processamento de dados (parsing), a filtragem e o armazenamento de informações do sensor em um ambiente de PC, sem a necessidade do hardware físico (microcontrolador ESP32 e sensor).

A simulação utiliza como entrada um arquivo de log real, capturado de um dispositivo em funcionamento, para garantir que a lógica seja testada com dados autênticos.

## 2. Arquivos do Projeto

-   **`main.c`**: Ponto de entrada da aplicação de simulação. Responsável por iniciar o processo e chamar a lógica principal.
-   **`sensor_code.c`**: Contém o núcleo da lógica de simulação. Suas responsabilidades incluem:
    -   Ler o arquivo de log de entrada (`.log`).
    -   Processar (fazer o "parse") das strings hexadecimais para buffers de dados brutos.
    -   Imprimir os dados brutos no console para depuração, replicando a saída de uma porta serial UART.
    -   Filtrar os dados válidos e salvá-los em um arquivo de saída (`.csv`).
-   **`sensor_code.h`**: Arquivo de cabeçalho que define a interface pública da lógica de simulação, permitindo que o `main.c` a utilize.
-   **`device-monitor-250706-173207.log`**: Arquivo de log real que serve como **entrada de dados** para a simulação. Contém capturas brutas de `HEX DATA` e `TARGET STATUS` do sensor.
-   **`parse_vl53l8ch_data.py`**: (Opcional) Um script de análise em Python para pós-processamento. Ele pode ser usado para ler o arquivo de log ou o `.csv` gerado e criar visualizações, como mapas de calor.

## 3. Como Compilar e Executar

1.  **Pré-requisito**: Garanta que todos os arquivos (`main.c`, `sensor_code.c`, `sensor_code.h` e `device-monitor-250706-173207.log`) estejam na mesma pasta.

2.  **Abra um Terminal**: Navegue até o diretório do projeto.

3.  **Compile o Programa**: Execute o seguinte comando para compilar os arquivos-fonte e gerar um executável chamado `simulador_pc`:
    ```bash
    gcc main.c sensor_code.c -o simulador_pc
    ```

4.  **Execute a Simulação**:
    -   No Linux ou macOS:
        ```bash
        ./simulador_pc
        ```
    -   No Windows:
        ```bash
        .\simulador_pc.exe
        ```

O programa iniciará e começará a processar o arquivo de log em um loop contínuo. Para encerrar, pressione `Ctrl+C` no terminal.

## 4. Saída Esperada

Ao executar a simulação, você observará duas saídas:

1.  **No Terminal (Console)**: O programa imprimirá continuamente os dados brutos de `HEX DATA` e `TARGET STATUS`, imitando a saída de depuração de uma porta serial UART de um firmware real.

2.  **Arquivo de Saída**: Um novo arquivo chamado `tof_log.csv` será criado na pasta do projeto. Este arquivo simula os dados que seriam salvos em um cartão SD e conterá as medições de distância válidas (status 5 ou 9)
