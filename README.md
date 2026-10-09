# 🥤 IA Embebido — Detección del estado de una lata de Coca-Cola con ESP32-S3

Sistema embebido de **TinyML** que detecta en tiempo real si una lata de Coca-Cola está **abierta**, **cerrada** o **vacía**, usando un módulo de galga extensiométrica (BF350) conectado a un **ESP32-S3**.

La red neuronal se entrena en **Google Colab** con TensorFlow/Keras, se convierte a **TensorFlow Lite**, y luego la inferencia se ejecuta **directamente en el microcontrolador** con forward-pass manual escrito en C++ (sin librerías externas).

```
┌─────────────┐   ┌──────────────┐   ┌──────────────┐   ┌──────────────────────┐
│  Sensor      │   │  Dataset     │   │  Colab       │   │  ESP32-S3 (Arduino)  │
│  BF350       │──▶│  (Excel)     │──▶│  TensorFlow  │──▶│  Inferencia en C++   │
│  (galga)     │   │  3 estados   │   │  → TFLite    │   │  Diagnóstico serie   │
└─────────────┘   └──────────────┘   └──────────────┘   └──────────────────────┘
```

## ✨ Características

- 🤖 Red neuronal `2 → 8 → 8 → 3` (ReLU + Softmax) entrenada con datos reales del sensor.
- 🔍 Inferencia **en tiempo real dentro del ESP32-S3**, segundo a segundo, sin conexión a internet.
- 📊 Salida por **Serial (115200 baud)** con probabilidades para cada estado y porcentaje de confianza.
- 📈 Entrenamiento, conversión a TinyML y exportación del modelo como cabecera C++ desde Colab.
- 🗂️ Datos reales capturados con una celda de carga BF350 en los 3 estados de la lata.

## 🧰 Hardware

| Componente | Detalle |
|---|---|
| Placa | **ESP32-S3** (ADC de 12 bits, 0–4095) |
| Sensor | Módulo de **galga extensiométrica BF350** (salida analógica OUT) |
| Objeto | Lata de Coca-Cola (llena abierta, llena cerrada, vacía) |

### Conexión

| Módulo BF350 | ESP32-S3 |
|---|---|
| `OUT` | Pin **GPIO 1** (`pinGalga`) |
| `VCC` | 3.3 V |
| `GND` | GND |

## 📁 Estructura del repositorio

```
iaembebido/
├── assets/img/                 # Imágenes del proyecto (setup, sensor, latas)
├── data/                       # Datos crudos capturados (Excel, 1 archivo por estado)
│   ├── datos_galga_coca_cola_abierta.xlsx
│   ├── datos_galga_coca_cola_cerrada.xlsx
│   └── datos_galga_coca_cola_vacia.xlsx
├── firmware/
│   └── cocacolaia.ino           # Sketch de Arduino para el ESP32-S3
├── models/
│   ├── c_header/modelo_coca_cola.h     # Modelo TFLite como array de bytes C/C++
│   └── tflite/modelo_coca_cola.tflite  # Modelo TinyML ya optimizado
└── notebooks/
    └── Cocacolaopenclose.ipynb  # Pipeline completo en Google Colab
```

## 📊 Dataset

Tres archivos Excel con lecturas de la galga BF350 registradas por el ESP32-S3:

| Estado | Descripción | Columna 1 | Columna 2 |
|---|---|---|---|
| `abierta` | Lata abierta (llena) | `LecturaCruda` | `Voltaje (V)` |
| `cerrada` | Lata cerrada (llena) | `LecturaCruda` | `Voltaje (V)` |
| `vacia` | Lata vacía | `LecturaCruda` | `Voltaje (V)` |

Las características (features) son el valor **crudo del ADC** y el **voltaje** calculado como `(crudo × 3.3) / 4095`.

## 🧠 Modelo y entrenamiento

El proceso completo está documentado paso a paso en `notebooks/Cocacolaopenclose.ipynb`:

1. Carga y estandarización de los 3 Excel (columna `Estado` + renombrado de columnas).
2. Unificación en `datos_latas_3_estados.csv` (527 filas).
3. Codificación de clases con `LabelEncoder`:
   - `0` = abierta · `1` = cerrada · `2` = vacía
4. División **80/20 estratificada** (`random_state=42`) y normalización con `StandardScaler`.
5. Arquitectura Keras:

| Capa | Neuronas | Activación |
|---|---|---|
| Entrada | 2 (crudo, voltaje) | — |
| Densa 1 | 8 | ReLU |
| Densa 2 | 8 | ReLU |
| Salida | 3 | Softmax |

6. Entrenamiento: `adam`, `sparse_categorical_crossentropy`, **100 épocas**, `batch_size=8`, validación con el 20%.
7. Conversión a **TensorFlow Lite** (`modelo_coca_cola.tflite`, ~2.6 KB).
8. Exportación del modelo como **array de bytes C++** → `modelo_coca_cola.h`.
9. Extracción e inyección de pesos/sesgos y constantes de calibración al sketch.

## 🚀 Cómo usarlo

### 1. Entrenar en Google Colab

1. Abre `notebooks/Cocacolaopenclose.ipynb` en [Google Colab](https://colab.research.google.com).
2. Sube los 3 archivos de `data/` a `/content/sample_data/`.
3. Ejecuta todas las celdas en orden.
4. Al final obtendrás `modelo_coca_cola.tflite`, `modelo_coca_cola.h` y los pesos listos para el sketch.

### 2. Programar el ESP32-S3 (Arduino IDE)

> ⚠️ **Importante:** `cocacolaia.ino` y `modelo_coca_cola.h` deben estar **en la misma carpeta**, porque el sketch incluye la cabecera con `#include "modelo_coca_cola.h"`.

1. Instala el soporte de placas **ESP32** desde el *Gestor de placas* del Arduino IDE.
2. Selecciona tu placa **ESP32-S3** (p. ej. *ESP32S3 Dev Module*) y el puerto USB correspondiente.
3. Copia a la carpeta del sketch los archivos:
   - `firmware/cocacolaia.ino`
   - `models/c_header/modelo_coca_cola.h`
4. Conecta el módulo BF350 según [la tabla de conexión](#-hardware).
5. Carga el sketch y abre el **Monitor Serie a 115200 baud**.

### 3. Salida esperada en el monitor serie

```
=================================================
🧠 ESP32-S3: MONITOREO CONTINUO CON IA ACTIVA 🧠
=================================================
Sensor Crudo: 649 | V: 0.523V -> [DIAGNÓSTICO IA]: 🔒 CERRADA (Confianza: 95.3%)
Sensor Crudo: 621 | V: 0.500V -> [DIAGNÓSTICO IA]: 🔒 CERRADA (Confianza: 92.1%)
Sensor Crudo: 1111 | V: 0.895V -> [DIAGNÓSTICO IA]: 🔓 ABIERTA (Confianza: 88.4%)
```

Cada **1 segundo** el ESP32-S3 lee el sensor, normaliza, ejecuta las 3 capas (ReLU + Softmax) e imprime el diagnóstico con su nivel de confianza.

## 🔧 Constantes de calibración (StandardScaler)

Valores usados por el firmware para normalizar igual que en Python:

| Parámetro | Valor |
|---|---|
| `media_cruda` | 420.581948 |
| `media_voltaje` | 0.338900238 |
| `desv_cruda` | 382.621624 |
| `desv_voltaje` | 0.308315998 |

## 📌 Notas técnicas

- El ADC se configura con `analogReadResolution(12)` → rango 0–4095.
- La inferencia no requiere TensorFlow Lite en el micro: los pesos están embebidos como `const float` y el forward-pass se calcula manualmente (máxima eficiencia y despliegue offline).
- `modelo_coca_cola.h` contiene el modelo `.tflite` (2684 bytes) como array de bytes, listo para usarse también con un intérprete TFLite Micro si se desea.

## 📄 Licencia

Proyecto educativo de ejemplo — úsalo y modifícalo libremente.