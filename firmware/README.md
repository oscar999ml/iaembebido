# 📁 Firmware para ESP32-S3

Esta carpeta contiene los sketches de Arduino (.ino) desarrollados durante el proyecto **"IA Embebido"** para la detección del estado de una lata de Coca-Cola con el módulo de galga BF350.

---

## 🏁 Versión ORIGINAL y FINAL — `cocacolaia.ino`

> **Este es el sketch final y original del proyecto.** Es el que debes usar.

- IA activa con los **pesos y sesgos reales extraídos del modelo entrenado en Google Colab** (`W1`, `W2`, `W3`).
- Clasifica en tiempo real los **3 estados**: 🔒 CERRADA, 🔓 ABIERTA y 🗑️ VACÍA.
- Normaliza con los valores exactos del `StandardScaler` de Colab.
- Requiere `modelo_coca_cola.h` en la misma carpeta (el sketch lo incluye).
- Imprime por el monitor serie el diagnóstico y el porcentaje de confianza **una vez por segundo**.

**Uso:** cargar junto con `../models/c_header/modelo_coca_cola.h` y abrir el monitor serie a **115200 baud**.

---

## 🛠️ Versiones PASADAS (guardadas aparte)

Estas versiones anteriores funcionaban, pero quedaron como referencia histórica. **No se usan en la versión final.**

### 1. `cocacolaia_2estados.ino` — Clasificación de 2 estados

Versión anterior que clasificaba solo **2 estados** (🔒 CERRADA vs 🔓 ABIERTA) usando pesos redondeados.

- Más simple (1 sola comparación en el diagnóstico).
- Compartía la misma normalización del `StandardScaler`.
- Se mantiene guardada porque era **más precisa distinguiendo cerrada/abierta**.

**¿Para qué era?** Probar la inferencia de la red neuronal en el ESP32-S3 con una versión reducida.

### 2. `lectura_cruda.ino` — Lectura en crudo del sensor

Versión inicial, **sin IA**. Solo leía el valor analógico del BF350 y lo imprimía en el monitor serie.

- Imprime `Lectura_Cruda` y `Voltaje_V` a 5 lecturas por segundo (`delay(200)`).
- Usaba `analogReadResolution(12)` (rango 0–4095).

**¿Para qué era?** Capturar los datos reales del sensor que luego se guardaron en los archivos Excel de `../data/` y sirvieron para entrenar el modelo en Colab.

---

## 📋 Resumen

| Archivo | Versión | Estados | ¿Para qué? |
|---|---|---|---|
| `cocacolaia.ino` | **Original / Final** | 3 (abierta, cerrada, vacía) | Inferencia de IA con el modelo real entrenado |
| `cocacolaia_2estados.ino` | Pasada | 2 (abierta, cerrada) | Prueba simplificada de inferencia con pesos redondeados |
| `lectura_cruda.ino` | Pasada | — (sin IA) | Recolección de datos crudos para el dataset |