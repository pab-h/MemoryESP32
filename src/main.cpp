#include <Arduino.h>
#include <EEPROM.h>

#define LDR_PIN             34

#define EEPROM_SIZE         0xFF
#define ADDRESS_CONFIG_FLAG 0x00
#define ADDRESS_MIN_VALUE   0x01
#define ADDRESS_MAX_VALUE   0x05

class LDRConfig {

  public:

    int maxValue;
    int minValue;

    LDRConfig();
    
  static void fromMemory(LDRConfig* config);
  static void toMemory(LDRConfig* config);

};

LDRConfig::LDRConfig() {

  this->maxValue = 0xFFF;
  this->minValue = 0x00;

}

void LDRConfig::fromMemory(LDRConfig* config) {

  uint8_t data       = EEPROM.read(ADDRESS_CONFIG_FLAG);
  bool    configFlag = static_cast<bool>(data);

  if (!configFlag) {

    Serial.println("[AVISO] Nenhuma Configuração encontrada");
    Serial.println("[AVISO] Valores padroes inseridos:");
    Serial.println("[AVISO] max = 0xFFF e min = 0x00");

    config->maxValue = 0xFFF;
    config->minValue = 0x00;

    return;
  }

  config->maxValue = EEPROM.readInt(ADDRESS_MAX_VALUE);
  config->minValue = EEPROM.readInt(ADDRESS_MIN_VALUE);

};

void LDRConfig::toMemory(LDRConfig* config) {

  EEPROM.write(ADDRESS_CONFIG_FLAG, 0x01);
  EEPROM.writeInt(ADDRESS_MIN_VALUE, config->minValue);
  EEPROM.writeInt(ADDRESS_MAX_VALUE, config->maxValue);
  EEPROM.commit();
  
}

uint16_t readLDR(uint8_t pin) {
  return analogRead(pin);
}

int readLDRPercentage(uint8_t pin, LDRConfig* config) {

  uint16_t ldrValue = readLDR(pin);
  long ldrMapValue  = map(
    static_cast<long>(ldrValue), 
    static_cast<long>(config->minValue), 
    static_cast<long>(config->maxValue),
    0, 
    100
  );  

  return static_cast<int>(ldrMapValue);
}

void configurationMode(LDRConfig* config) {

  Serial.println("[MODO CONFIGURAÇÃO]");
  Serial.println("Digite o valor MÍNIMO (0 a 4095): ");
  
  while (Serial.available() == 0) delay(10);

  int minValue = Serial.readStringUntil('\n').toInt();

  Serial.println("Valor mínimo recebido: " + String(minValue));
  Serial.println("Digite o valor MÁXIMO (0 a 4095): ");

  while (Serial.available() == 0) delay(10);

  int maxValue = Serial.readStringUntil('\n').toInt();

  Serial.println("Valor máximo recebido: " + String(maxValue));

  if (minValue >= 0 && maxValue <= 4095 && minValue < maxValue) {
    
    config->minValue = minValue;
    config->maxValue = maxValue;

    LDRConfig::toMemory(config);
    
    Serial.println("[CONFIG] Configuração salva com sucesso!");
  
  } else {
  
    Serial.println("[ERRO] Valores inválidos. Tente novamente.");
  
  }

  Serial.println("[FIM DO MODO CONFIGURAÇÃO]");

}

void resetConfig(LDRConfig* config) {

  Serial.println("[RESET] Apagando configuração da EEPROM...");

  EEPROM.write(ADDRESS_CONFIG_FLAG, 0x00);
  EEPROM.commit();

  config->minValue = 0;
  config->maxValue = 0xFF;

  Serial.println("[RESET] Configuração apagada com sucesso!");

}

void showConfig(LDRConfig* config) {

    Serial.println("[MOSTRAR] Lendo configuração da EEPROM...");

    LDRConfig::fromMemory(config);

    Serial.println("[MOSTRAR] minValue = " + String(config->minValue));
    Serial.println("[MOSTRAR] maxValue = " + String(config->maxValue));

}

LDRConfig config;
String    command;

void setup() {

  Serial.begin(9600);
  EEPROM.begin(EEPROM_SIZE);
  
  LDRConfig::fromMemory(&config);

}

void loop() {

  uint16_t ldrValue      = readLDR(LDR_PIN);
  int ldrPercentageValue = readLDRPercentage(LDR_PIN, &config);

  Serial.print("LDR read = ");
  Serial.print(ldrValue);
  Serial.print(", ");
  Serial.print(ldrPercentageValue);
  Serial.println("%");

  if (Serial.available() > 0) {

    command = Serial.readStringUntil('\n');
    command.trim();

  }

  if (command.equalsIgnoreCase("CONFIG")) {
    configurationMode(&config);
  }

  if (command.equalsIgnoreCase("RESET")) {
    resetConfig(&config);
  }

  if (command.equalsIgnoreCase("SHOW")) {
    showConfig(&config);
  }

  command = ""; 

  delay(1000);

}

