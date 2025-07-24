# Lista de arquivos fonte do driver (adicione os arquivos .c reais aqui quando os tiver)
# Por agora, pode estar vazio ou com um arquivo placeholder se necessário.
set(SRC_FILES "")

# Registra o diretório como um componente chamado "vl53l8ch_driver"
idf_component_register(SRCS ${SRC_FILES}
                       INCLUDE_DIRS ".") # Expõe o diretório atual para includes
