# Projeto_IoT_Energia

Problema:
Segundo a Resolução RE 01/2005 publicada pela Anvisa algumas condições de temperatura e umidade devem ser preservadas quando se trata do armazenamento de medicações em hospitais e postos de saúde. Para mantermos a integridade destes insumos é necessário que a temperatura esteja na faixa de 15°C a 30°C e a umidade abaixo de 75%. Sabendo disso é de extrema importância que os locais de armazenamento sejam monitorados quanto a essas características.

Solução:
Um sistema de monitoramento de temperatura e umidade baseado na ESP32. Ele usa um sensor DHT22 para coletar dados ambientais e dois LEDs para indicar as condições: um LED para temperatura dentro de um intervalo desejado e outro LED para umidade abaixo de 75%. Os dados são enviados para um servidor MQTT. O dispositivo se conecta à rede Wi-Fi, publica os dados e se reconecta automaticamente em caso de desconexão.