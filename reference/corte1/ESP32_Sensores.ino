#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <math.h>
#include <string.h>

/*
  WMS - Water Monitoring System
  Prototipo local con ESP32. No interpreta el LDR como W/m2: muestra porcentaje
  de luz relativa, calibrado con dos lecturas ADC tomadas en el montaje real.
*/

// ============================================================================
// PINES: conservar este mapa si el montaje actual ya funciona.
// ============================================================================
const int PIN_TRIG = 18;
const int PIN_ECHO = 19;       // Debe recibir el ECHO mediante divisor 5 V -> 3.3 V.
const int PIN_DHT = 27;
const int PIN_FOTORESISTOR = 34;
const int PIN_RGB_ROJO = 25;
const int PIN_RGB_VERDE = 26;
const int PIN_RGB_AZUL = 32;
const int PIN_BUZZER = 33;
const int PIN_BOTON = 14;

#define TIPO_DHT DHT11

// ============================================================================
// CALIBRACION Y UMBRALES - EDITAR ESTOS VALORES DURANTE LA PRUEBA
// ============================================================================
// Nivel: el agua baja cuando la distancia sensor-superficie AUMENTA.
const float UMBRAL_DISTANCIA_CRITICA_CM = 15.0f;
const float HISTERESIS_NIVEL_CM = 0.5f;  // Evita conmutaciones por ruido cerca del limite.

// PONDERACION AMBIENTAL PARA CHIA, CUNDINAMARCA, EN AGOSTO.
// Serie NASA POWER 2000-2025 + sensibilidad FAO-56 Penman-Monteith.
// Los pesos suman 1.0. El LDR es un proxy de luz relativa, NO un piranometro.
const float PESO_LUZ_RELATIVA = 0.7474f;
const float PESO_TEMPERATURA = 0.1479f;
const float PESO_AIRE_SECO = 0.1026f;
const float PESO_PRESION_BAJA = 0.0021f;

// Rangos intercuartilicos locales de agosto usados para normalizar cada factor.
// T: 12.38-13.43 C | HR: 83.89-87.34 % | P: 758.8-759.8 hPa.
const float TEMPERATURA_Q25_CHIA_C = 12.38f;
const float TEMPERATURA_Q75_CHIA_C = 13.43f;
const float HUMEDAD_Q25_CHIA_PCT = 83.89f;
const float HUMEDAD_Q75_CHIA_PCT = 87.34f;
const float PRESION_Q25_CHIA_HPA = 758.8f;
const float PRESION_Q75_CHIA_HPA = 759.8f;

// La alerta amarillo/naranja aparece desde este indice ambiental relativo.
const float UMBRAL_INDICE_EVAPORACION_PCT = 70.0f;

// LDR: abre el monitor serie y anota ADC_LUZ_OSCURO y ADC_LUZ_CLARA.
// El modulo usado normalmente entrega ADC menor con mas luz. Se deja el 100%
// deliberadamente exigente: solo se alcanza con luz muy intensa.
const int ADC_LUZ_OSCURO = 3500;
const int ADC_LUZ_CLARA = 600;
const int MUESTRAS_LDR = 20;
const float FACTOR_SUAVIZADO_LUZ = 0.15f;

// ============================================================================
// TIEMPOS DE INTERFAZ
// ============================================================================
const unsigned long INTERVALO_SENSORES_MS = 250UL;
const unsigned long INTERVALO_DHT_MS = 2000UL;
const unsigned long INTERVALO_LCD_MS = 200UL;
const unsigned long INTERVALO_PARPADEO_MS = 350UL;
const unsigned long INTERVALO_BUZZER_MS = 140UL;
const unsigned long BLOQUEO_BOTON_MS = 250UL;
const int NUM_PANTALLAS = 4;

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_BMP085 bmp;
DHT dht(PIN_DHT, TIPO_DHT);

enum EstadoAlerta { ESTADO_NORMAL, ESTADO_AMARILLO, ESTADO_ROJO };

float temperatura = NAN;
float humedad = NAN;
float distanciaActual = -1.0f;
float presionActual = NAN;
float luzPct = 0.0f;
bool luzInicializada = false;
float indiceEvaporacion = NAN;
int adcLuz = 0;
bool dhtValido = false;
bool nivelCritico = false;
bool fusionAmbientalCompleta = false;
EstadoAlerta estadoActual = ESTADO_NORMAL;

int pantalla = 0;
int pantallaDibujada = -1;
int ultimaLecturaBoton = HIGH;
unsigned long ultimoCambioPantalla = 0;
bool faseLuz = false;
bool faseBuzzer = false;
unsigned long ultimoParpadeo = 0;
unsigned long ultimoBuzzer = 0;
unsigned long ultimaLecturaSensores = 0;
unsigned long ultimaLecturaDHT = 0;
unsigned long ultimaActualizacionLCD = 0;

// ============================================================================
// SENSORES
// ============================================================================
float medirDistancia() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duracion = pulseIn(PIN_ECHO, HIGH, 30000UL);
  if (duracion == 0) return -1.0f;
  return duracion * 0.0343f / 2.0f;
}

int leerADCPromedio() {
  long suma = 0;
  for (int i = 0; i < MUESTRAS_LDR; i++) {
    suma += analogRead(PIN_FOTORESISTOR);
    delay(2);
  }
  return suma / MUESTRAS_LDR;
}

float convertirLuzAPorcentaje(int adc) {
  // Esta formula tambien funciona si las lecturas reales quedan invertidas,
  // siempre que se escriban correctamente ADC_LUZ_OSCURO y ADC_LUZ_CLARA.
  if (ADC_LUZ_CLARA == ADC_LUZ_OSCURO) return 0.0f;
  float porcentaje = 100.0f * (adc - ADC_LUZ_OSCURO) /
                     (float)(ADC_LUZ_CLARA - ADC_LUZ_OSCURO);
  return constrain(porcentaje, 0.0f, 100.0f);
}

void actualizarNivelCritico() {
  if (distanciaActual < 0.0f) {
    nivelCritico = false;
    return;
  }
  if (!nivelCritico && distanciaActual >= UMBRAL_DISTANCIA_CRITICA_CM) {
    nivelCritico = true;
  } else if (nivelCritico &&
             distanciaActual <= UMBRAL_DISTANCIA_CRITICA_CM - HISTERESIS_NIVEL_CM) {
    nivelCritico = false;
  }
}

float normalizar(float valor, float minimo, float maximo) {
  if (maximo <= minimo) return 0.0f;
  return constrain((valor - minimo) / (maximo - minimo), 0.0f, 1.0f);
}

float calcularIndiceEvaporacionRelativo() {
  if (!dhtValido || isnan(presionActual)) return NAN;

  // Indice relativo, 0 a 100. Humedad alta reduce evaporacion: por eso se usa
  // AIRE SECO, no humedad alta. La presion participa, pero con peso muy bajo.
  float factorTemperatura = normalizar(temperatura,
                                       TEMPERATURA_Q25_CHIA_C,
                                       TEMPERATURA_Q75_CHIA_C);
  float factorLuz = luzPct / 100.0f;
  float factorAireSeco = normalizar(HUMEDAD_Q75_CHIA_PCT - humedad,
                                    0.0f,
                                    HUMEDAD_Q75_CHIA_PCT - HUMEDAD_Q25_CHIA_PCT);
  float factorPresionBaja = normalizar(PRESION_Q75_CHIA_HPA - presionActual,
                                       0.0f,
                                       PRESION_Q75_CHIA_HPA - PRESION_Q25_CHIA_HPA);
  return 100.0f * (PESO_LUZ_RELATIVA * factorLuz +
                    PESO_TEMPERATURA * factorTemperatura +
                    PESO_AIRE_SECO * factorAireSeco +
                    PESO_PRESION_BAJA * factorPresionBaja);
}

// ============================================================================
// FUSION Y ACTUADORES
// ============================================================================
void actualizarEstadoFusion() {
  fusionAmbientalCompleta = dhtValido && !isnan(indiceEvaporacion) &&
                            indiceEvaporacion >= UMBRAL_INDICE_EVAPORACION_PCT;

  // Prioridad: una situacion de nivel critico siempre gana a la preventiva.
  if (nivelCritico) estadoActual = ESTADO_ROJO;
  else if (fusionAmbientalCompleta) estadoActual = ESTADO_AMARILLO;
  else estadoActual = ESTADO_NORMAL;
}

// RGB de anodo comun: LOW enciende un color y HIGH lo apaga.
void escribirRGB(bool rojo, bool verde, bool azul) {
  digitalWrite(PIN_RGB_ROJO, rojo ? LOW : HIGH);
  digitalWrite(PIN_RGB_VERDE, verde ? LOW : HIGH);
  digitalWrite(PIN_RGB_AZUL, azul ? LOW : HIGH);
}

// RGB de anodo comun: PWM bajo da mas brillo. Rojo + verde tenue se percibe
// naranja; rojo + verde al maximo se percibe amarillo.
void escribirRGBBrillo(byte rojo, byte verde, byte azul) {
  analogWrite(PIN_RGB_ROJO, 255 - rojo);
  analogWrite(PIN_RGB_VERDE, 255 - verde);
  analogWrite(PIN_RGB_AZUL, 255 - azul);
}

void actualizarActuadores() {
  unsigned long ahora = millis();
  if (ahora - ultimoParpadeo >= INTERVALO_PARPADEO_MS) {
    ultimoParpadeo = ahora;
    faseLuz = !faseLuz;
  }
  if (ahora - ultimoBuzzer >= INTERVALO_BUZZER_MS) {
    ultimoBuzzer = ahora;
    faseBuzzer = !faseBuzzer;
  }

  if (estadoActual == ESTADO_ROJO) {
    // Nivel critico: SOLO rojo. Nunca se ordena encender el canal azul.
    escribirRGBBrillo(255, 0, 0);
    digitalWrite(PIN_BUZZER, faseBuzzer ? LOW : HIGH);
  } else if (estadoActual == ESTADO_AMARILLO) {
    // Indice ambiental alto: alterna amarillo y naranja, SIN sonido.
    if (faseLuz) escribirRGBBrillo(255, 255, 0);  // Amarillo.
    else escribirRGBBrillo(255, 80, 0);           // Naranja.
    digitalWrite(PIN_BUZZER, HIGH);
  } else {
    escribirRGBBrillo(0, 0, 0);
    digitalWrite(PIN_BUZZER, HIGH);
  }
}

// ============================================================================
// BOTON Y LCD
// ============================================================================
void revisarBoton() {
  int lectura = digitalRead(PIN_BOTON);

  // INPUT_PULLUP: el boton presionado lee LOW. Se actua en el flanco HIGH->LOW
  // y se bloquean 250 ms para ignorar rebotes sin exigir mantenerlo presionado.
  if (lectura == LOW && ultimaLecturaBoton == HIGH &&
      millis() - ultimoCambioPantalla >= BLOQUEO_BOTON_MS) {
    pantalla = (pantalla + 1) % NUM_PANTALLAS;
    pantallaDibujada = -1;
    ultimoCambioPantalla = millis();
    Serial.print("Pantalla: ");
    Serial.println(pantalla);
  }
  ultimaLecturaBoton = lectura;
}

void imprimirLineaFija(byte fila, const char *texto) {
  char linea[17];
  size_t longitud = strlen(texto);
  for (byte i = 0; i < 16; i++) linea[i] = i < longitud ? texto[i] : ' ';
  linea[16] = '\0';
  lcd.setCursor(0, fila);
  lcd.print(linea);
}

void mostrarEstadoFijo() {
  char linea[32];
  int evap = isnan(indiceEvaporacion) ? 0 : round(indiceEvaporacion);

  if (estadoActual == ESTADO_ROJO) {
    imprimirLineaFija(0, "ROJO:NIVEL BAJO");
    snprintf(linea, sizeof(linea), "P:%.0f SON:SI", presionActual);
    imprimirLineaFija(1, linea);
  } else if (estadoActual == ESTADO_AMARILLO) {
    imprimirLineaFija(0, "FUSION:EVAP ALTA");
    snprintf(linea, sizeof(linea), "IND:%d%% SON:NO", evap);
    imprimirLineaFija(1, linea);
  } else {
    imprimirLineaFija(0, "NORMAL");
    snprintf(linea, sizeof(linea), "P:%.0f EVAP:%d%%", presionActual, evap);
    imprimirLineaFija(1, linea);
  }
}

void mostrarPantalla() {
  if (pantallaDibujada != pantalla) {
    lcd.clear();
    pantallaDibujada = pantalla;
  }

  char linea[48];
  if (pantalla == 0) {
    if (distanciaActual < 0.0f) imprimirLineaFija(0, "Nivel: sin eco");
    else {
      snprintf(linea, sizeof(linea), "Dist: %.1f cm", distanciaActual);
      imprimirLineaFija(0, linea);
    }
    if (estadoActual == ESTADO_ROJO) imprimirLineaFija(1, "ALARMA NIVEL");
    else {
      snprintf(linea, sizeof(linea), "P: %.1f hPa", presionActual);
      imprimirLineaFija(1, linea);
    }
  } else if (pantalla == 1) {
    if (!dhtValido) {
      imprimirLineaFija(0, "Error DHT11");
      imprimirLineaFija(1, "Revise cableado");
    } else {
      snprintf(linea, sizeof(linea), "Temp: %.1f C", temperatura);
      imprimirLineaFija(0, linea);
      snprintf(linea, sizeof(linea), "Hum: %.1f %%", humedad);
      imprimirLineaFija(1, linea);
    }
  } else if (pantalla == 2) {
    snprintf(linea, sizeof(linea), "Luz rel: %.0f %%", luzPct);
    imprimirLineaFija(0, linea);
    int evap = isnan(indiceEvaporacion) ? 0 : round(indiceEvaporacion);
    snprintf(linea, sizeof(linea), "Indice: %d %%", evap);
    imprimirLineaFija(1, linea);
  } else {
    mostrarEstadoFijo();
  }
}

void imprimirLecturasSerie() {
  Serial.print("Distancia="); Serial.print(distanciaActual, 1);
  Serial.print(" cm | Presion="); Serial.print(presionActual, 1);
  Serial.print(" hPa | Luz rel="); Serial.print(luzPct, 1);
  Serial.print(" % | ADC="); Serial.print(adcLuz);
  if (dhtValido) {
    Serial.print(" | Temp="); Serial.print(temperatura, 1);
    Serial.print(" C | Hum="); Serial.print(humedad, 1);
    Serial.print(" % | Indice evap rel="); Serial.print(indiceEvaporacion, 1);
    Serial.print(" %");
  } else Serial.print(" | DHT11 sin lectura");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_FOTORESISTOR, INPUT);
  pinMode(PIN_RGB_ROJO, OUTPUT);
  pinMode(PIN_RGB_VERDE, OUTPUT);
  pinMode(PIN_RGB_AZUL, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  escribirRGB(false, false, false);
  digitalWrite(PIN_BUZZER, HIGH);

  analogReadResolution(12);
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  dht.begin();

  lcd.setCursor(0, 0);
  lcd.print("Iniciando WMS");
  if (!bmp.begin(BMP085_STANDARD, &Wire)) {
    lcd.setCursor(0, 1);
    lcd.print("BMP180 no OK");
    while (true) delay(10);
  }
  delay(1200);
  lcd.clear();
  Serial.println("WMS iniciado. Calibre ADC_LUZ_OSCURO y ADC_LUZ_CLARA.");
}

void loop() {
  unsigned long ahora = millis();

  if (ahora - ultimaLecturaSensores >= INTERVALO_SENSORES_MS) {
    ultimaLecturaSensores = ahora;
    distanciaActual = medirDistancia();
    presionActual = bmp.readPressure() / 100.0f;
    adcLuz = leerADCPromedio();
    float luzSinFiltrar = convertirLuzAPorcentaje(adcLuz);
    if (!luzInicializada) {
      luzPct = luzSinFiltrar;
      luzInicializada = true;
    } else {
    luzPct = luzPct + FACTOR_SUAVIZADO_LUZ * (luzSinFiltrar - luzPct);
    }
    actualizarNivelCritico();
  }

  if (ahora - ultimaLecturaDHT >= INTERVALO_DHT_MS) {
    ultimaLecturaDHT = ahora;
    humedad = dht.readHumidity();
    temperatura = dht.readTemperature();
    dhtValido = !isnan(humedad) && !isnan(temperatura);
    indiceEvaporacion = calcularIndiceEvaporacionRelativo();
    imprimirLecturasSerie();
  }

  actualizarEstadoFusion();
  actualizarActuadores();
  revisarBoton();

  if (ahora - ultimaActualizacionLCD >= INTERVALO_LCD_MS) {
    ultimaActualizacionLCD = ahora;
    mostrarPantalla();
  }
}
