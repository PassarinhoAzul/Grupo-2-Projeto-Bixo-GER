#include <Arduino.h>

// Variáveis de Tempo e Velocidade
unsigned long tempoAtual;
unsigned long deltaTempo;
unsigned long tempoAnterior = 0;
double velocidadeAtual = 0;
double velocidadeDesejada = 100.0; // Velocidade alvo em pulsos por segundo

// Constantes PID (Esses valores devem ser ajustados/tunados para cada caso)
double kp = 2.0;
double ki = 0.5;
double kd = 1.0;

// Variáveis PID
double erroIntegral = 0;
double erroAnterior = 0;

void logicaPID();

void setup()
{
  Serial.begin(9600);
}

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

void logicaPID()
{
  // 1. Obter a velocidade atual do motor, isso pode vir de uma lógia externa à esta função
  // neste caso, está sendo feito no loop, atualizada na variável 'velocidadeAtual'


  // 2. Algoritmo PID

  // Proporcional
  double erro = velocidadeDesejada - velocidadeAtual;

  // Integral
  erroIntegral += erro * (deltaTempo / 1000.0);

  // Derivativo
  double erroDerivativo = (erro - erroAnterior) / (deltaTempo / 1000.0);

  // Fórmula final do OUTPUT (Esforço de Controle)
  // Este valor não tem unidade e deve ser convertido para PWM (0-255) para controlar o motor
  // As próprias constantes PID (kp, ki, kd) se encarregam de ajustar a escala do sinal de saída
  double sinalSaida = (kp * erro) + (ki * erroIntegral) + (kd * erroDerivativo);


  // 3. Converter o OUTPUT (Esforço de Controle) para PWM (0-255 com direção)

  // Lidar com a direção
  if (sinalSaida > 0)
  {
    // Frente
    motorParaFrente();

    // Para ponte H l298n deve ser aglo como:
    // digitalWrite(IN1, HIGH);
    // digitalWrite(IN2, LOW);
  }
  else
  {
    // Ré / Freio
    motorParaTras();

    // Para ponte H l298n deve ser aglo como:
    // digitalWrite(IN1, LOW);
    // digitalWrite(IN2, HIGH);
  }

  // Valor absoluto (módulo)
  int pwm = abs(sinalSaida);

  // IMPORTANTE: limitar a saída para o intervalo possível
  if (pwm > 255)
  {
    pwm = 255;
  }


  // 4. Enviar saída para o motor (PWM)
  analogWrite(ENA, pwm);

  
  // 5. Atualizar variáveis para o próximo ciclo
  tempoAnterior = tempoAtual;
  // Alguma lógica associada ao cálculo da velocidade pode vir aqui

  // Monitor Serial
  Serial.print("Desejada:");
  Serial.print(velocidadeDesejada);
  Serial.print("Atual:");
  Serial.println(velocidadeAtual);
}