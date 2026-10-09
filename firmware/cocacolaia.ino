#include "modelo_coca_cola.h"

const int pinGalga = 1; // Pin analógico conectado al OUT de tu módulo BF350

// === 📐 VALORES REALES DE CALIBRACIÓN DE GOOGLE COLAB ===
const float media_cruda   = 420.581948;
const float media_voltaje = 0.338900238;
const float desv_cruda    = 382.621624;
const float desv_voltaje  = 0.308315998;

// === 🧠 PESOS Y SESGOS REALES INYECTADOS DESDE TU MODELO DE COLAB ===
const float W1[8][2] = {
  {-0.381300f, -0.717259f},
  {-0.629985f, 0.623742f},
  {1.012472f, 0.734520f},
  {-0.577860f, 0.013082f},
  {-1.265206f, -1.154849f},
  {-0.824057f, -0.073154f},
  {-0.153291f, -0.756301f},
  {1.040298f, 0.082727f},
};
const float b1[8] = {-0.116147f, -0.022258f, 0.069811f, 0.534275f, 0.096951f, 0.612038f, 0.619545f, 0.314966f};

const float W2[8][8] = {
  {0.344719f, -0.407096f, -0.218633f, -0.580292f, -0.119407f, 0.381936f, 0.033900f, 0.137802f},
  {0.119114f, 0.362318f, 0.357452f, 0.160321f, 0.174745f, -0.621161f, -1.061640f, 0.674716f},
  {0.881078f, 0.532303f, -0.167139f, 1.287490f, 0.680525f, 0.506255f, 0.383140f, -0.071023f},
  {-0.778304f, 0.581876f, -0.440441f, 0.446140f, -0.408708f, 0.527479f, 0.327997f, 0.070883f},
  {0.230873f, 0.566475f, 0.086362f, 0.903005f, 0.569424f, 1.130472f, 0.652006f, -0.560565f},
  {-0.592785f, -0.282193f, 0.398729f, 0.092049f, -0.208856f, -0.083116f, 0.427733f, 0.982268f},
  {0.543510f, -0.411797f, -0.415063f, 0.501476f, -0.357753f, -0.251380f, 0.017385f, -0.215254f},
  {0.564636f, 0.372099f, -0.372000f, -0.229948f, 0.963842f, 0.133930f, 0.739690f, -0.817047f},
};
const float b2[8] = {-0.066560f, 0.160825f, 0.034709f, 0.415914f, 0.272640f, 0.586081f, -0.034031f, -0.030277f};

const float W3[3][8] = {
  {-0.297711f, -0.473616f, 0.388900f, 1.485707f, 0.553383f, 0.241302f, 0.315579f, -0.767850f}, // 0 = ABIERTA
  {-0.631100f, -0.842059f, 0.668344f, -1.846890f, 0.921781f, -1.537332f, -0.264444f, 0.117430f}, // 1 = CERRADA
  {0.217626f, 0.861680f, -0.765603f, -1.603135f, -1.583339f, 0.858804f, 0.201757f, -0.513349f}, // 2 = VACÍA
};
const float b3[3] = {-0.002250f, -0.150239f, 0.126913f};

void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // Establece el rango 0-4095 nativo del ESP32-S3
  delay(2000);
  Serial.println("=================================================");
  Serial.println("🧠 ESP32-S3: MONITOREO CONTINUO CON IA ACTIVA 🧠");
  Serial.println("=================================================");
}

void loop() {
  // 1. Leer los datos reales actuales del sensor físico
  int valorCrudo = analogRead(pinGalga);
  float voltaje = (valorCrudo * 3.3) / 4095.0;

  // 2. Normalización idéntica a Python
  float x0_norm = (valorCrudo - media_cruda) / desv_cruda;
  float x1_norm = (voltaje - media_voltaje) / desv_voltaje;

  // 3. Capa Oculta 1 (Activación ReLU)
  float capa1_salida[8];
  for (int i = 0; i < 8; i++) {
    float suma = x0_norm * W1[i][0] + x1_norm * W1[i][1] + b1[i];
    capa1_salida[i] = (suma > 0.0f) ? suma : 0.0f;
  }

  // 4. Capa Oculta 2 (Activación ReLU)
  float capa2_salida[8];
  for (int i = 0; i < 8; i++) {
    float suma = b2[i];
    for (int j = 0; j < 8; j++) {
      suma += capa1_salida[j] * W2[i][j];
    }
    capa2_salida[i] = (suma > 0.0f) ? suma : 0.0f;
  }

  // 5. Capa de Salida 3 (Activación Softmax Real)
  float capa3_salida[3];
  float suma_exp = 0.0f;
  for (int i = 0; i < 3; i++) {
    float suma = b3[i];
    for (int j = 0; j < 8; j++) {
      suma += capa2_salida[j] * W3[i][j];
    }
    capa3_salida[i] = exp(suma);
    suma_exp += capa3_salida[i];
  }

  // 6. Extracción de probabilidades mapeadas según tu LabelEncoder de Colab
  float prob_Abierta = (capa3_salida[0] / suma_exp) * 100.0f;
  float prob_Cerrada = (capa3_salida[1] / suma_exp) * 100.0f;
  float prob_Vacia   = (capa3_salida[2] / suma_exp) * 100.0f;

  // 7. DIAGNÓSTICO EN TIEMPO REAL DESDE EL SENSOR
  Serial.print("Sensor Crudo: "); Serial.print(valorCrudo);
  Serial.print(" | V: "); Serial.print(voltaje, 3);
  Serial.print("V -> [DIAGNÓSTICO IA]: ");

  if (prob_Cerrada > prob_Abierta && prob_Cerrada > prob_Vacia) {
    Serial.print("🔒 CERRADA (Confianza: "); Serial.print(prob_Cerrada, 1); Serial.println("%)");
  } 
  else if (prob_Abierta > prob_Cerrada && prob_Abierta > prob_Vacia) {
    Serial.print("🔓 ABIERTA (Confianza: "); Serial.print(prob_Abierta, 1); Serial.println("%)");
  } 
  else {
    Serial.print("🗑️ VACÍA (Confianza: "); Serial.print(prob_Vacia, 1); Serial.println("%)");
  }

  delay(1000); // Evalúa el estado real de la lata segundo a segundo
}
