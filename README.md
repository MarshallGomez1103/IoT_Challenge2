# IoT Challenge 2 — Water Monitoring System

Repositorio del proyecto del segundo corte. Contiene el firmware del ESP32, la documentación de diseño y una plantilla local para completar la Wiki con el grupo. La Wiki vive en un repositorio separado de GitHub y su publicación se gestiona aparte.

## Contenido

- `firmware/ESP32_WMS_Challenge2/`: firmware del ESP32 y configuración de red de ejemplo.
- `docs/REVISION_Y_PENDIENTES.md`: brechas frente al enunciado y la retroalimentación del primer corte.
- `docs/LOGICA_DE_FUSION.md`: auditoría del 80/20, límites de la evidencia, método local de calibración y una sola regla de alarma basada en nivel/velocidad.
- `docs/PROPUESTA_EVAPORACION_OBSERVADA.md`: propuesta futura, todavía no implementada, para medir pérdida aparente de nivel y proyectar una hora simulada usando 1 minuto real por hora.
- `docs/ESQUEMA_PROTEUS.md` y `docs/esquema_wms.svg`: mapa de conexiones según el firmware y prompt reutilizable para dibujar el esquemático.
- `docs/plantilla_registro_calibracion.csv`: encabezados para comenzar a registrar datos reales; todavía no contiene mediciones.
- `tools/registrar_muestras.py` y `tools/calibrar_peso_fusion.py`: registro local desde `/api/status` y ajuste/validación temporal de `alpha` cuando haya suficientes datos reales (Python 3.10 o posterior, solo biblioteca estándar).
- `wiki-template/`: guion Markdown de 10 páginas, siguiendo la navegación de la Wiki del primer corte y añadiendo preguntas para el tablero, WLAN y fusión.
- `reference/corte1/ESP32_Sensores.ino`: copia de referencia del sketch anterior, sin modificar.

## Antes de abrir el sketch

1. Si aún no existe, copiar `firmware/ESP32_WMS_Challenge2/secrets.example.h` como `secrets.h` en la misma carpeta. En Linux/macOS se puede usar `cp -n firmware/ESP32_WMS_Challenge2/secrets.example.h firmware/ESP32_WMS_Challenge2/secrets.h`; `-n` evita sobrescribir una configuración local existente.
2. En la entrega final, usar `WMS_WIFI_MODE_AP 0` y completar la WLAN autorizada indicada para el sitio. El modo AP es solo para pruebas directas con el celular y no reemplaza el requisito de acceso mediante la WLAN de la zona.
3. Consultar `docs/ESQUEMA_PROTEUS.md`, confirmar el mapa de pines y la geometría real del recipiente. Los valores iniciales de nivel y los pesos de fusión están marcados como provisionales.
4. Instalar en Arduino IDE las mismas bibliotecas del sketch previo: LiquidCrystal_I2C, Adafruit BMP085, DHT sensor library. WiFi y WebServer vienen con el core ESP32.

## Manejo de credenciales

- Completar la WLAN solo en `firmware/ESP32_WMS_Challenge2/secrets.h`.
- El `.gitignore` excluye ese archivo local. `secrets.example.h` contiene marcadores y sí es el archivo de ejemplo para compartir.
- Confirmar el patrón localmente con `git check-ignore -v firmware/ESP32_WMS_Challenge2/secrets.h`. No agregues `secrets.h` con `git add -f`.
- La carpeta puede conservar un `secrets.h` local para compilar; no se debe publicar su contenido.

## Estado de verificación

El usuario reporta que cargó y probó el ESP32 y que el servidor web responde correctamente. En esta sesión no se volvió a compilar ni a ejecutar el hardware; la calibración del recipiente, el comportamiento con cada sensor conectado/desconectado y la validación de la lógica de alarma deben registrarse con evidencias del grupo. El archivo `secrets.h` local no se incluye en Git.

## Repositorio y Wiki

El repositorio de código es [IoT_Challenge2](https://github.com/MarshallGomez1103/IoT_Challenge2); su Wiki se publica en el repositorio separado [IoT_Challenge2 Wiki](https://github.com/MarshallGomez1103/IoT_Challenge2/wiki).

- `wiki/`: espejo local de la Wiki fusionada. Conserva el contenido de las páginas 1–4 con los emojis retirados y preguntas pendientes; las páginas 5–10 quedan como guion de preguntas.
- `wiki-template/`: plantilla original de preguntas, conservada como referencia.
- `reference/wiki_corte1/`: Wiki del primer corte para consultar su navegación.
