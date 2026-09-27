#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include <string.h>

// Crear secrets.h a partir de secrets.example.h antes de compilar.
#include "secrets.h"

/*
  WMS - segundo corte
  Superloop Arduino (sin tareas/hilos creados por la aplicación).
  El HC-SR04 se mide capturando los flancos ECHO con una ISR GPIO.
  BMP180, DHT11 y LDR se leen fuera de la ISR porque sus interfaces requieren
  transacciones I2C o rutinas temporizadas.
*/

// Pines del sketch del primer corte: confirmar contra el cableado rearmado.
const uint8_t PIN_TRIG = 18;
const uint8_t PIN_ECHO = 19; // ECHO de 5 V del HC-SR04: usar divisor hacia 3.3 V.
const uint8_t PIN_DHT = 27;
const uint8_t PIN_LDR = 34;
const uint8_t PIN_RGB_R = 25;
const uint8_t PIN_RGB_G = 26;
const uint8_t PIN_RGB_B = 32;
const uint8_t PIN_BUZZER = 33; // Buzzer activo en LOW, según el sketch previo.
const uint8_t PIN_BUTTON = 14;

#define DHT_TYPE DHT11

// Puntos de calibración provisionales. Reemplazar con mediciones del recipiente.
const float DISTANCIA_NIVEL_OPERATIVO_CM = 5.0f;
const float DISTANCIA_NIVEL_CRITICO_CM = 15.0f; // Umbral que venía del corte 1.
// Pesos provisionales de política; no se han ajustado con observaciones del tanque.
const float PESO_RIESGO_NIVEL = 0.80f;
const float PESO_RIESGO_AMBIENTAL = 0.20f;
// Anticipación de alarma, pendiente de ajustar al tiempo de respuesta del equipo.
const float HORIZONTE_ALARMA_MIN = 5.0f;
const float HORIZONTE_SALIDA_ALARMA_MIN = 7.5f;
const float UMBRAL_PREVENTIVO = 40.0f;
const float UMBRAL_CRITICO = 70.0f;
const float HIST_PREVENTIVO_SALIDA = 35.0f;
const float HIST_CRITICO_SALIDA = 65.0f;

// Valores de referencia previos; comprobar su origen, vigencia y pertinencia.
const float PESO_LUZ = 0.7474f;
const float PESO_TEMPERATURA = 0.1479f;
const float PESO_AIRE_SECO = 0.1026f;
const float PESO_PRESION_BAJA = 0.0021f;
const float TEMP_Q25_C = 12.38f;
const float TEMP_Q75_C = 13.43f;
const float HUM_Q25_PCT = 83.89f;
const float HUM_Q75_PCT = 87.34f;
const float PRES_Q25_HPA = 758.8f;
const float PRES_Q75_HPA = 759.8f;
const int ADC_LUZ_OSCURO = 3500;
const int ADC_LUZ_CLARA = 600;

const uint32_t INTERVALO_ECHO_MS = 80;
const uint32_t TIMEOUT_ECHO_MS = 35;
const uint32_t INTERVALO_AMBIENTE_MS = 1000;
const uint32_t INTERVALO_DHT_MS = 2000;
const uint32_t INTERVALO_REINTENTO_BMP_MS = 3000;
const uint32_t INTERVALO_HISTORIAL_MS = 2000;
const uint32_t INTERVALO_LCD_MS = 250;
const uint32_t INTERVALO_WIFI_MS = 15000;
const uint8_t HISTORIAL_MAX = 60; // 60 muestras × 2 s ≈ 2 min en RAM.
const uint8_t NUM_PANTALLAS = 4;

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_BMP085 bmp;
DHT dht(PIN_DHT, DHT_TYPE);
WebServer server(80);

enum EstadoAlerta : uint8_t {
  ESTADO_FALLA = 0,
  ESTADO_NORMAL = 1,
  ESTADO_PREVENTIVO = 2,
  ESTADO_CRITICO = 3
};

// La ISR solo captura tiempos y levanta una bandera; no hace cálculos ni I/O.
volatile uint32_t echoRiseUs = 0;
volatile uint32_t echoWidthUs = 0;
volatile bool echoRiseSeen = false;
volatile bool echoReady = false;
volatile bool echoArmed = false;

float distanciaCm = NAN;
float temperaturaC = NAN;
float humedadPct = NAN;
float presionHpa = NAN;
float luzPct = NAN;
float indiceAmbientalPct = NAN;
float riesgoNivelPct = NAN;
float riesgoFusionPct = NAN;
float velocidadDescensoCmMin = NAN;
float tiempoAlLimiteMin = NAN;
int adcLuz = 0;
bool distanciaValida = false;
bool dhtValido = false;
bool bmpValido = false;
bool ldrEnRangoCalibrado = false;
bool wifiConectado = false;
bool servidorIniciado = false;
bool alarmaSilenciada = false;
bool faseBuzzer = false;
bool faseRoja = false;
uint8_t pantalla = 0;
uint8_t pantallaDibujada = 255;
int botonAnterior = HIGH;
uint32_t ultimoEcoMs = 0;
uint32_t ultimoDisparoEcoMs = 0;
uint32_t inicioEsperaEcoMs = 0;
uint32_t ultimaLecturaAmbienteMs = 0;
uint32_t ultimaLecturaDhtMs = 0;
uint32_t ultimoIntentoBmpMs = 0;
uint32_t ultimaMuestraHistorialMs = 0;
uint32_t ultimaActualizacionLcdMs = 0;
uint32_t ultimoIntentoWifiMs = 0;
uint32_t ultimaAlarmaMs = 0;
uint32_t ultimoParpadeoMs = 0;
uint32_t ultimoCambioPantallaMs = 0;
bool esperandoEco = false;
bool luzInicializada = false;
bool bmpDisponible = false;
bool alarmaPorTendencia = false;
EstadoAlerta estadoActual = ESTADO_FALLA;

struct Muestra {
  uint32_t segundos;
  float distancia;
  float ambiente;
  float fusion;
  uint8_t estado;
};

Muestra historial[HISTORIAL_MAX];
uint8_t historialSiguiente = 0;
uint8_t historialCantidad = 0;

const char PAGINA[] PROGMEM = R"HTML(
<!doctype html><html lang="es"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#0b1017">
<title>WMS · Monitor local</title>
<style>
:root{color-scheme:dark;--bg:#0b1017;--panel:#121a24;--panel2:#17222e;--line:#263443;--text:#edf4f8;--muted:#9cabb8;--green:#42d3aa;--yellow:#ffc65c;--red:#ff6876;--blue:#7db8ff}
*{box-sizing:border-box}body{margin:0;min-height:100vh;background:var(--bg);color:var(--text);font:15px/1.45 system-ui,-apple-system,"Segoe UI",sans-serif}
main{width:min(100%,760px);margin:0 auto;padding:20px 16px 34px}.top{display:flex;justify-content:space-between;align-items:center;gap:12px;margin-bottom:18px}.brand{display:flex;align-items:center;gap:11px}.mark{display:grid;place-items:center;width:40px;height:40px;border-radius:13px;background:#173b39;color:var(--green);font-weight:800;letter-spacing:.04em}.brand h1{font-size:1.05rem;line-height:1.2;margin:0}.brand p,.muted{color:var(--muted);font-size:.82rem;margin:3px 0 0}.network{border:1px solid var(--line);border-radius:999px;padding:7px 10px;color:var(--muted);font-size:.75rem;white-space:nowrap}.network[data-online="true"]{color:var(--green);border-color:#285448}
h2{font-size:.98rem;margin:22px 0 10px}.risk-card{background:linear-gradient(145deg,#17242d,#111923);border:1px solid var(--line);border-radius:20px;padding:19px}.risk-head{display:flex;justify-content:space-between;align-items:center;gap:10px}.eyebrow,.label{color:var(--muted);font-size:.79rem}.state{font-size:.73rem;font-weight:750;letter-spacing:.04em;border:1px solid var(--line);border-radius:999px;padding:6px 10px;color:var(--muted)}.state[data-state="NORMAL"]{color:var(--green);border-color:#285448}.state[data-state="PREVENTIVO"]{color:var(--yellow);border-color:#67502b}.state[data-state="CRITICO"],.state[data-state="FALLA"]{color:var(--red);border-color:#67313a}.risk-value{font-size:clamp(2.6rem,14vw,4.3rem);font-weight:760;line-height:1.05;letter-spacing:-.055em;margin:15px 0 4px}.risk-value small{font-size:.42em;letter-spacing:0;color:var(--muted)}.risk-sub{color:var(--muted);font-size:.84rem}.track{height:8px;background:#263342;border-radius:99px;overflow:hidden;margin:15px 0 10px}.bar{height:100%;width:0;background:var(--green);border-radius:inherit;transition:width .35s ease,background .2s}.risk-card[data-state="PREVENTIVO"] .bar{background:var(--yellow)}.risk-card[data-state="CRITICO"] .bar,.risk-card[data-state="FALLA"] .bar{background:var(--red)}.formula{font-size:.75rem;color:var(--muted)}.alert{margin-top:12px;padding:11px 13px;border:1px solid var(--line);border-radius:12px;background:#101821;color:var(--muted);font-size:.85rem}.alert[data-state="PREVENTIVO"]{border-color:#67502b;color:#ffdda0}.alert[data-state="CRITICO"],.alert[data-state="FALLA"]{border-color:#67313a;color:#ffc1c7}
.grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px}.metric{background:var(--panel);border:1px solid var(--line);border-radius:15px;padding:14px;min-width:0}.value{font-size:clamp(1.2rem,5.5vw,1.65rem);font-weight:700;letter-spacing:-.025em;margin-top:8px;overflow-wrap:anywhere}.detail{font-size:.73rem;color:var(--muted);margin-top:5px}.sensor-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:9px}.sensor{background:var(--panel);border:1px solid var(--line);border-radius:13px;padding:12px;min-width:0}.sensor-top{display:flex;justify-content:space-between;align-items:center;gap:8px}.sensor-name{font-weight:650;font-size:.82rem}.pill{font-size:.67rem;line-height:1.25;border-radius:999px;padding:5px 7px;background:#26313c;color:var(--muted);text-align:center}.pill[data-ok="true"]{background:#17362f;color:var(--green)}.pill[data-ok="false"]{background:#3b2929;color:#ffb0b6}.fineprint{font-size:.75rem;color:var(--muted);line-height:1.5;margin:14px 2px 0}button{width:100%;margin-top:13px;border:1px solid #67404a;border-radius:12px;padding:12px;background:#38242a;color:#ffd6da;font:inherit;font-weight:650}button:disabled{opacity:.7}
@media(min-width:620px){main{padding-top:28px}.grid{gap:12px}.metric{padding:17px}.sensor-grid{grid-template-columns:repeat(4,minmax(0,1fr))}.risk-card{padding:23px}}
@media(max-width:360px){main{padding-left:11px;padding-right:11px}.grid{gap:8px}.metric{padding:11px}.sensor{padding:10px}.top{align-items:flex-start}.network{font-size:.68rem;padding:6px 8px}}
</style></head><body><main>
<header class="top"><div class="brand"><div class="mark">W</div><div><h1>Water Monitoring</h1><p>Monitor local del ESP32</p></div></div><div id="network" class="network" data-online="false">Conectando…</div></header>
<section id="risk-card" class="risk-card" data-state="FALLA" aria-labelledby="risk-title">
 <div class="risk-head"><div id="risk-title" class="eyebrow">Riesgo fusionado</div><span id="state" class="state" data-state="FALLA">Cargando</span></div>
 <div id="risk" class="risk-value">—<small> %</small></div>
 <div id="risk-sub" class="risk-sub">Esperando lecturas válidas de los sensores.</div>
 <div class="track" role="progressbar" aria-label="Riesgo fusionado" aria-valuemin="0" aria-valuemax="100"><div id="risk-bar" class="bar"></div></div>
 <div class="formula">Fusión provisional: 80% nivel + 20% ambiente. La alarma crítica también estima el tiempo hasta el límite.</div>
</section>
<div id="alert" class="alert" data-state="FALLA" aria-live="polite">Esperando datos del ESP32…</div>
<h2>Mediciones</h2>
<div class="grid">
 <article class="metric"><div class="label">Distancia al agua</div><div id="level" class="value">—</div><div class="detail">Sensor a superficie; referencias de riesgo: 5 y 15 cm, por calibrar.</div></article>
 <article class="metric"><div class="label">Índice ambiental relativo</div><div id="ambient" class="value">—</div><div class="detail">Índice del entorno; no representa el porcentaje de agua.</div></article>
 <article class="metric"><div class="label">Temperatura</div><div id="temperature" class="value">—</div><div class="detail">DHT11 · grados Celsius</div></article>
 <article class="metric"><div class="label">Humedad relativa</div><div id="humidity" class="value">—</div><div class="detail">DHT11 · porcentaje</div></article>
 <article class="metric"><div class="label">Presión atmosférica</div><div id="pressure" class="value">—</div><div class="detail">BMP180 · hPa</div></article>
 <article class="metric"><div class="label">Luz relativa</div><div id="light" class="value">—</div><div id="light-detail" class="detail">LDR · lectura por calibrar</div></article>
</div>
<h2>Estado de sensores</h2>
<div class="sensor-grid">
 <article class="sensor"><div class="sensor-top"><span class="sensor-name">HC-SR04</span><span id="sensor-ultra" class="pill" data-ok="false">Sin lectura</span></div><div class="detail">Distancia y eco</div></article>
 <article class="sensor"><div class="sensor-top"><span class="sensor-name">DHT11</span><span id="sensor-dht" class="pill" data-ok="false">Sin lectura</span></div><div class="detail">Temperatura y humedad</div></article>
 <article class="sensor"><div class="sensor-top"><span class="sensor-name">BMP180</span><span id="sensor-bmp" class="pill" data-ok="false">Sin lectura</span></div><div class="detail">Presión atmosférica</div></article>
 <article class="sensor"><div class="sensor-top"><span class="sensor-name">LDR · ADC</span><span id="sensor-ldr" class="pill" data-ok="false">Sin validar</span></div><div class="detail">Rango calibrado, no presencia física</div></article>
</div>
<p class="fineprint">El índice ambiental usa las lecturas del DHT11, BMP180 y LDR. Las referencias y ponderaciones son provisionales y deben validarse con el montaje. El ESP32 comprueba lecturas y rango ADC; el pin analógico no puede confirmar por sí solo que el LDR esté conectado.</p>
<button id="silence" hidden>Silenciar buzzer</button>
<p id="updated" class="fineprint">Actualizando cada 2 segundos.</p>
</main><script>
const byId=id=>document.getElementById(id);let loading=false;
const stateLabel=s=>({FALLA:'FALLA',NORMAL:'NORMAL',PREVENTIVO:'PREVENTIVO',CRITICO:'CRÍTICO'}[s]||'SIN ESTADO');
function number(v,d=1,suffix=''){return Number.isFinite(v)?v.toFixed(d)+suffix:'—'}
function setPill(id,ok,good,bad){const e=byId(id);e.textContent=ok?good:bad;e.dataset.ok=ok?'true':'false'}
function updateView(d){const s=d.state||'FALLA',sensors=d.sensors||{};const title=stateLabel(s);byId('state').textContent=title;byId('state').dataset.state=s;byId('risk-card').dataset.state=s;byId('alert').dataset.state=s;byId('network').textContent=d.wifi_ready?'ESP32 en línea':'Wi-Fi sin servidor';byId('network').dataset.online=d.wifi_ready?'true':'false';
 byId('risk').innerHTML=number(d.fusion_pct,0)+'<small> %</small>';byId('risk-sub').textContent=s==='FALLA'?'Fusión no disponible: faltan lecturas válidas.':s==='CRITICO'&&d.trend_alarm?'Alarma anticipada por descenso proyectado del nivel.':s==='CRITICO'?'Riesgo crítico según nivel y ambiente.':s==='PREVENTIVO'?'Riesgo preventivo según la fusión de nivel y ambiente.':'Lecturas válidas; fusión dentro del rango normal.';
 const score=Number.isFinite(d.fusion_pct)?Math.max(0,Math.min(100,d.fusion_pct)):0;byId('risk-bar').style.width=score+'%';byId('risk-bar').parentElement.setAttribute('aria-valuenow',String(Math.round(score)));
 let mensajeCritico=d.silenced?'Alarma crítica silenciada.':'Alarma crítica activa.';if(d.trend_alarm){if(Number.isFinite(d.time_to_critical_min)&&d.time_to_critical_min>0)mensajeCritico='Descenso estimado: '+number(d.falling_rate_cm_min,1,' cm/min')+'; límite en ~'+number(d.time_to_critical_min,1,' min')+'.';else mensajeCritico='Se alcanzó el límite de distancia.';if(d.silenced)mensajeCritico+=' Buzzer silenciado.'}const messages={FALLA:'Fusión no disponible: revise el sensor que falta.',NORMAL:'Estado normal según el único riesgo fusionado.',PREVENTIVO:'Estado preventivo según el único riesgo fusionado.',CRITICO:mensajeCritico};byId('alert').textContent=messages[s]||'Esperando estado del firmware.';
 byId('level').textContent=number(d.level_cm,1,' cm');byId('ambient').textContent=number(d.ambient_pct,0,'%');byId('temperature').textContent=number(d.temp_c,1,' °C');byId('humidity').textContent=number(d.rh_pct,0,'%');byId('pressure').textContent=number(d.pressure_hpa,1,' hPa');byId('light').textContent=number(d.light_pct,0,'%');
 byId('light-detail').textContent=sensors.ldr_in_calibrated_range?'LDR · valor relativo dentro del rango calibrado':'LDR · sin valor dentro del rango calibrado';
 setPill('sensor-ultra',!!sensors.ultrasonic_valid,'Lectura vigente','Sin eco válido');setPill('sensor-dht',!!sensors.dht_valid,'Lectura válida','Sin lectura válida');setPill('sensor-bmp',!!sensors.bmp_valid,'Lectura válida','Sin lectura válida');setPill('sensor-ldr',!!sensors.ldr_in_calibrated_range,'En rango ADC','Fuera de rango');
 const silence=byId('silence');silence.hidden=s!=='CRITICO';silence.disabled=!!d.silenced;silence.textContent=d.silenced?'Buzzer silenciado':'Silenciar buzzer';byId('updated').textContent='Actualizado: '+new Date().toLocaleTimeString();
}
async function refresh(){if(loading)return;loading=true;const controller=new AbortController();const timeout=setTimeout(()=>controller.abort(),2500);try{const response=await fetch('/api/status',{cache:'no-store',signal:controller.signal});if(!response.ok)throw Error('HTTP');updateView(await response.json())}catch(e){byId('network').textContent='ESP32 sin respuesta';byId('network').dataset.online='false';byId('state').textContent='SIN RESPUESTA';byId('state').dataset.state='FALLA';byId('risk-card').dataset.state='FALLA';byId('risk').innerHTML='—<small> %</small>';byId('risk-sub').textContent='Sin datos actuales del ESP32.';byId('risk-bar').style.width='0%';['level','ambient','temperature','humidity','pressure','light'].forEach(id=>byId(id).textContent='—');setPill('sensor-ultra',false,'Lectura vigente','Sin respuesta');setPill('sensor-dht',false,'Lectura válida','Sin respuesta');setPill('sensor-bmp',false,'Lectura válida','Sin respuesta');setPill('sensor-ldr',false,'En rango ADC','Sin respuesta');byId('silence').hidden=true;byId('alert').dataset.state='FALLA';byId('alert').textContent='No se pudo consultar el ESP32. Comprueba que sigues conectado a su Wi-Fi.'}finally{clearTimeout(timeout);loading=false}}
byId('silence').onclick=async()=>{try{await fetch('/api/silence',{method:'POST'});refresh()}catch(e){byId('alert').textContent='No se pudo enviar el comando al ESP32.'}};refresh();setInterval(refresh,2000);
</script></body></html>
)HTML";

void IRAM_ATTR capturarEcoISR() {
  if (!echoArmed) return;
  uint32_t ahoraUs = micros();
  if (digitalRead(PIN_ECHO) == HIGH) {
    echoRiseUs = ahoraUs;
    echoRiseSeen = true;
  } else if (echoRiseSeen) {
    echoWidthUs = ahoraUs - echoRiseUs;
    echoRiseSeen = false;
    echoReady = true;
  }
}

float limitar(float x, float minimo, float maximo) {
  if (x < minimo) return minimo;
  if (x > maximo) return maximo;
  return x;
}

float normalizar(float x, float minimo, float maximo) {
  if (!isfinite(x) || maximo <= minimo) return NAN;
  return limitar((x - minimo) / (maximo - minimo), 0.0f, 1.0f);
}

float convertirLuzPct(int adc) {
  if (ADC_LUZ_OSCURO == ADC_LUZ_CLARA) return NAN;
  float pct = 100.0f * (adc - ADC_LUZ_OSCURO) /
              (float)(ADC_LUZ_CLARA - ADC_LUZ_OSCURO);
  return limitar(pct, 0.0f, 100.0f);
}

float calcularIndiceAmbiental() {
  if (!dhtValido || !bmpValido || !isfinite(luzPct)) return NAN;
  float t = normalizar(temperaturaC, TEMP_Q25_C, TEMP_Q75_C);
  float seco = normalizar(HUM_Q75_PCT - humedadPct, 0.0f, HUM_Q75_PCT - HUM_Q25_PCT);
  float presionBaja = normalizar(PRES_Q75_HPA - presionHpa, 0.0f, PRES_Q75_HPA - PRES_Q25_HPA);
  if (!isfinite(t) || !isfinite(seco) || !isfinite(presionBaja)) return NAN;
  return 100.0f * (PESO_LUZ * (luzPct / 100.0f) +
                   PESO_TEMPERATURA * t +
                   PESO_AIRE_SECO * seco +
                   PESO_PRESION_BAJA * presionBaja);
}

float calcularRiesgoNivel() {
  if (!distanciaValida || DISTANCIA_NIVEL_CRITICO_CM <= DISTANCIA_NIVEL_OPERATIVO_CM) return NAN;
  return limitar(100.0f * (distanciaCm - DISTANCIA_NIVEL_OPERATIVO_CM) /
                 (DISTANCIA_NIVEL_CRITICO_CM - DISTANCIA_NIVEL_OPERATIVO_CM), 0.0f, 100.0f);
}

float calcularVelocidadDescensoCmMin(uint32_t ahoraMs) {
  if (historialCantidad < 8) return NAN;

  uint8_t ultimoIdx = (historialSiguiente + HISTORIAL_MAX - 1) % HISTORIAL_MAX;
  const Muestra &ultima = historial[ultimoIdx];
  uint32_t ahoraSeg = ahoraMs / 1000UL;
  if (!isfinite(ultima.distancia) || ahoraSeg - ultima.segundos > 3) return NAN;

  const uint32_t ventanaSeg = 30;
  uint32_t ultimoSeg = ultima.segundos;
  uint32_t primeroSeg = ultimoSeg;
  float sumaX = 0.0f, sumaY = 0.0f, sumaXX = 0.0f, sumaXY = 0.0f;
  uint8_t cantidad = 0;

  // Usa el tramo continuo más reciente de distancias válidas para amortiguar ruido.
  for (uint8_t i = 0; i < historialCantidad; ++i) {
    uint8_t idx = (ultimoIdx + HISTORIAL_MAX - i) % HISTORIAL_MAX;
    const Muestra &m = historial[idx];
    if (ultimoSeg - m.segundos > ventanaSeg) break;
    if (!isfinite(m.distancia)) break;

    float x = -((float)(ultimoSeg - m.segundos));
    float y = m.distancia;
    sumaX += x;
    sumaY += y;
    sumaXX += x * x;
    sumaXY += x * y;
    primeroSeg = m.segundos;
    ++cantidad;
  }

  if (cantidad < 8 || ultimoSeg - primeroSeg < 14) return NAN;
  float n = (float)cantidad;
  float denominador = n * sumaXX - sumaX * sumaX;
  if (denominador <= 0.0f) return NAN;
  float pendienteCmPorSegundo = (n * sumaXY - sumaX * sumaY) / denominador;
  return pendienteCmPorSegundo * 60.0f;
}

void cambiarEstado(EstadoAlerta nuevo) {
  if (nuevo == estadoActual) return;
  if (nuevo == ESTADO_NORMAL) alarmaSilenciada = false;
  else if (nuevo == ESTADO_PREVENTIVO || nuevo == ESTADO_CRITICO) alarmaSilenciada = false;
  estadoActual = nuevo;
}

void actualizarFusion() {
  uint32_t ahoraMs = millis();
  bool nivelValido = distanciaValida && (ahoraMs - ultimoEcoMs <= 500);
  velocidadDescensoCmMin = calcularVelocidadDescensoCmMin(ahoraMs);
  tiempoAlLimiteMin = NAN;
  alarmaPorTendencia = false;

  if (nivelValido && distanciaCm >= DISTANCIA_NIVEL_CRITICO_CM) {
    tiempoAlLimiteMin = 0.0f;
  } else if (nivelValido && isfinite(velocidadDescensoCmMin) && velocidadDescensoCmMin > 0.0f) {
    tiempoAlLimiteMin = (DISTANCIA_NIVEL_CRITICO_CM - distanciaCm) / velocidadDescensoCmMin;
  }

  if (!nivelValido) {
    riesgoNivelPct = NAN;
    riesgoFusionPct = NAN;
    cambiarEstado(ESTADO_FALLA);
    return;
  }

  riesgoNivelPct = calcularRiesgoNivel();
  if (!isfinite(riesgoNivelPct)) {
    riesgoFusionPct = NAN;
    cambiarEstado(ESTADO_FALLA);
    return;
  }

  bool ambienteValido = dhtValido && bmpValido && isfinite(indiceAmbientalPct);
  float horizonteActivoMin = estadoActual == ESTADO_CRITICO
                                 ? HORIZONTE_SALIDA_ALARMA_MIN
                                 : HORIZONTE_ALARMA_MIN;
  alarmaPorTendencia = isfinite(tiempoAlLimiteMin) && tiempoAlLimiteMin <= horizonteActivoMin;

  // Si falta el ambiente, conservamos la alarma de nivel/velocidad y reportamos FALLA
  // cuando no haya peligro de nivel; no convertimos una señal ausente en cero.
  if (!ambienteValido) {
    riesgoFusionPct = NAN;
    if (distanciaCm >= DISTANCIA_NIVEL_CRITICO_CM || alarmaPorTendencia)
      cambiarEstado(ESTADO_CRITICO);
    else
      cambiarEstado(ESTADO_FALLA);
    return;
  }

  riesgoFusionPct = PESO_RIESGO_NIVEL * riesgoNivelPct +
                    PESO_RIESGO_AMBIENTAL * indiceAmbientalPct;

  EstadoAlerta siguiente = estadoActual;
  if (distanciaCm >= DISTANCIA_NIVEL_CRITICO_CM || alarmaPorTendencia) {
    siguiente = ESTADO_CRITICO;
  } else if (estadoActual == ESTADO_CRITICO) {
    if (riesgoFusionPct < HIST_CRITICO_SALIDA)
      siguiente = riesgoFusionPct < HIST_PREVENTIVO_SALIDA ? ESTADO_NORMAL : ESTADO_PREVENTIVO;
  } else if (estadoActual == ESTADO_PREVENTIVO) {
    if (riesgoFusionPct >= UMBRAL_CRITICO) siguiente = ESTADO_CRITICO;
    else if (riesgoFusionPct < HIST_PREVENTIVO_SALIDA) siguiente = ESTADO_NORMAL;
  } else {
    if (riesgoFusionPct >= UMBRAL_CRITICO) siguiente = ESTADO_CRITICO;
    else if (riesgoFusionPct >= UMBRAL_PREVENTIVO) siguiente = ESTADO_PREVENTIVO;
    else siguiente = ESTADO_NORMAL;
  }
  cambiarEstado(siguiente);
}

void iniciarMedicionEcho(uint32_t ahoraMs) {
  noInterrupts();
  echoRiseSeen = false;
  echoReady = false;
  echoArmed = true;
  interrupts();
  esperandoEco = true;
  inicioEsperaEcoMs = ahoraMs;
  ultimoDisparoEcoMs = ahoraMs;
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
}

void procesarEcho(uint32_t ahoraMs) {
  bool lista;
  uint32_t anchoUs;
  noInterrupts();
  lista = echoReady;
  anchoUs = echoWidthUs;
  if (lista) {
    echoReady = false;
    echoArmed = false;
  }
  interrupts();

  if (lista) {
    esperandoEco = false;
    ultimoEcoMs = ahoraMs;
    if (anchoUs >= 116 && anchoUs <= 23500) {
      distanciaCm = anchoUs * 0.0343f / 2.0f;
      distanciaValida = true;
    } else {
      distanciaCm = NAN;
      distanciaValida = false;
    }
  } else if (esperandoEco && ahoraMs - inicioEsperaEcoMs >= TIMEOUT_ECHO_MS) {
    noInterrupts();
    echoArmed = false;
    echoRiseSeen = false;
    interrupts();
    esperandoEco = false;
    ultimoEcoMs = ahoraMs;
    distanciaCm = NAN;
    distanciaValida = false;
  }
}

void leerSensoresAmbientales(uint32_t ahoraMs) {
  if (esperandoEco || ahoraMs - ultimaLecturaAmbienteMs < INTERVALO_AMBIENTE_MS) return;
  ultimaLecturaAmbienteMs = ahoraMs;

  if (!bmpDisponible && ahoraMs - ultimoIntentoBmpMs >= INTERVALO_REINTENTO_BMP_MS) {
    ultimoIntentoBmpMs = ahoraMs;
    bmpDisponible = bmp.begin(BMP085_STANDARD, &Wire);
  }
  if (bmpDisponible) {
    float p = bmp.readPressure() / 100.0f;
    bmpValido = isfinite(p) && p > 0.0f;
    presionHpa = bmpValido ? p : NAN;
    if (!bmpValido) bmpDisponible = false;
  } else {
    bmpValido = false;
    presionHpa = NAN;
  }

  long suma = 0;
  for (uint8_t i = 0; i < 8; ++i) suma += analogRead(PIN_LDR);
  adcLuz = suma / 8;
  // Los extremos de calibración son saturación, no una lectura confiable de presencia.
  ldrEnRangoCalibrado = adcLuz > ADC_LUZ_CLARA && adcLuz < ADC_LUZ_OSCURO;
  float luzNueva = ldrEnRangoCalibrado ? convertirLuzPct(adcLuz) : NAN;
  if (!ldrEnRangoCalibrado) {
    luzPct = NAN;
    luzInicializada = false;
  } else if (!luzInicializada) {
    luzPct = luzNueva;
    luzInicializada = true;
  } else if (isfinite(luzNueva)) {
    luzPct += 0.20f * (luzNueva - luzPct);
  }

  if (ahoraMs - ultimaLecturaDhtMs >= INTERVALO_DHT_MS) {
    // La biblioteca DHT es temporizada y puede bloquear brevemente el loop.
    // No se inicia una medición ultrasónica mientras este bloque se ejecuta.
    ultimaLecturaDhtMs = ahoraMs;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    dhtValido = isfinite(h) && isfinite(t);
    humedadPct = dhtValido ? h : NAN;
    temperaturaC = dhtValido ? t : NAN;
  }
  indiceAmbientalPct = calcularIndiceAmbiental();
}

const char *nombreEstado(EstadoAlerta e) {
  switch (e) {
    case ESTADO_NORMAL: return "NORMAL";
    case ESTADO_PREVENTIVO: return "PREVENTIVO";
    case ESTADO_CRITICO: return "CRITICO";
    default: return "FALLA";
  }
}

void agregarFloatJson(String &json, float valor, unsigned int decimales) {
  if (isfinite(valor)) json += String(valor, decimales);
  else json += "null";
}

String crearEstadoJson() {
  String json;
  json.reserve(480);
  json = "{\"state\":\"";
  json += nombreEstado(estadoActual);
  json += "\",\"level_cm\":"; agregarFloatJson(json, distanciaCm, 1);
  json += ",\"temp_c\":"; agregarFloatJson(json, temperaturaC, 1);
  json += ",\"rh_pct\":"; agregarFloatJson(json, humedadPct, 1);
  json += ",\"pressure_hpa\":"; agregarFloatJson(json, presionHpa, 1);
  json += ",\"light_pct\":"; agregarFloatJson(json, luzPct, 1);
  json += ",\"ambient_pct\":"; agregarFloatJson(json, indiceAmbientalPct, 1);
  json += ",\"fusion_pct\":"; agregarFloatJson(json, riesgoFusionPct, 1);
  json += ",\"falling_rate_cm_min\":"; agregarFloatJson(json, velocidadDescensoCmMin, 2);
  json += ",\"time_to_critical_min\":"; agregarFloatJson(json, tiempoAlLimiteMin, 2);
  json += ",\"trend_alarm\":"; json += alarmaPorTendencia ? "true" : "false";
  json += ",\"sensors\":{";
  json += "\"ultrasonic_valid\":"; json += distanciaValida ? "true" : "false";
  json += ",\"dht_valid\":"; json += dhtValido ? "true" : "false";
  json += ",\"bmp_valid\":"; json += bmpValido ? "true" : "false";
  json += ",\"ldr_in_calibrated_range\":"; json += ldrEnRangoCalibrado ? "true" : "false";
  json += "},\"wifi_ready\":"; json += servidorIniciado ? "true" : "false";
  json += ",\"silenced\":";
  json += alarmaSilenciada ? "true" : "false";
  json += ",\"uptime_s\":"; json += String(millis() / 1000UL);
  json += "}";
  return json;
}

void guardarMuestra(uint32_t ahoraMs) {
  Muestra &m = historial[historialSiguiente];
  m.segundos = ahoraMs / 1000UL;
  m.distancia = distanciaValida ? distanciaCm : NAN;
  m.ambiente = isfinite(indiceAmbientalPct) ? indiceAmbientalPct : NAN;
  m.fusion = isfinite(riesgoFusionPct) ? riesgoFusionPct : NAN;
  m.estado = (uint8_t)estadoActual;
  historialSiguiente = (historialSiguiente + 1) % HISTORIAL_MAX;
  if (historialCantidad < HISTORIAL_MAX) ++historialCantidad;
}

String crearHistorialJson() {
  String json;
  json.reserve(60 + historialCantidad * 100);
  json = "[";
  uint8_t primero = (historialSiguiente + HISTORIAL_MAX - historialCantidad) % HISTORIAL_MAX;
  for (uint8_t i = 0; i < historialCantidad; ++i) {
    const Muestra &m = historial[(primero + i) % HISTORIAL_MAX];
    if (i) json += ",";
    json += "{\"t\":"; json += String(m.segundos);
    json += ",\"level_cm\":"; agregarFloatJson(json, m.distancia, 1);
    json += ",\"ambient_pct\":"; agregarFloatJson(json, m.ambiente, 1);
    json += ",\"fusion_pct\":"; agregarFloatJson(json, m.fusion, 1);
    json += ",\"state\":\""; json += nombreEstado((EstadoAlerta)m.estado); json += "\"}";
  }
  json += "]";
  return json;
}

void configurarServidor() {
  server.on("/", HTTP_GET, []() { server.send_P(200, "text/html; charset=utf-8", PAGINA); });
  server.on("/api/status", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", crearEstadoJson());
  });
  server.on("/api/history", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", crearHistorialJson());
  });
  server.on("/api/silence", HTTP_POST, []() {
    alarmaSilenciada = true;
    server.send(200, "application/json", "{\"ok\":true}");
  });
  server.onNotFound([]() { server.send(404, "text/plain", "Ruta no encontrada"); });
}

void iniciarWifi() {
#if WMS_WIFI_MODE_AP
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(WMS_AP_SSID, WMS_AP_PASSWORD);
  Serial.println(ok ? "AP demo iniciado" : "No se pudo iniciar AP demo");
#else
  WiFi.mode(WIFI_STA);
  WiFi.begin(WMS_WIFI_SSID, WMS_WIFI_PASSWORD);
  ultimoIntentoWifiMs = millis();
  Serial.println("Conectando a la WLAN autorizada…");
#endif
}

void atenderWifiYServidor(uint32_t ahoraMs) {
#if WMS_WIFI_MODE_AP
  wifiConectado = WiFi.getMode() == WIFI_AP; // El AP queda activo aunque no haya clientes.
#else
  wifiConectado = WiFi.status() == WL_CONNECTED;
  if (!wifiConectado && ahoraMs - ultimoIntentoWifiMs >= INTERVALO_WIFI_MS) {
    ultimoIntentoWifiMs = ahoraMs;
    WiFi.reconnect();
  }
#endif

  if (wifiConectado && !servidorIniciado) {
    configurarServidor();
    server.begin();
    servidorIniciado = true;
#if WMS_WIFI_MODE_AP
    Serial.print("Tablero demo: http://"); Serial.println(WiFi.softAPIP());
#else
    Serial.print("Tablero WLAN: http://"); Serial.println(WiFi.localIP());
#endif
  }
  if (!wifiConectado && servidorIniciado) {
    server.stop();
    servidorIniciado = false;
  }
  if (servidorIniciado) server.handleClient();
}

void imprimirLinea(uint8_t fila, const char *texto) {
  char s[17];
  size_t n = strlen(texto);
  for (uint8_t i = 0; i < 16; ++i) s[i] = (i < n) ? texto[i] : ' ';
  s[16] = '\0';
  lcd.setCursor(0, fila);
  lcd.print(s);
}

void mostrarLcd(uint32_t ahoraMs) {
  if (ahoraMs - ultimaActualizacionLcdMs < INTERVALO_LCD_MS) return;
  ultimaActualizacionLcdMs = ahoraMs;
  if (pantalla != pantallaDibujada) {
    lcd.clear();
    pantallaDibujada = pantalla;
  }
  char linea[32];
  if (pantalla == 0) {
    if (distanciaValida) snprintf(linea, sizeof(linea), "Dist: %.1f cm", distanciaCm);
    else snprintf(linea, sizeof(linea), "Nivel: sin eco");
    imprimirLinea(0, linea);
    snprintf(linea, sizeof(linea), "Riesgo: %.0f%%", riesgoFusionPct);
    if (!isfinite(riesgoFusionPct)) snprintf(linea, sizeof(linea), "Riesgo: FALLA");
    imprimirLinea(1, linea);
  } else if (pantalla == 1) {
    if (dhtValido) {
      snprintf(linea, sizeof(linea), "Temp: %.1f C", temperaturaC); imprimirLinea(0, linea);
      snprintf(linea, sizeof(linea), "Hum: %.1f %%", humedadPct); imprimirLinea(1, linea);
    } else { imprimirLinea(0, "DHT11 sin lectura"); imprimirLinea(1, "Revise conexion"); }
  } else if (pantalla == 2) {
    snprintf(linea, sizeof(linea), "Pres: %.1f hPa", presionHpa);
    if (!bmpValido) snprintf(linea, sizeof(linea), "BMP180 sin dato");
    imprimirLinea(0, linea);
    if (ldrEnRangoCalibrado) snprintf(linea, sizeof(linea), "Luz rel: %.0f%%", luzPct);
    else snprintf(linea, sizeof(linea), "LDR fuera rango");
    imprimirLinea(1, linea);
  } else {
    snprintf(linea, sizeof(linea), "Estado: %s", nombreEstado(estadoActual)); imprimirLinea(0, linea);
    if (!wifiConectado) snprintf(linea, sizeof(linea), "WiFi: sin red");
    else snprintf(linea, sizeof(linea), "WiFi: conectado");
    imprimirLinea(1, linea);
  }
}

void actualizarBoton() {
  int lectura = digitalRead(PIN_BUTTON);
  uint32_t ahora = millis();
  if (lectura == LOW && botonAnterior == HIGH && ahora - ultimoCambioPantallaMs >= 250) {
    pantalla = (pantalla + 1) % NUM_PANTALLAS;
    pantallaDibujada = 255;
    ultimoCambioPantallaMs = ahora;
  }
  botonAnterior = lectura;
}

void actualizarActuadores(uint32_t ahoraMs) {
  if (ahoraMs - ultimoParpadeoMs >= 350) {
    ultimoParpadeoMs = ahoraMs;
    faseRoja = !faseRoja;
  }
  if (ahoraMs - ultimaAlarmaMs >= 140) {
    ultimaAlarmaMs = ahoraMs;
    faseBuzzer = !faseBuzzer;
  }

  // RGB de ánodo común: PWM invertido, conservando el montaje previo.
  uint8_t r = 0, g = 0, b = 0;
  if (estadoActual == ESTADO_NORMAL) g = 170;
  else if (estadoActual == ESTADO_PREVENTIVO) { r = 255; g = 105; }
  else if (estadoActual == ESTADO_CRITICO) r = faseRoja ? 255 : 30;
  else { r = 180; g = 45; } // Falla: ámbar, sin ocultar el error como estado normal.
  analogWrite(PIN_RGB_R, 255 - r);
  analogWrite(PIN_RGB_G, 255 - g);
  analogWrite(PIN_RGB_B, 255 - b);

  bool sonar = estadoActual == ESTADO_CRITICO && !alarmaSilenciada && faseBuzzer;
  digitalWrite(PIN_BUZZER, sonar ? LOW : HIGH);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_RGB_R, OUTPUT);
  pinMode(PIN_RGB_G, OUTPUT);
  pinMode(PIN_RGB_B, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  digitalWrite(PIN_TRIG, LOW);
  digitalWrite(PIN_BUZZER, HIGH);

  analogReadResolution(12);
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  dht.begin();
  bmpDisponible = bmp.begin(BMP085_STANDARD, &Wire);
  ultimoIntentoBmpMs = millis();
  if (!bmpDisponible) Serial.println("BMP180 no detectado; se reintentara en el superloop.");

  attachInterrupt(digitalPinToInterrupt(PIN_ECHO), capturarEcoISR, CHANGE);
  iniciarWifi();
  ultimaLecturaDhtMs = millis() - INTERVALO_DHT_MS;
  ultimaLecturaAmbienteMs = millis() - INTERVALO_AMBIENTE_MS;
  ultimaMuestraHistorialMs = millis();
  Serial.println("WMS listo; completar calibracion antes de evaluar umbrales.");
}

void loop() {
  uint32_t ahoraMs = millis();

  // Superloop: atiende red, sensores, fusión, actuadores y pantalla sin hilos.
  atenderWifiYServidor(ahoraMs);
  procesarEcho(ahoraMs);
  if (!esperandoEco && ahoraMs - ultimoDisparoEcoMs >= INTERVALO_ECHO_MS)
    iniciarMedicionEcho(ahoraMs);
  leerSensoresAmbientales(ahoraMs);
  actualizarFusion();
  actualizarActuadores(ahoraMs);
  actualizarBoton();
  mostrarLcd(ahoraMs);

  if (ahoraMs - ultimaMuestraHistorialMs >= INTERVALO_HISTORIAL_MS) {
    ultimaMuestraHistorialMs = ahoraMs;
    guardarMuestra(ahoraMs);
  }
}
