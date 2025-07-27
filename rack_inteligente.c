/* -------------------------------------------------------------------------------------------------------------------------------------
/ Projeto: Botão MQTT
/ Descrição: Este código lê o estado de um botão, enviando o status para um broker MQTT.
/ Bibliotecas: pico-sdk, lwIP, CYW43
/ Autor: José Adriano
/ Obs_1: A parte do DNS foi adaptada de códigos de exemplos encontrado na internet.
/ Obs_2: Necessário efetuar ajustes nos arquivos CMakeLists.txt e lwipopts.h para compilar corretamente.
/ Data de Criação: 22/06/2025
/----------------------------------------------------------------------------------------------------------------------------------------
*/
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"
#include "rack_inteligente.h"

#define MQTT_BROKER_PORT 1883

//#define MQTT_TOPIC "pico/button"
#define MQTT_TOPIC  MQTT_BASE_TOPIC"/"MQTT_RACK_NUMBER

// Configurações do Botão
#define RACK_PORT_STATE 5

/* Choose 'C' for Celsius or 'F' for Fahrenheit. */
#define TEMPERATURE_UNITS 'C'

// Variáveis Globais
static mqtt_client_t *mqtt_client;
static ip_addr_t broker_ip;
static bool mqtt_connected = false;
static bool last_rack_door_state = false;
static float last_rack_temperature = -1.0f;

// Protótipos de Funções
static void mqtt_connection_callback(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

float read_rack_temperature(const char unit);

void publish_door_state(bool pressed);
void publish_rack_temperature(float temperature);

void dns_check_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg);

// Função Principal
int main() {
    stdio_init_all();
    /* Initialize hardware AD converter, enable onboard temperature sensor and
     *   select its channel (do this once for efficiency, but beware that this
     *   is a global operation). */
     adc_init();
     adc_set_temp_sensor_enabled(true);
     adc_select_input(4);
 
    sleep_ms(2000);
    printf("\n=== Iniciando MQTT Button Monitor ===\n");

    // Inicializa Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro na inicialização do Wi-Fi\n");
        return -1;
    }
    cyw43_arch_enable_sta_mode();

    printf("[Wi-Fi] Conectando...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("[Wi-Fi] Falha na conexão Wi-Fi\n");
        return -1;
    } else {
        printf("[Wi-Fi] Conectado com sucesso!\n");
    }

    // Configura GPIO do botão
    gpio_init(RACK_PORT_STATE);
    gpio_set_dir(RACK_PORT_STATE, GPIO_IN);
    gpio_pull_up(RACK_PORT_STATE); // <<< ATENÇÃO: pull-up ativado

    // Inicializa cliente MQTT
    mqtt_client = mqtt_client_new();

    // Resolve DNS do broker MQTT
    err_t err = dns_gethostbyname(MQTT_BROKER, &broker_ip, dns_check_callback, NULL);
    if (err == ERR_OK) {
        dns_check_callback(MQTT_BROKER, &broker_ip, NULL);
    } else if (err == ERR_INPROGRESS) {
        printf("[DNS] Resolvendo...\n");
    } else {
        printf("[DNS] Erro ao resolver DNS: %d\n", err);
        return -1;
    }

    // Loop principal
    while (true) {
        // Atualiza tarefas de rede
        cyw43_arch_poll();

        // Lê o estado do botão
        bool rack_port_state = !gpio_get(RACK_PORT_STATE); // <<< Inverte porque é pull-up

        // Se mudou de estado, publica
        if (rack_port_state != last_rack_door_state) {
            printf("[BOTÃO] Estado mudou para: %s\n", rack_port_state ? "ON" : "OFF");
            publish_door_state(rack_port_state);
            last_rack_door_state = rack_port_state;
        }

        // Lê a temperatura do rack
        float rack_temperature = read_rack_temperature(TEMPERATURE_UNITS);
        if (rack_temperature != last_rack_temperature) {
            printf("[TEMPERATURA] Temperatura mudou para: %.2f\n", rack_temperature);
            publish_rack_temperature(rack_temperature);
            last_rack_temperature = rack_temperature;
        }

        sleep_ms(1000); // Ajuste conforme desejado
    }

    // Finaliza (nunca chega aqui)
    cyw43_arch_deinit();
    return 0;
}

/* References for this implementation:
 * raspberry-pi-pico-c-sdk.pdf, Section '4.1.1. hardware_adc'
 * pico-examples/adc/adc_console/adc_console.c */
float read_rack_temperature(const char unit) {
    
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);

    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    if (unit == 'C') {
        return tempC;
    } else if (unit == 'F') {
        return tempC * 9 / 5 + 32;
    }

    return -1.0f;
}

// Callback de conexão MQTT
static void mqtt_connection_callback(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("[MQTT] Conectado ao broker!\n");
        mqtt_connected = true;
    } else {
        printf("[MQTT] Falha na conexão MQTT. Código: %d\n", status);
        mqtt_connected = false;
    }
}

void publish_rack_temperature(float temperature) {
    if (!mqtt_connected) {
        printf("[MQTT] Não conectado, não publicando temperatura do rack\n");
        return;
    }
    const char *topic_rack_temperature = MQTT_TOPIC "/temperature";

    char message[16];
    snprintf(message, sizeof(message), "%.2f", temperature);

    printf("[MQTT] Publicando: tópico='%s', mensagem='%s'\n", topic_rack_temperature, message);

    err_t err = mqtt_publish(mqtt_client, topic_rack_temperature, message, strlen(message), 0, 0, NULL, NULL);

    if (err == ERR_OK) {
        printf("[MQTT] Publicação enviada com sucesso\n");
    } else {
        printf("[MQTT] Erro ao publicar: %d\n", err);
    }
}

void publish_door_state(bool pressed) {
    if (!mqtt_connected) {
        printf("[MQTT] Não conectado, não publicando estado da porta\n");
        return;
    }
    const char *topic_door_state = MQTT_TOPIC"/door";

    const char *message = pressed ? "ON" : "OFF";

    printf("[MQTT] Publicando: tópico='%s', mensagem='%s'\n", topic_door_state, message);

    err_t err = mqtt_publish(mqtt_client, topic_door_state, message, strlen(message), 0, 0, NULL, NULL);

    if (err == ERR_OK) {
        printf("[MQTT] Publicação enviada com sucesso\n");
    } else {
        printf("[MQTT] Erro ao publicar: %d\n", err);
    }
}

// Callback de DNS
void dns_check_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr != NULL) {
        broker_ip = *ipaddr;
        printf("[DNS] Resolvido: %s -> %s\n", name, ipaddr_ntoa(ipaddr));

        struct mqtt_connect_client_info_t ci = {
            .client_id = "pico-client",
            .keep_alive = 60,
            .client_user = NULL,
            .client_pass = NULL,
            .will_topic = NULL,
            .will_msg = NULL,
            .will_qos = 0,
            .will_retain = 0
        };

        printf("[MQTT] Conectando ao broker...\n");
        mqtt_client_connect(mqtt_client, &broker_ip, MQTT_BROKER_PORT, mqtt_connection_callback, NULL, &ci);
    } else {
        printf("[DNS] Falha ao resolver DNS para %s\n", name);
    }
}