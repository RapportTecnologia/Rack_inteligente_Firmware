# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

if(EXISTS "${CMAKE_SOURCE_DIR}/../env.cmake")
    include("${CMAKE_SOURCE_DIR}/../env.cmake")
    message(STATUS "Arquivo env.cmake carregado com sucesso.")
else()
    message(FATAL_ERROR "Arquivo env.cmake não encontrado!")
endif()

# Variáveis default (caso não estejam no .env)
if(NOT DEFINED ENV{WIFI_SSID})
    message(WARNING "Variável WIFI_SSID não definida no .env.")
    set(ENV{WIFI_SSID} "ArvoreDosSaberes")
endif()
if(NOT DEFINED ENV{WIFI_PASSWORD})
    message(WARNING "Variável WIFI_PASSWORD não definida no .env.")
    set(ENV{WIFI_PASSWORD} "Arduino2022")
endif()
if(NOT DEFINED ENV{MQTT_BROKER})
    message(WARNING "Variável MQTT_BROKER não definida no .env.")
    set(ENV{MQTT_BROKER} "mqtt.rapport.tec.br")
endif()
if(NOT DEFINED ENV{MQTT_USERNAME})
    message(FATAL_ERROR "Variável MQTT_USERNAME não definida no .env. Favor, defina essa variável com o nome de usuário do seu broker MQTT.")
endif()
if(NOT DEFINED ENV{MQTT_PASSWORD})
    message(FATAL_ERROR "Variável MQTT_PASSWORD não definida no .env. Favor, defina essa variável com a senha do seu broker MQTT.")
endif()
if(NOT DEFINED ENV{MQTT_BASE_TOPIC})
    message(WARNING "Variável MQTT_BASE_TOPIC não definida no .env.")
    set(ENV{MQTT_BASE_TOPIC} "rack_inteligente")
endif()
if(NOT DEFINED ENV{MQTT_RACK_NUMBER})
    message(FATAL_ERROR "Variável MQTT_RACK_NUMBER não definida no .env. Favor, defina essa variável com o número do seu rack.")
endif()

set(WIFI_SSID "$ENV{WIFI_SSID}")
set(WIFI_PASSWORD "$ENV{WIFI_PASSWORD}")
set(MQTT_BROKER "$ENV{MQTT_BROKER}")
set(MQTT_USERNAME "$ENV{MQTT_USERNAME}")
set(MQTT_PASSWORD "$ENV{MQTT_PASSWORD}")
set(MQTT_BASE_TOPIC "$ENV{MQTT_BASE_TOPIC}")
set(MQTT_RACK_NUMBER "$ENV{MQTT_RACK_NUMBER}")

message(STATUS "WIFI_SSID: ${WIFI_SSID}")
message(STATUS "WIFI_PASSWORD: ${WIFI_PASSWORD}")
message(STATUS "MQTT_USERNAME: ${MQTT_USERNAME}")
message(STATUS "MQTT_PASSWORD: ${MQTT_PASSWORD}")
message(STATUS "MQTT_BROKER: ${MQTT_BROKER}")
message(STATUS "MQTT_BASE_TOPIC: ${MQTT_BASE_TOPIC}")
message(STATUS "MQTT_RACK_NUMBER: ${MQTT_RACK_NUMBER}")

# Define the name of an MQTT broker/server to enable this example
set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "Wi-Fi SSID for examples")
set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "Wi-Fi password for examples")
set(MQTT_BROKER "${MQTT_BROKER}" CACHE INTERNAL "MQTT broker for examples")
set(MQTT_USERNAME "${MQTT_USERNAME}" CACHE INTERNAL "MQTT username for examples")
set(MQTT_PASSWORD "${MQTT_PASSWORD}" CACHE INTERNAL "MQTT password for examples")
set(MQTT_BASE_TOPIC "${MQTT_BASE_TOPIC}" CACHE INTERNAL "MQTT base topic for examples")
set(MQTT_RACK_NUMBER "${MQTT_RACK_NUMBER}" CACHE INTERNAL "MQTT rack number for examples")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(rack_inteligente C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(rack_inteligente rack_inteligente.c )

pico_set_program_name(rack_inteligente "rack_inteligente")
pico_set_program_version(rack_inteligente "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(rack_inteligente 0)
pico_enable_stdio_usb(rack_inteligente 1)

# Add the standard library to the build
target_link_libraries(rack_inteligente
        pico_stdlib
        hardware_adc)

# Add the standard include files to the build
target_include_directories(rack_inteligente PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(rack_inteligente 
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mqtt
        )

        target_compile_definitions(rack_inteligente PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        MQTT_BROKER=\"${MQTT_BROKER}\"
        MQTT_USERNAME=\"${MQTT_USERNAME}\"
        MQTT_PASSWORD=\"${MQTT_PASSWORD}\"
        MQTT_BASE_TOPIC=\"${MQTT_BASE_TOPIC}\"
        MQTT_RACK_NUMBER=\"${MQTT_RACK_NUMBER}\"
    )        
pico_add_extra_outputs(rack_inteligente)

