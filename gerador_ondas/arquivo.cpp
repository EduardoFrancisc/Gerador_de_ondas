#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

// Definições
#define saidaPWM 3
#define pinoPOT A0
#define pinoBOTAO 2

LiquidCrystal_I2C lcd(32, 16, 2);

enum TipoOnda {
  OFF, SENOIDAL, TRIANGULAR, DENTE_SERRA, QUADRADA, RUIDO
};

TipoOnda tipoOnda = OFF;
bool estadoBotaoAnterior = HIGH;
unsigned long tempoUltimoClique = 0;
const int debounceDelay = 200;

unsigned long tempoUltimaAtualizacao = 0;
int passo = 0;
int freq = 10;
bool subindo = true;

const int PWM_RESOLUTION = 256;
const int MAX_FREQ = 50;

void setup() {
  pinMode(saidaPWM, OUTPUT);
  pinMode(pinoBOTAO, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  Serial.begin(9600);

  analogWrite(saidaPWM, 0);

  lcd.setCursor(0, 0);
  lcd.print("Gerador de Ondas");
  delay(1000);
  lcd.clear();
}

void loop() {
  int leituraPot = analogRead(pinoPOT);  
  freq = map(leituraPot, 0, 1023, 1, MAX_FREQ);

  bool estadoAtual = digitalRead(pinoBOTAO);
  if (estadoAtual == LOW && estadoBotaoAnterior == HIGH &&
      (millis() - tempoUltimoClique > debounceDelay)) {
    tipoOnda = static_cast<TipoOnda>((tipoOnda + 1) % 6);
    tempoUltimoClique = millis();
    passo = 0;
    subindo = true;
    if (tipoOnda == OFF) {
      analogWrite(saidaPWM, 0);
    }
    atualizaLCD(true);
  }
  estadoBotaoAnterior = estadoAtual;

  unsigned long intervalo = 1000000UL / (freq * PWM_RESOLUTION);

  if (micros() - tempoUltimaAtualizacao >= intervalo && tipoOnda != OFF) {
    tempoUltimaAtualizacao = micros();

    switch (tipoOnda) {
      case SENOIDAL: {
        float rad = passo * (2.0 * PI / PWM_RESOLUTION);
        int valorPWM = int(127.5 + 127.5 * sin(rad));
        analogWrite(saidaPWM, valorPWM);
        passo = (passo + 1) % PWM_RESOLUTION;
        break;
      }

      case TRIANGULAR:
        analogWrite(saidaPWM, passo);
        if (subindo) {
          passo++;
          if (passo >= 255) subindo = false;
        } else {
          passo--;
          if (passo <= 0) subindo = true;
        }
        break;

      case DENTE_SERRA:
        analogWrite(saidaPWM, passo);
        passo = (passo + 1) % PWM_RESOLUTION;
        break;

      case QUADRADA:
        analogWrite(saidaPWM, (passo < PWM_RESOLUTION/2) ? 255 : 0);
        passo = (passo + 1) % PWM_RESOLUTION;
        break;

      case RUIDO:
        analogWrite(saidaPWM, random(0, 256)); 

      default:
        analogWrite(saidaPWM, 0);
        break;
    }
  }

  static unsigned long tempoLCD = 0;
  if (millis() - tempoLCD > 200) {
    tempoLCD = millis();
    atualizaLCD(false);
  }
}

void atualizaLCD(bool forcarAtualizacao) {
  static int freqAnterior = -1;
  static TipoOnda ondaAnterior = OFF;

  if (forcarAtualizacao || freq != freqAnterior || tipoOnda != ondaAnterior) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Freq: ");
    lcd.print(freq);
    lcd.print(" Hz");

    lcd.setCursor(0, 1);
    lcd.print("Onda: ");
    switch(tipoOnda) {
      case OFF: lcd.print("OFF"); break;
      case SENOIDAL: lcd.print("SENOIDAL"); break;
      case TRIANGULAR: lcd.print("TRIANGULAR"); break;
      case DENTE_SERRA: lcd.print("DENTE_SERRA"); break;
      case QUADRADA: lcd.print("QUADRADA"); break;
      case RUIDO: lcd.print("RUIDO"); break;
    }

    freqAnterior = freq;
    ondaAnterior = tipoOnda;
  }
}