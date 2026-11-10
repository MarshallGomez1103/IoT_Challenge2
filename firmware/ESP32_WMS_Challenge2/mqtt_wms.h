#pragma once           // Evita incluir este archivo dos veces.
#include <WiFi.h>      // Permite comprobar la conexión al Wi-Fi.
#include <MQTT.h>      // Proporciona el cliente MQTT.
#include "secrets.h"   // Carga la IP y las credenciales locales.

struct LecturaMqtt {  // Agrupa las dos mediciones en una sola muestra.
  float nivelCm;      // Guarda el nivel de agua.
  float tempC;        // Guarda la temperatura del DHT11.
};
static QueueHandle_t colaNivelMqtt = nullptr;
static void tareaMqtt(void *) {                      // Ejecuta MQTT en una tarea propia.
  WiFiClient red;                                    // Abre la conexión de red.
  MQTTClient mqtt(256);                              // Crea el cliente MQTT.
  mqtt.begin(WMS_MQTT_HOST, 1883, red);               // Apunta a Mosquitto en la Pi.

  for (;;) {                                         // Mantiene la tarea en ejecución.
    if (WiFi.status() != WL_CONNECTED) {             // Espera a que el Wi-Fi esté listo.
      vTaskDelay(pdMS_TO_TICKS(1000));                // Espera un segundo.
      continue;                                      // Vuelve a comprobar el Wi-Fi.
    }

    if (!mqtt.connected()) {                         // Comprueba si falta la sesión MQTT.
      if (!mqtt.connect("wms-esp32-01", WMS_MQTT_USER, WMS_MQTT_PASSWORD)) {
        Serial.printf("MQTT falló: error=%d, broker=%d\n",
                      (int)mqtt.lastError(), (int)mqtt.returnCode());  // Muestra el motivo.
        vTaskDelay(pdMS_TO_TICKS(5000));              // Espera antes de reintentar.
        continue;                                    // Vuelve a intentar la conexión.
      }
      Serial.println("MQTT conectado a Mosquitto");   // Confirma que el broker aceptó la sesión.

      const char *estado = "{\"device\":\"esp32-01\",\"status\":\"online\"}";  // Arma el mensaje de estado.
      bool enviado = mqtt.publish("wms/esp32/status", estado, false, 1);       // Publica con confirmación del broker.
      Serial.println(enviado ? "Estado MQTT enviado" : "Error al enviar estado MQTT");  // Muestra el resultado.
    }

    mqtt.loop();                                     // Mantiene activa la sesión.
    LecturaMqtt lectura;  // Recibe la muestra del bucle principal.
    if (xQueueReceive(colaNivelMqtt, &lectura, 0) == pdTRUE) {  // Lee la cola sin esperar.
      char mensaje[80];  // Reserva espacio para el JSON.
      if (isfinite(lectura.tempC)) {  // Incluye la temperatura si el DHT11 dio un valor válido.
        snprintf(mensaje, sizeof(mensaje), "{\"water_level_cm\":%.1f,\"temp_c\":%.1f}",
                lectura.nivelCm, lectura.tempC);  // Empaqueta ambas mediciones.
      } else {
        snprintf(mensaje, sizeof(mensaje), "{\"water_level_cm\":%.1f}",
                lectura.nivelCm);  // Si falta temperatura, envía el nivel.
      }
      bool enviado = mqtt.publish("wms/esp32/telemetry", mensaje, false, 1);  // Publica con QoS 1.
      Serial.println(enviado ? mensaje : "Error al publicar la lectura");  // Muestra el resultado.
    }
    vTaskDelay(pdMS_TO_TICKS(50));                    // Da tiempo a las demás tareas.
  }
}