#pragma once

// Copiar este archivo como secrets.h en la misma carpeta del sketch.
// No subir secrets.h al repositorio.

// 0 = estación Wi-Fi conectada a la WLAN autorizada (modo de entrega).
// 1 = punto de acceso ESP32 para pruebas directas con el celular.
#define WMS_WIFI_MODE_AP 0

#define WMS_WIFI_SSID "CONFIGURAR_SSID_AUTORIZADO"
#define WMS_WIFI_PASSWORD "CONFIGURAR_CLAVE"

#define WMS_AP_SSID "WMS-Demo"
#define WMS_AP_PASSWORD "Cambiar123" // Mínimo 8 caracteres; cambiar antes de usar.
