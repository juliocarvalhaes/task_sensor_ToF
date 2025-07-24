# Versão mínima do CMake exigida pelo ESP-IDF
cmake_minimum_required(VERSION 3.16)

# Inclui as funcionalidades de build do ESP-IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Define o nome do projeto
project(task_rtos)
