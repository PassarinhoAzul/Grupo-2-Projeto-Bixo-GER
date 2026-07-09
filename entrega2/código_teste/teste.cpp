#include <Arduino.h>

// ===========================
// Definição dos pinos
// ===========================

// Motor esquerdo
const int IN1 = 5;
const int IN2 = 6;

// Motor direito
const int IN3 = 3;
const int IN4 = 4;

void setup()
{
    // Configura os pinos como saída
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    // Garante que os motores começam desligados
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);


  
}

void loop()
{
    // Frente
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    delay(3000);

    // Trás
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    delay(3000);
}
