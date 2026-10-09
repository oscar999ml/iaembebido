// Definimos el pin analógico. Usaremos el GPIO 1 (A1 en algunas placas)
const int pinGalga = 1; 

void setup() {
  // Inicializamos el monitor serie a 115200 baudios
  Serial.begin(115200);
  Serial.println("--- Sistema de Monitoreo de Galga BF350 Iniciado ---");
  
  // Opcional: Configuramos la resolución a 12 bits (0-4095)
  analogReadResolution(12); 
}

void loop() {
  // 1. Leer el valor analógico crudo (0 a 4095)
  int valorCrudo = analogRead(pinGalga);
  
  // 2. Convertir el valor crudo a Voltaje estimado (0.0V a 3.3V)
  // Multiplicamos por 3.3V de referencia y dividimos entre la resolución máxima (4095)
  float voltaje = (valorCrudo * 3.3) / 4095.0;
  
  // 3. Imprimir los datos en el monitor serie en formato limpio
  Serial.print("Lectura_Cruda:");
  Serial.print(valorCrudo);
  Serial.print(",");
  Serial.print("Voltaje_V:");
  Serial.println(voltaje, 3); // Muestra 3 decimales para ver variaciones micro

  // Esperamos 200 milisegundos entre lecturas (5 lecturas por segundo)
  delay(200);
}