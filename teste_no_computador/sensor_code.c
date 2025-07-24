/**
 * @file sensor_code.c
 * @brief VERSÃO PARA COMPILAR E RODAR EM UM PC 
 *
 * Este programa simula a lógica de um firmware embarcado. Ele lê dados brutos
 * de um arquivo de log disponibilizado, os imprime no console para depuração e salva os dados
 * processados em um arquivo CSV, imitando o comportamento de um sistema com cartão SD.
 */

// Bibliotecas padrão de C (não inclui nada do ESP-IDF)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

// Bloco para compatibilidade de delay entre Windows e Linux/macOS
#ifdef _WIN32
#include <windows.h>
#define msleep(msec) Sleep(msec)
#else
#include <unistd.h>
#define msleep(msec) usleep(msec * 1000)
#endif

#include "sensor_code.h"


static inline const char* get_log_timestamp() {
    static char buffer[10];
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    if (t) {
        strftime(buffer, sizeof(buffer), "%H:%M:%S", t);
    }
    return buffer;
}

#define ESP_LOGI(tag, format, ...) printf("%s I (%s): " format "\n", get_log_timestamp(), tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) printf("%s W (%s): " format "\n", get_log_timestamp(), tag, ##__VA_ARGS__)
#define ESP_LOGE(tag, format, ...) printf("%s E (%s): " format "\n", get_log_timestamp(), tag, ##__VA_ARGS__)
#define ESP_LOGD(tag, format, ...) printf("%s D (%s): " format "\n", get_log_timestamp(), tag, ##__VA_ARGS__)


static const char *TAG = "TOF_SIM";
#define OUTPUT_CSV_FILE "tof_log.csv"
#define SENSOR_POLLING_RATE_MS 200
#define SENSOR_DATA_BUFFER_SIZE 64
#define LOG_LINE_MAX_LEN 256

static FILE* g_log_file = NULL;


static bool simulation_init(const char* log_filename);
static void simulation_deinit(void);
static bool get_sensor_data_from_log(uint8_t* dist_buf, uint8_t* status_buf);
static void print_raw_data_as_hex(const char* prefix, uint8_t* buffer, size_t len);
static void save_data_to_csv(uint8_t* dist_buf, uint8_t* status_buf);
static int hex_char_to_int(char c);
static bool hex_string_to_bytes(const char* hex_str, uint8_t* byte_array, size_t array_len);
static long long get_simulated_timestamp_ms();

// =========================================================================
// IMPLEMENTAÇÃO DA LÓGICA PRINCIPAL
// =========================================================================

int run_sensor_simulation(const char* log_filename) {
    ESP_LOGI(TAG, "Iniciando simulação do firmware do sensor ToF.");

    if (!simulation_init(log_filename)) {
        return -1;
    }

    uint8_t distance_data[SENSOR_DATA_BUFFER_SIZE];
    uint8_t status_data[SENSOR_DATA_BUFFER_SIZE];

    while (1) {
        if (get_sensor_data_from_log(distance_data, status_data)) {
            ESP_LOGD(TAG, "Par de dados lido do log com sucesso.");
            print_raw_data_as_hex("TOF: HEX DATA", distance_data, sizeof(distance_data));
            print_raw_data_as_hex("TOF: TARGET STATUS", status_data, sizeof(status_data));
            save_data_to_csv(distance_data, status_data);
        } else {
            ESP_LOGW(TAG, "Fim do arquivo de log alcançado. Reiniciando a leitura para loop contínuo.");
            rewind(g_log_file); 
        }
        msleep(SENSOR_POLLING_RATE_MS);
    }

    simulation_deinit();
    return 0;
}

// =========================================================================
// IMPLEMENTAÇÃO DAS FUNÇÕES AUXILIARES
// =========================================================================

static bool simulation_init(const char* log_filename) {
    ESP_LOGI(TAG, "Abrindo arquivo de log de entrada: %s", log_filename);
    g_log_file = fopen(log_filename, "r");
    if (g_log_file == NULL) {
        ESP_LOGE(TAG, "ERRO: Nao foi possivel abrir o arquivo de log! Verifique se '%s' esta na mesma pasta.", log_filename);
        return false;
    }

    FILE* f = fopen(OUTPUT_CSV_FILE, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "ERRO: Nao foi possivel criar o arquivo de saida %s", OUTPUT_CSV_FILE);
        fclose(g_log_file);
        return false;
    }
    fprintf(f, "timestamp_ms,zone_id,distance_mm,status\n");
    fclose(f);

    ESP_LOGI(TAG, "Simulador inicializado. Pressione Ctrl+C para encerrar.");
    return true;
}

static void simulation_deinit(void) {
    if (g_log_file) {
        fclose(g_log_file);
    }
}

static bool get_sensor_data_from_log(uint8_t* dist_buf, uint8_t* status_buf) {
    char line[LOG_LINE_MAX_LEN];
    char* hex_data_ptr = NULL;

    while (fgets(line, sizeof(line), g_log_file)) {
        if ((hex_data_ptr = strstr(line, "TOF: HEX DATA:"))) {
            hex_data_ptr += strlen("TOF: HEX DATA:");
            while (*hex_data_ptr == ' ') hex_data_ptr++;
            if (!hex_string_to_bytes(hex_data_ptr, dist_buf, SENSOR_DATA_BUFFER_SIZE)) continue;

            if (fgets(line, sizeof(line), g_log_file)) {
                if ((hex_data_ptr = strstr(line, "TOF: TARGET STATUS:"))) {
                    hex_data_ptr += strlen("TOF: TARGET STATUS:");
                    while (*hex_data_ptr == ' ') hex_data_ptr++;
                    if (!hex_string_to_bytes(hex_data_ptr, status_buf, SENSOR_DATA_BUFFER_SIZE)) continue;
                    return true;
                }
            }
        }
    }
    return false;
}

static void print_raw_data_as_hex(const char* prefix, uint8_t* buffer, size_t len) {
    printf("%s: \t", prefix);
    for (size_t i = 0; i < len; i++) {
        printf("%02X", buffer[i]);
    }
    printf("\n");
}

static void save_data_to_csv(uint8_t* dist_buf, uint8_t* status_buf) {
    FILE* f = fopen(OUTPUT_CSV_FILE, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Falha ao abrir o arquivo CSV para escrita.");
        return;
    }
    long long timestamp = get_simulated_timestamp_ms();
    for (int i = 0; i < SENSOR_DATA_BUFFER_SIZE; i++) {
        if (status_buf[i] == 5 || status_buf[i] == 9) {
            fprintf(f, "%lld,%d,%d,%d\n", timestamp, i, dist_buf[i], status_buf[i]);
        }
    }
    fclose(f);
}

static int hex_char_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool hex_string_to_bytes(const char* hex_str, uint8_t* byte_array, size_t array_len) {
    size_t hex_len = strlen(hex_str);
    if (hex_str[hex_len - 1] == '\n' || hex_str[hex_len - 1] == '\r') {
        hex_len--;
    }
    if (hex_str[hex_len - 1] == '\r') {
        hex_len--;
    }
    if (hex_len != array_len * 2) {
        ESP_LOGE(TAG, "Comprimento de string HEX invalido. Esperado: %zu, Recebido: %zu", array_len * 2, hex_len);
        return false;
    }

    for (size_t i = 0; i < array_len; i++) {
        int high = hex_char_to_int(hex_str[2 * i]);
        int low = hex_char_to_int(hex_str[2 * i + 1]);
        if (high == -1 || low == -1) {
            return false;
        }
        byte_array[i] = (uint8_t)((high << 4) | low);
    }
    return true;
}

static long long get_simulated_timestamp_ms() {
    return (long long)(clock() * 1000 / CLOCKS_PER_SEC);
}
