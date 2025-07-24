set(SRCS "main.c" "sensor_code.c")

idf_component_register(SRCS ${SRCS}
                       INCLUDE_DIRS "."
                       REQUIRES
                                # Dependência do seu driver customizado
                                vl53l8ch_driver

                                # Dependências do ESP-IDF para o cartão SD
                                fatfs
                                sdmmc

                                # Dependência para o timer de alta resolução
                                esp_timer
                       )
