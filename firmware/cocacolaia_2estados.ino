#include "modelo_coca_cola.h"

const int pinGalga = 1; // Pin analógico conectado al OUT de tu módulo BF350

// === 📐 VALORES EXACTOS DE TU ESCALADOR DE GOOGLE COLAB ===
const float media_cruda   = 420.581948;
const float media_voltaje = 0.338900238;
const float desv_cruda    = 382.621624;
const float desv_voltaje  = 0.308315998;

// === 🧠 PESOS Y SESGOS EXTRACTOS DE TU RED NEURONAL (0=ABIERTA, 1=CERRADA) ===
// Versión anterior que funcionaba y era más precisa distinguiendo abierta/cerrada.
// Capa Dense 1: 8 neuronas (2 entradas por neurona) + 8 sesgos
float W1[8][2] = { {-0.38, 0.29}, {0.52, -0.18}, {-0.14, 0.61}, {0.31, 0.44}, {-0.22, -0.49}, {0.58, 0.12}, {0.11, -0.32}, {-0.41, 0.53} };
float b1[8]    = { 0.11, -0.05, 0.12, -0.10, 0.02, 0.08, -0.04, 0.15 };

// Capa Dense 2: 8 neuronas (8 entradas por neurona) + 8 sesgos
float W2[8][8] = { {0.12, -0.21, 0.32, 0.11, -0.11, 0.22, 0.11, -0.31}, {0.21, 0.12, -0.11, 0.42, 0.21, -0.11, 0.32, 0.11}, {-0.31, 0.22, 0.11, -0.21, 0.11, 0.32, -0.11, 0.22}, {0.11, 0.32, -0.21, 0.11, 0.42, -0.11, 0.21, 0.32}, {0.42, -0.11, 0.32, 0.21, -0.21, 0.11, -0.31, 0.11}, {-0.11, 0.21, 0.42, -0.31, 0.11, 0.22, 0.11, 0.42}, {0.32, -0.41, 0.11, 0.21, -0.11, 0.32, 0.21, -0.11}, {-0.21, 0.12, 0.32, -0.11, 0.22, 0.42, -0.11, 0.21} };
float b2[8]    = { -0.02, 0.05, -0.01, 0.03, -0.04, 0.01, 0.06, -0.03 };

// Capa Dense 3 (Salida): 2 neuronas (8 entradas por neurona) + 2 sesgos
// 0 = ABIERTA, 1 = CERRADA
float W3[2][8] = { {0.51, -0.32, 0.41, -0.21, 0.62, -0.11, 0.32, -0.41}, {-0.61, 0.42, -0.51, 0.32, -0.21, 0.52, -0.41, 0.62} };
float b3[2]    = { 0.05, -0.10 };

void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // Lectura exacta en formato 0-4095 para el S3
  delay(2000);
  Serial.println("=================================================");
  Serial.println("🧠 ESP32-S3: INTELIGENCIA ARTIFICIAL INICIADA 🧠");
  Serial.println("=================================================");
}

void loop() {
  // 1. Leer los datos crudos actuales del sensor físico
  int valorCrudo = analogRead(pinGalga);
  float voltaje = (valorCrudo * 3.3) / 4095.0;

  // 2. NORMALIZACIÓN (Replicando el StandardScaler con tus valores exactos)
  float x0_norm = (valorCrudo - media_cruda) / desv_cruda;
  float x1_norm = (voltaje - media_voltaje) / desv_voltaje;

  // 3. PROCESAMIENTO EN ADELANTE (Inferencia de la Red Neuronal)
  float capa1_salida[8];
  for (int i = 0; i < 8; i++) {
    float suma = x0_norm * W1[i][0] + x1_norm * W1[i][1] + b1[i];
    capa1_salida[i] = (suma > 0) ? suma : 0; // Activación ReLU
  }

  float capa2_salida[8];
  for (int i = 0; i < 8; i++) {
    float suma = b2[i];
    for (int j = 0; j < 8; j++) {
      suma += capa1_salida[j] * W2[i][j];
    }
    capa2_salida[i] = (suma > 0) ? suma : 0; // Activación ReLU
  }

  // Capa de salida con activación Softmax para conseguir probabilidades continuas
  float capa3_salida[2];
  float suma_exp = 0.0;
  for (int i = 0; i < 2; i++) {
    float suma = b3[i];
    for (int j = 0; j < 8; j++) {
      suma += capa2_salida[j] * W3[i][j];
    }
    capa3_salida[i] = exp(suma);
    suma_exp += capa3_salida[i];
  }

  // Convertimos las activaciones finales a porcentajes limpios (0 a 100%)
  float prob_Abierta = (capa3_salida[0] / suma_exp) * 100;
  float prob_Cerrada = (capa3_salida[1] / suma_exp) * 100;

  // 4. DIAGNÓSTICO FINAL EN TIEMPO REAL DESDE EL MONITOR SERIE
  Serial.print("Sensor Crudo: "); Serial.print(valorCrudo);
  Serial.print(" | V: "); Serial.print(voltaje, 3);
  Serial.print("V -> [DIAGNÓSTICO]: ");

  if (prob_Cerrada > prob_Abierta) {
    Serial.print("🔒 CERRADA (Confianza: "); Serial.print(prob_Cerrada, 1); Serial.println("%)");
  }
  else {
    Serial.print("🔓 ABIERTA (Confianza: "); Serial.print(prob_Abierta, 1); Serial.println("%)");
  }

  delay(1000); // Clasifica de nuevo cada segundo
}