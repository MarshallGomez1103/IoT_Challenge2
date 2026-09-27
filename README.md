# IoT Challenge 2 — Water Monitoring System

Firmware del sistema de monitoreo de agua para ESP32.

## Archivos

- `firmware/ESP32_WMS_Challenge2/ESP32_WMS_Challenge2.ino`: sketch del segundo corte.
- `firmware/ESP32_WMS_Challenge2/secrets.example.h`: plantilla de configuración Wi-Fi.
- `reference/corte1/ESP32_Sensores.ino`: sketch de referencia del primer corte.

## Configuración

1. Copia `firmware/ESP32_WMS_Challenge2/secrets.example.h` como `secrets.h` en la carpeta del sketch.
2. Configura allí las credenciales WLAN autorizadas o activa el modo punto de acceso para pruebas directas.
3. No publiques ni agregues `secrets.h` al repositorio; solo se comparte el archivo de ejemplo.
4. En Arduino IDE selecciona la placa ESP32 correspondiente e instala LiquidCrystal_I2C, Adafruit BMP085 y DHT sensor library. Wi-Fi y WebServer forman parte del core ESP32.

## Verificación reportada

El usuario reporta que cargó el firmware al ESP32 y que el servidor web funciona. La calibración del recipiente y la validación individual de los sensores deben acompañarse con las pruebas del grupo.
