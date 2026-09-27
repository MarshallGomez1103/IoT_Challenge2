# Water Monitoring System (WMS)

**IoT Challenge 2 · Universidad de La Sabana · 2026-2**

## Presentación

Somos un equipo de estudiantes de Internet de las Cosas y presentamos un prototipo basado en ESP32 para observar el nivel de agua en un recipiente. El dispositivo reúne esa medición con variables ambientales y permite consultar el estado del sistema desde una pantalla local o desde un tablero web servido por el propio ESP32 a través de Wi-Fi.

El proyecto continúa el dispositivo del primer corte. En este segundo corte reincorporamos el hardware y añadimos el servidor web embebido, la visualización desde el celular y una lógica de fusión que combina el nivel con las lecturas ambientales para mostrar un estado de riesgo. Los pesos y umbrales siguen sujetos a calibración con mediciones del recipiente.

## Equipo

| Integrante | Aporte descrito en la Wiki del equipo |
|---|---|
| **Jasub Sastre** | Planeación del proyecto, arquitectura y seguimiento de requisitos. |
| **Thomas** | Integración de hardware y apoyo al desarrollo y validación del prototipo. |
| **Andrés Suárez** | Gestión de componentes, logística y apoyo al ensamblaje. |

El ensamblaje físico se realizó como esfuerzo conjunto. Los roles resumen la distribución que aparece en la Wiki del equipo.

## Qué presentamos

- **Nivel de agua:** el HC-SR04 mide la distancia hasta la superficie; con la geometría calibrada del recipiente, esa distancia permite estimar el nivel.
- **Variables ambientales:** DHT11 para temperatura y humedad relativa, BMP180 para presión atmosférica y LDR para luz relativa.
- **Interfaz local:** LCD I²C de 16×2, LED RGB, buzzer y botón para silenciar la alarma.
- **Tablero web:** alojado en el ESP32 y accesible desde un celular conectado a la misma red Wi-Fi. Muestra las lecturas, el estado del sistema y el historial reciente guardado en RAM.
- **Fusión y alertas:** el firmware combina un indicador de riesgo por nivel con un componente ambiental. El resultado es una lógica experimental que debe calibrarse y validarse con datos del montaje.

La captura del eco del HC-SR04 se realiza mediante una interrupción breve; el cálculo, las lecturas ambientales, la atención del servidor, la pantalla y las alarmas se atienden en el superloop de la aplicación, sin tareas adicionales creadas por el equipo.

## Archivos de este repositorio

- [`firmware/ESP32_WMS_Challenge2/ESP32_WMS_Challenge2.ino`](firmware/ESP32_WMS_Challenge2/ESP32_WMS_Challenge2.ino): firmware del segundo corte.
- [`firmware/ESP32_WMS_Challenge2/secrets.example.h`](firmware/ESP32_WMS_Challenge2/secrets.example.h): plantilla de configuración Wi-Fi.
- [`reference/corte1/ESP32_Sensores.ino`](reference/corte1/ESP32_Sensores.ino): firmware de referencia del primer corte.

## Preparar y cargar el firmware

1. Copia `secrets.example.h` como `secrets.h` en la carpeta del sketch y configura una red autorizada o el modo punto de acceso para pruebas.
2. No publiques `secrets.h`; este repositorio solo debe contener el archivo de ejemplo.
3. Abre el sketch en Arduino IDE, selecciona la placa ESP32 correspondiente e instala las bibliotecas LiquidCrystal_I2C, Adafruit BMP085 y DHT sensor library. Wi-Fi y WebServer forman parte del core ESP32.
4. En modo punto de acceso, conecta el celular a la red configurada en `secrets.h` y abre `http://192.168.4.1/`. En modo estación, abre la dirección IP que el ESP32 recibe del router.

## Estado del prototipo

El equipo reporta que cargó el firmware y comprobó el servidor web en el ESP32. La calibración del recipiente y los resultados de las pruebas individuales de sensores deben documentarse a partir de las mediciones del grupo.
