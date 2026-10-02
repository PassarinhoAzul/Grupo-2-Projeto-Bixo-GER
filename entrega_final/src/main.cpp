#include <Arduino.h>

#define ENC_IN_ESQ_A 1//0
#define ENC_IN_ESQ_B 0//1
#define ENC_IN_DIR_A 3
#define ENC_IN_DIR_B 2

// Pinos da Ponte H 
#define MOT_ESQ_PWM 4
#define MOT_ESQ_IN1 6
#define MOT_ESQ_IN2 5
#define MOT_DIR_PWM 7
#define MOT_DIR_IN1 10
#define MOT_DIR_IN2 8

#define COMPRIMENTO_RODA 0.68 // cm

volatile long ticks_esq = 0;
volatile long ticks_dir = 0;

long ticks_ant_esq = ticks_esq;
long ticks_ant_dir = ticks_dir;

long delta_tempo = 0;

// Variaveis para garantir que o loop principal rode em frequencia fixa (Sem delay!)
long tempo_anterior = 0;
const int INTERVALO_AMOSTRAGEM_MS = 100; // Roda o controle a 20Hz

double kp = 2.0;
double ki = 0.5;
double kd = 1.0;

// Variáveis PID
double erroIntegral = 0;
double erroAnterior = 0;

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

      int direcao;
};

Motor motorEsquerdo = {MOT_ESQ_PWM, MOT_ESQ_IN1, MOT_ESQ_IN2, 0.0, 0.0, 0.0, 0.0, 0};
Motor motorDireito  = {MOT_DIR_PWM, MOT_DIR_IN1, MOT_DIR_IN2, 0.0, 0.0, 0.0, 0.0, 0};

// ISR do Encoder Esquerdo
void IRAM_ATTR isr_encoder_esq() {
  /*if (digitalRead(ENC_IN_ESQ_B) == HIGH) {
    ticks_esq++;
  } else {
    ticks_esq--;
  }*/
 ticks_esq++;
}

// ISR do Encoder Direito
void IRAM_ATTR isr_encoder_dir() {
  /*if (digitalRead(ENC_IN_DIR_B) == HIGH) {
    ticks_dir++;
  } else {
    ticks_dir--;
  }*/
 ticks_dir++;
}


void calcula_odometria() {
  // O resgate de variaveis volatile precisa ser rapido. 
  // Desligamos as interrupcoes por um microssegundo para copiar os valores e nao corromper os dados.
  noInterrupts();
  long ticks_atuais_esq = ticks_esq;
  long ticks_atuais_dir = ticks_dir;
  
  interrupts();
  
  // Quantidade de ticks no intervalo delta de tempo
  int delta_ticks_esq = ticks_atuais_esq - ticks_ant_esq;
  int delta_ticks_dir = ticks_atuais_dir - ticks_ant_dir;


  // Tick por seg
  // Calcular para cada roda
  double vel_esq_ticks = ((double)delta_ticks_esq / ((double)delta_tempo / 1000));
  double vel_dir_ticks = ((double)delta_ticks_dir / ((double)delta_tempo / 1000));


  // Velocidade angular
  // 1 tick /10
  double rot_dir = vel_dir_ticks / 10;
  double rot_esq = vel_esq_ticks / 10;

  motorEsquerdo.velocidadeAtual = rot_esq * COMPRIMENTO_RODA * motorEsquerdo.direcao;
  motorDireito.velocidadeAtual = rot_dir * COMPRIMENTO_RODA * motorDireito.direcao;

  Serial.print("Delta ticks (esq) (dir) = ");
  Serial.print(delta_ticks_esq);
  Serial.print("    ");
  Serial.println(delta_ticks_dir);

  Serial.print("Velocidade Linear (esq) (dir) = ");
  Serial.print(motorEsquerdo.velocidadeAtual);
  Serial.print("    ");
  Serial.println(motorDireito.velocidadeAtual);

  ticks_ant_esq = ticks_atuais_esq;
  ticks_ant_dir = ticks_atuais_dir;

}

void calcularPIDMotor(Motor *motor)
{
      if (delta_tempo <= 0) return;

      // Cálculo do Erro
      double erro = motor->velocidadeDesejada - motor->velocidadeAtual;

      // Termo Integral com Anti-Windup
      motor->erroIntegral += erro * (delta_tempo / 1000.0);
      motor->erroIntegral = constrain(motor->erroIntegral, -100.0, 100.0);

      // Termo Derivativo
      double erroDerivativo = (erro - motor->erroAnterior) / (delta_tempo / 1000.0);

      // Sinal de Saída usando os Kps globais
      double sinalSaida = (kp * erro) + (ki * motor->erroIntegral) + (kd * erroDerivativo);

      // Tratamento de parada
      if (motor->velocidadeDesejada == 0)
      {
      sinalSaida = 0;
      motor->erroIntegral = 0;
      }

      // Direção dos pinos da Ponte H
      if (sinalSaida > 0)
      {
        motor->direcao = 1;
      digitalWrite(motor->pinIN1, HIGH);
      digitalWrite(motor->pinIN2, LOW);
      }
      else if (sinalSaida < 0)
      {
        motor->direcao = -1;
      digitalWrite(motor->pinIN1, LOW);
      digitalWrite(motor->pinIN2, HIGH);
      }
      else
      {
      digitalWrite(motor->pinIN1, LOW);
      digitalWrite(motor->pinIN2, LOW);
      }

      // Cálculo do PWM com Offset para atrito estático
      int pwm = abs((int)sinalSaida);
      if (motor->velocidadeDesejada != 0 && pwm > 0)
      {
      pwm = pwm + 30; // Mínimo PWM para dar a partida
      }
      pwm = constrain(pwm, 0, 255);

      // Enviar PWM para a Ponte H
      analogWrite(motor->pinPWM, pwm);

      // Atualizar erro anterior do motor
      motor->erroAnterior = erro;
}

void setup() {
  Serial.begin(9600);
  
  // Encoders
  pinMode(ENC_IN_ESQ_A, INPUT_PULLUP);
  pinMode(ENC_IN_ESQ_B, INPUT_PULLUP);
  pinMode(ENC_IN_DIR_A, INPUT_PULLUP);
  pinMode(ENC_IN_DIR_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_IN_ESQ_A), isr_encoder_esq, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_IN_DIR_A), isr_encoder_dir, RISING);

  // Motores
  pinMode(motorEsquerdo.pinPWM, OUTPUT);
  pinMode(motorEsquerdo.pinIN1, OUTPUT);
  pinMode(motorEsquerdo.pinIN2, OUTPUT);
  pinMode(motorDireito.pinPWM, OUTPUT);
  pinMode(motorDireito.pinIN1, OUTPUT);
  pinMode(motorDireito.pinIN2, OUTPUT);

  motorDireito.velocidadeDesejada = 0;
  motorEsquerdo.velocidadeDesejada = 0;

  
  Serial.println("Sistema Iniciado. Aguardando inicio dos ciclos de controle...");
}

void loop() {
  unsigned long tempo_atual = millis();
  delta_tempo = tempo_atual - tempo_anterior;

  if (delta_tempo >= INTERVALO_AMOSTRAGEM_MS) {
    calcula_odometria();
    calcularPIDMotor(&motorDireito);
    calcularPIDMotor(&motorEsquerdo);
    tempo_anterior = tempo_atual;
  }

  if (tempo_atual > 2000 && tempo_atual < 7000)
  {
    motorDireito.velocidadeDesejada = 40;
    motorEsquerdo.velocidadeDesejada = 40;
  }

  if (tempo_atual > 7000)
  {
    motorDireito.velocidadeDesejada = 0;
    motorEsquerdo.velocidadeDesejada = 0;
  }
}
