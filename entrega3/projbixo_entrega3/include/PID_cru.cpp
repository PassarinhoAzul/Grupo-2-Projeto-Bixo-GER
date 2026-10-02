#include <Arduino.h>

#define MOT_ESQ_PWM 4
#define MOT_ESQ_IN1 5
#define MOT_ESQ_IN2 6

#define MOT_DIR_PWM 7
#define MOT_DIR_IN1 8
#define MOT_DIR_IN2 9

// Variáveis de Tempo e Velocidade
/*
unsigned long tempoAtual;
unsigned long deltaTempo;
unsigned long tempoAnterior = 0;
double velocidadeAtual_Esq = 0;
double velocidadeAtual_Dir = 0;
double velocidadeDesejada_Esq = 100.0; // Velocidade alvo em pulsos por segundo
double velocidadeDesejada_Dir = 100.0; // Velocidade alvo em pulsos por segundo
*/

// Constantes PID (Esses valores devem ser ajustados/tunados para cada caso)
double kp = 2.0;
double ki = 0.5;
double kd = 1.0;

// Variáveis PID
double erroIntegral = 0;
double erroAnterior = 0;


void setup()
{
  Serial.begin(9600);
}

struct Motor {
      // Pinos de controle (Ponte H)
      int pinPWM;
      int pinIN1;
      int pinIN2;

      // Variáveis de Velocidade
      double velocidadeDesejada;
      double velocidadeAtual;

      // Variáveis do PID
      double erroIntegral;
      double erroAnterior;
};

Motor motorEsquerdo = {MOT_ESQ_PWM, MOT_ESQ_IN1, MOT_ESQ_IN2, 0.0, 0.0, 0.0, 0.0};
Motor motorDireito  = {MOT_DIR_PWM, MOT_DIR_IN1, MOT_DIR_IN2, 0.0, 0.0, 0.0, 0.0};

void loop()
{
  unsigned long tempoAtual = millis();

  // Quanto tempo passou desde a última atualização
  unsigned long deltaTempo = tempoAtual - tempoAnterior;

  // Atualiza a cada 100ms
  if (deltaTempo >= 100)
  {
    // Lógica para obter a velocidade atual do motor
    velocidadeAtual = logica_obter_velocidade(); // Função fictícia que deve ser implementada para obter a velocidade atual do motor

    // Lógica PID, essa função vai calcular/enviar um novo sinal para
    // o motor a partir de 'velocidadeAtual' e 'velocidadeDesejada'
    logicaPID();
  }
}

void calcularPIDMotor(Motor &motor, double deltaTempo)
{
      if (deltaTempo <= 0) return;

      // Cálculo do Erro
      double erro = motor.velocidadeDesejada - motor.velocidadeAtual;

      // Termo Integral com Anti-Windup
      motor.erroIntegral += erro * (deltaTempo / 1000.0);
      motor.erroIntegral = constrain(motor.erroIntegral, -100.0, 100.0);

      // Termo Derivativo
      double erroDerivativo = (erro - motor.erroAnterior) / (deltaTempo / 1000.0);

      // Sinal de Saída usando os Kps globais
      double sinalSaida = (kp * erro) + (ki * motor.erroIntegral) + (kd * erroDerivativo);

      // Tratamento de parada
      if (motor.velocidadeDesejada == 0)
      {
      sinalSaida = 0;
      motor.erroIntegral = 0;
      }

      // Direção dos pinos da Ponte H
      if (sinalSaida > 0)
      {
      digitalWrite(motor.pinIN1, HIGH);
      digitalWrite(motor.pinIN2, LOW);
      }
      else if (sinalSaida < 0)
      {
      digitalWrite(motor.pinIN1, LOW);
      digitalWrite(motor.pinIN2, HIGH);
      }
      else
      {
      digitalWrite(motor.pinIN1, LOW);
      digitalWrite(motor.pinIN2, LOW);
      }

      // Cálculo do PWM com Offset para atrito estático
      int pwm = abs((int)sinalSaida);
      if (motor.velocidadeDesejada != 0 && pwm > 0)
      {
      pwm = pwm + 30; // Mínimo PWM para dar a partida
      }
      pwm = constrain(pwm, 0, 255);

      // Enviar PWM para a Ponte H
      analogWrite(motor.pinPWM, pwm);

      // Atualizar erro anterior do motor
      motor.erroAnterior = erro;
}