#include "DHTesp.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

// Configuração do Wi-Fi
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6 // Definir o canal Wi-Fi acelera a conexão
int status = WL_IDLE_STATUS; // Status da rede Wi-Fi

// Configuração do MQTT
const char* mqtt_server = "test.mosquitto.org"; // Broker MQTT
char* my_topic_SUB = "FIT/SUB"; // Tópico escolhido para assinatura
char* my_topic_PUB1 = "FIT/PUB_Temperatura"; // Tópico escolhido para publicação
char* my_topic_PUB2 = "FIT/PUB_Umidade"; // Tópico escolhido para publicação

// Configuração do tempo de leitura
int contador = 1;
char mensagem[30];
unsigned long lastMillis = 0;

// Configuração do DHT22
const int DHT_PIN = 15;

// Define os pinos dos LEDs
const int LED_PIN = 2; // Pino do LED para temperatura
const int LED_UMIDADE_PIN = 4; // Pino do LED para umidade

DHTesp dhtSensor;
float ambiente[2];
float lastTemp = -999.0;
float lastHum = -999.0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Inicializa a comunicação serial
  Serial.begin(115200);
  Serial.println("Olá, ESP32!");

  // Define os pinos dos LEDs como saída
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_UMIDADE_PIN, OUTPUT);

  // Inicializa o sensor DHT22
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  // Conecta ao Wi-Fi
  conectaWifi();

  // Configura o servidor MQTT e a função de callback
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // Coleta os dados do sensor
  sensor_ambiente();

  // Verifica se a temperatura está dentro do intervalo desejado
  if (ambiente[0] > 15 && ambiente[0] < 30) {
    digitalWrite(LED_PIN, HIGH); // Acende o LED
  } else {
    digitalWrite(LED_PIN, LOW); // Apaga o LED
  }

  // Verifica se a umidade está abaixo de 75%
  if (ambiente[1] < 75) {
    digitalWrite(LED_UMIDADE_PIN, HIGH); // Acende o LED de umidade
  } else {
    digitalWrite(LED_UMIDADE_PIN, LOW); // Apaga o LED de umidade
  }

  // Verifica se houve uma alteração nos valores de temperatura ou umidade
  if (ambiente[0] != lastTemp || ambiente[1] != lastHum) {
    // Imprime os valores no monitor serial
    Serial.print("Temperatura: ");
    Serial.print(ambiente[0]);
    Serial.print(" C Umidade: ");
    Serial.print(ambiente[1]);
    Serial.println("");

    // Atualiza os valores anteriores
    lastTemp = ambiente[0];
    lastHum = ambiente[1];
  }

  // Verifica a conexão MQTT e processa as mensagens recebidas
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Aguarda 15 segundos para enviar uma nova mensagem
  if (millis() - lastMillis > 15000) {
    lastMillis = millis();

    // Envio da temperatura
    int val_int = (int)ambiente[0];
    float val_float = (abs(ambiente[0]) - abs(val_int)) * 100000;
    int val_fra = (int)val_float;
    sprintf(mensagem, "%d.%d", val_int, val_fra);

    // Imprime a mensagem no monitor serial
    Serial.print("Mensagem enviada: ");
    Serial.println(mensagem);

    // Envia a mensagem para o broker MQTT
    client.publish(my_topic_PUB1, mensagem);
    Serial.print(contador);
    Serial.println("-Temperatura enviada com sucesso...");

    // Envio da umidade
    val_int = (int)ambiente[1];
    val_float = (abs(ambiente[1]) - abs(val_int)) * 100000;
    val_fra = (int)val_float;
    sprintf(mensagem, "%d.%d", val_int, val_fra);

    // Imprime a mensagem no monitor serial
    Serial.print("Mensagem enviada: ");
    Serial.println(mensagem);

    // Envia a mensagem para o broker MQTT
    client.publish(my_topic_PUB2, mensagem);
    Serial.print(contador);
    Serial.println("-Umidade enviada com sucesso...");

    // Incrementa o contador
    contador++;
  }

  delay(100); // Adiciona um atraso para evitar sobrecarregar o broker MQTT com mensagens
}

void conectaWifi() {
  // Inicia a conexão Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Conectando ao Wi-Fi ");
  Serial.print(WIFI_SSID);

  // Aguarda pela conexão
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  // Imprime informações sobre a conexão Wi-Fi
  Serial.println(" Conectado!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Função de callback para processar mensagens recebidas
  String string = "";

  // Imprime informações sobre a mensagem recebida
  Serial.print("Chegou a mensagem [");
  Serial.print(topic);
  Serial.print("] ");

  // Converte a mensagem para uma string
  for (int i = 0; i < length; i++) {
    string += ((char)payload[i]);
  }

  // Imprime a mensagem no monitor serial
  Serial.println(string);
}

void reconnect() {
  // Tenta reconectar ao broker MQTT
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");

    // Tenta conectar ao broker MQTT com um ID de cliente único
    if (client.connect("ESPClient")) {
      Serial.println("Conectado");

      // Assina o tópico escolhido
      client.subscribe(my_topic_SUB);
    } else {
      // Imprime informações sobre a falha na conexão
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" Tente novamente em 5 segundos");

      // Aguarda 5 segundos antes de tentar novamente
      delay(5000);
    }
  }
}

void sensor_ambiente() {
  // Coleta os dados de temperatura e umidade do sensor DHT22
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  ambiente[0] = data.temperature;
  ambiente[1] = data.humidity;
}