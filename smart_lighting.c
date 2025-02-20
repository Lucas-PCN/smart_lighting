#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdlib.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "ssd1306.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"

// Definições dos pinos
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define LED_G_PIN 11
#define BTN_A_PIN 5
#define BTN_B_PIN 6
#define JOY_X_PIN 27
#define JOY_Y_PIN 26
#define JOY_BTN_PIN 22
#define ADC_CHANNEL_0 0 // Canal ADC para o eixo X do joystick
#define ADC_CHANNEL_1 1 // Canal ADC para o eixo Y do joystick

// Config para a conexão do wifi
#define WIFI_SSID "SUA_REDE_WIFI" // Nome da rede Wi-Fi
#define WIFI_PASSWORD "SUA_SENHA"        // Senha da rede Wi-Fi

// Config para o Thingspeak
#define THINGSPEAK_HOST "api.thingspeak.com"
#define THINGSPEAK_PORT 80
#define API_KEY "Q69864I9JVW7LER1" // API Key do ThingSpeak

// Global
static struct tcp_pcb *tcp_client_pcb;
char request[256]; // Buffer para o HTTP GET

// Configurações do display OLED
const uint I2C_SDA = I2C_SDA_PIN;
const uint I2C_SCL = I2C_SCL_PIN;

// Buffer do display OLED
uint8_t ssd_buffer[ssd1306_buffer_length];
struct render_area frame_area;

// Estado da luz
int luz_ligada = 0;

int contador_luzes = 0;  // Contador de quantas vezes a luz foi acesa

// Função para exibir mensagens no OLED
void display_messages(const char *messages[], int count) {
    memset(ssd_buffer, 0, ssd1306_buffer_length); // Limpa o buffer

    int y = 0; // Posição vertical inicial
    for (int i = 0; i < count; i++) {
        ssd1306_draw_string(ssd_buffer, 5, y, (char*)messages[i]); // Desenha cada linha
        y += 8; // Move para a próxima linha
    }

    // Renderiza o conteúdo no display
    render_on_display(ssd_buffer, &frame_area);
}

void exibir_instrucoes() {
    const char *instrucao[] = {
        "Bem vindo",
        "A liga desliga",
        "B sensor de luz",
        "Joystick",
        "Sensor de mov.",
        "e enviar dados"};
    display_messages(instrucao, 6);
}

// Callback quando dados são recebidos
err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        printf("Conexão encerrada pelo servidor.\n");

        const char *mensagem[] = {"dados enviados", "com sucesso"};
        display_messages(mensagem, 2);
        sleep_ms(2000);

        exibir_instrucoes();

        tcp_close(tpcb);
        return ERR_OK;
    }

    char buffer[512]; // Buffer maior para evitar overflow
    size_t copy_len = (p->len < sizeof(buffer) - 1) ? p->len : sizeof(buffer) - 1;
    pbuf_copy_partial(p, buffer, copy_len, 0);
    buffer[copy_len] = '\0'; 
    //printf("Recebido: %s\n", buffer);

    pbuf_free(p);
    return ERR_OK;
}

// Callback quando a conexão é estabelecida
err_t tcp_connect_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        printf("Falha ao conectar ao servidor.\n");
        return err;
    }

    printf("Conectado ao servidor.\n");
    
    // Enviar a requisição HTTP GET ao ThingSpeak com o valor recebido
    snprintf(request, sizeof(request),
             "GET /update?api_key=%s&field1=%d HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n",
             API_KEY, contador_luzes, THINGSPEAK_HOST);

    tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    tcp_recv(tpcb, tcp_recv_callback);
    return ERR_OK;
}

// Callback de resolução DNS
static void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr == NULL) {
        printf("Falha ao resolver DNS para %s\n", name);
        return;
    }

    printf("Resolvido %s para %s\n", name, ipaddr_ntoa(ipaddr));
    tcp_connect(tcp_client_pcb, ipaddr, THINGSPEAK_PORT, tcp_connect_callback);
}

// Configuração do Wi-Fi
void wifi_setup() {
    const char *mensagem[] = {"Conectando", "ao wi fi"};
    display_messages(mensagem, 2);

    if (cyw43_arch_init()) {
        printf("Falha ao inicializar o módulo Wi-Fi.\n");
        const char *mensagem2[] = {"Falha", "ao conectar"};
        display_messages(mensagem2, 2);
        return;
    }

    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Não foi possível encontrar a rede Wi-Fi.\n");
        const char *mensagem3[] = {"Rede", "nao encontrada"};
        display_messages(mensagem3, 2);
        cyw43_arch_poll();
        cyw43_arch_deinit();
        sleep_ms(1000);
        wifi_setup();
    }

    printf("Wi-Fi conectado com sucesso.\n");

    const char *mensagem4[] = {"wi fi", "conectado", "com sucesso"};
    display_messages(mensagem4, 3);
    sleep_ms(2000);
}

// Função para enviar dados ao ThingSpeak
void enviar_para_thingspeak(int contador) {
    const char *mensagem1[] = {"enviando dados", "para o", "Thingspeak"};
    display_messages(mensagem1, 3);
    sleep_ms(2000);

    tcp_client_pcb = tcp_new();
    if (!tcp_client_pcb) {
        const char *mensagem2[] = {"falha ao", "criar TCP", "PCB"};
        display_messages(mensagem2, 3);

        printf("Falha ao criar o TCP PCB.\n");
        return;
    }

    // Atualiza o valor a ser enviado
    contador_luzes = contador;

    printf("Resolvendo %s...\n", THINGSPEAK_HOST);
    ip_addr_t server_ip;
    err_t err = dns_gethostbyname(THINGSPEAK_HOST, &server_ip, dns_callback, NULL);
    if (err == ERR_OK) {
        printf("DNS resolvido imediatamente: %s\n", ipaddr_ntoa(&server_ip));
        tcp_connect(tcp_client_pcb, &server_ip, THINGSPEAK_PORT, tcp_connect_callback);
    } else if (err != ERR_INPROGRESS) {
        printf("Falha na resolução DNS: %d\n", err);
        tcp_close(tcp_client_pcb);
    }
}

// Função para incrementar o contador quando a luz for acesa
void incrementar_contador() {
    contador_luzes++;
    printf("A luz foi acesa %d vezes\n", contador_luzes);
}

// Inicializa o display OLED
void init_oled() {
    i2c_init(i2c1, 100 * 1000); // Configura a comunicação I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(); // Inicializa o display OLED

    // Configura a área de renderização
    frame_area.start_column = 0;
    frame_area.end_column = ssd1306_width - 1;
    frame_area.start_page = 0;
    frame_area.end_page = ssd1306_n_pages - 1;

    calculate_render_area_buffer_length(&frame_area);

    // Limpa o display
    memset(ssd_buffer, 0, ssd1306_buffer_length);
    render_on_display(ssd_buffer, &frame_area);
}

// Função para debounce do botão
bool debounce_button(uint gpio) {
    static uint32_t last_ms = 0;
    const uint32_t debounce_delay = 50; // Tempo de debounce em milissegundos

    if (gpio_get(gpio) == 0) { // Botão pressionado
        if (time_us_32() - last_ms > debounce_delay * 1000) {
            last_ms = time_us_32() / 1000;
            return true; // Retorna verdadeiro apenas uma vez após o debounce
        }
    } else {
        last_ms = 0; // Reseta o tempo se o botão for liberado
    }

    return false;
}

// Controle da iluminação
void alternar_luz() {
    luz_ligada = !luz_ligada;
    gpio_put(LED_G_PIN, luz_ligada);
    if (luz_ligada) {
        incrementar_contador(); // Incrementa o contador quando a luz for acesa
        const char *messages[] = {"Luz Acesa", "Ambiente", "Iluminado"};
        display_messages(messages, 3);
    } else {
        const char *messages[] = {"Luz Apagada"};
        display_messages(messages, 1);
    }
}

// Simula um sensor de luminosidade
void simular_sensor_luminosidade() {
    const char *mensagem_avaliacao[] = {"Sensor", "de luminosidade", "Avaliando", "ambiente"};
    display_messages(mensagem_avaliacao, 4);
    sleep_ms(2000);

    // Gera um valor aleatório entre 0 e 100 (simulando um sensor real)
    int luminosidade = rand() % 101; // Valor entre 0 e 100

    if (luminosidade < 40) { // Considera escuro se for menor que 40
        if (!luz_ligada) {
            luz_ligada = 1; // Liga a luz
            gpio_put(LED_G_PIN, luz_ligada);

            incrementar_contador(); // Incrementa o contador quando a luz for acesa
        }
        const char *mensagem_luz[] = {"Estava escuro", "Luz acesa"};
        display_messages(mensagem_luz, 2);
    } else { // Ambiente claro
        if (luz_ligada) {
            luz_ligada = 0; // Desliga a luz
            gpio_put(LED_G_PIN, luz_ligada);
        }
        const char *mensagem_economia[] = {"Esta claro", "Luz apagada", "Economia", "de energia"};
        display_messages(mensagem_economia, 4);
    }
}

// Simula um sensor de movimento
void simular_sensor_movimento() {
    const char *messages[] = {"Sensor", "de movimento", "detectado"};
    display_messages(messages, 3);
    sleep_ms(2000);

    if (luz_ligada) {
        const char *messagem_luz_acesa[] = {"Luz acesa", "Desligando em", "5 segundos"};
        display_messages(messagem_luz_acesa, 3);
        sleep_ms(5000);

        gpio_put(LED_G_PIN, !luz_ligada);
        luz_ligada = 0;
        const char *messagem_luz_apagada[] = {"Sem movimentos", "no ambiente", "luz apagada"};
        display_messages(messagem_luz_apagada, 3);
    } else {
        luz_ligada = 1; // Liga a luz
        gpio_put(LED_G_PIN, luz_ligada);

        incrementar_contador(); // Incrementa o contador quando a luz for acesa

        const char *messagem_luz_acesa[] = {"Luz acesa", "Desligando em", "5 segundos"};
        display_messages(messagem_luz_acesa, 3);
        sleep_ms(5000);

        gpio_put(LED_G_PIN, !luz_ligada);
        luz_ligada = 0;
        const char *messagem_luz_apagada[] = {"Sem movimentos", "no ambiente", "luz apagada"};
        display_messages(messagem_luz_apagada, 3);
    }
}

int main() {
    printf("Iniciando o programa...\n");
    stdio_init_all();

    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);

    // Configuração do botão do joystick
    gpio_init(JOY_BTN_PIN);
    gpio_set_dir(JOY_BTN_PIN, GPIO_IN);
    gpio_pull_up(JOY_BTN_PIN);

    // Configuração dos botões A e B
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);

    adc_init();
    adc_gpio_init(JOY_Y_PIN);
    adc_gpio_init(JOY_X_PIN);

    // Inicialização do OLED
    init_oled();

    wifi_setup();  // Inicializa o Wi-Fi

    // Exibe instruções iniciais
    exibir_instrucoes();

    while (true) {
        if (!gpio_get(BTN_A_PIN)) {
            alternar_luz();
            sleep_ms(500);
        }

        if (!gpio_get(BTN_B_PIN)) {
            simular_sensor_luminosidade();
            sleep_ms(500);
        }

        if (debounce_button(JOY_BTN_PIN)) {
            enviar_para_thingspeak(contador_luzes);
        }

        adc_select_input(1);
        uint adc_y_raw = adc_read();
        adc_select_input(0);
        uint adc_x_raw = adc_read();

        if (adc_x_raw > 3500 || adc_x_raw < 500 || adc_y_raw > 3500 || adc_y_raw < 500) {
            simular_sensor_movimento();
            sleep_ms(500);
        }
    }
    return 0;
}