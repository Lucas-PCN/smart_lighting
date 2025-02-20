# Projeto Final - Curso de Capacitação em Sistemas Embarcados

## Descrição do Projeto
Este projeto foi desenvolvido como parte da avaliação final do curso de Capacitação em Sistemas Embarcados, utilizando a placa **BitDogLab**. O objetivo é aplicar os conhecimentos adquiridos durante o curso, incluindo **programação em C, comunicação entre dispositivos, IoT e boas práticas de desenvolvimento de software embarcado**.

## Objetivo
O projeto visa criar uma **solução IoT (Internet das Coisas)** para controle e monitoramento de iluminação, utilizando sensores, botoes, joystick e comunicação Wi-Fi para envio de dados ao **ThingSpeak**. O sistema permite interação por meio do **display OLED, LED RGB e botões**, além de simular a coleta de informações do ambiente e armazená-las em um serviço na nuvem.

## Funcionalidades
- **Interação com múltiplos periféricos:**
  - **Joystick** para simulação de um sensor de movimento e para envio do dados para o ThingSpeak.
  - **Botões físicos** para acionamento do LED e simulação do sensor de luminosidade.
- **Comunicacão via Wi-Fi:**
  - Conexão com redes Wi-Fi para envio de dados.
  - Upload de informações ao **ThingSpeak**.
- **Protocolo de comunicação:**
  - Utiliza **I2C** para comunicação com o display OLED.
  - Utiliza **ADC** para leitura do joystick.
- **Interface intuitiva:**
  - Instruções exibidas no **display OLED**.
  - Indicação de status por meio de **LEDs**.
- **Contabiliza a quantidade de acionamentos da luz** e transmite ao servidor.

## Tecnologias e Componentes Utilizados
- **BitDogLab** com **Raspberry Pi Pico W**.
- **Linguagem C** para programação do firmware.
- **SSD1306** para exibição no display OLED.
- **Matriz de LEDs RGB** para sinais visuais.
- **Joystick e botões** para entrada de comandos.
- **ADC (Conversor Analógico-Digital)** para leitura do joystick.
- **Wi-Fi e protocolo HTTP** para envio de dados ao **ThingSpeak**.

## Como Executar
### 1. Configurar o Ambiente
Certifique-se de ter instalado:
- **SDK do Raspberry Pi Pico**
- **Compilador GCC para ARM**
- **CMake** para compilação
- **Bibliotecas da BitDogLab**

### 2. Compilar o Código
Execute os seguintes comandos:
```sh
mkdir build && cd build
cmake ..
make
```

### 3. Configurar Wi-Fi e API Key do ThingSpeak
No arquivo `wifi.c`, substitua as credenciais:
```c
#define WIFI_SSID "SUA_REDE_WIFI"
#define WIFI_PASSWORD "SUA_SENHA"
#define API_KEY "SUA_API_KEY_THINGSPEAK"
```

### 4. Carregar o Firmware
Conecte a **BitDogLab** ao PC e envie o firmware:
```sh
sudo picotool load -f build/main.uf2
```

### **5. Simulação pronta no Wokwi**
- Acesse [Wokwi](https://wokwi.com/projects/422016578752670721).

## Resultados
- O sistema permite **controle remoto de iluminação**, contabilizando os acionamentos e registrando no **ThingSpeak**.
- A interface **OLED e LEDs** garante uma experiência intuitiva.
- A comunicação Wi-Fi foi implementada com sucesso.
- O protótipo pode ser expandido para integrações mais complexas, como dashboards interativos.

## Conclusão

Este projeto demonstra uma abordagem simples e eficiente para automação residencial utilizando BitDogLab e Wi-Fi. A integração com ThingSpeak possibilita o monitoramento remoto, tornando o sistema mais flexível e útil.

## Referências
- Documentação da **BitDogLab**: [Link Oficial](https://bitdoglab.com/docs)
- API **ThingSpeak**: [Link Oficial](https://thingspeak.com/)
- SDK **Raspberry Pi Pico**: [Link Oficial](https://www.raspberrypi.org/documentation/microcontrollers/)

