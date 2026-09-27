# Revisión del segundo corte

## Qué hay en el primer corte

El sketch de referencia integra ESP32, HC-SR04 para distancia al agua, BMP180 para presión, DHT11 para temperatura y humedad, fotoresistor como proxy de luz, LCD 16x2, LED RGB, buzzer y botón. El prototipo ya tiene pantallas, estados y un índice ambiental relativo.

La lógica actual no fusiona nivel y ambiente en un único nivel de riesgo: si la distancia alcanza 15 cm se declara rojo; si el índice ambiental alcanza 70 % se declara amarillo; el primero tiene prioridad. El código sí combina luz, temperatura, humedad y presión dentro de un índice relativo, pero el nivel del agua queda como condición separada.

## Brechas funcionales frente al enunciado

| Requisito | Estado observado en el código del primer corte | Pendiente para el segundo |
|---|---|---|
| Nivel o caudal | Se mide distancia al agua con `pulseIn()` | Capturar los flancos de ECHO por interrupción y calibrar distancia contra el recipiente real |
| Evaporación y variables ambientales | Existe índice relativo con luz, temperatura, humedad y presión | Justificar parámetros, mostrar límites y llamarlo proxy/índice relativo mientras no haya medición calibrada de radiación o evapotranspiración |
| Fusión | Nivel crítico e índice ambiental activan estados separados | Combinar riesgo de nivel e índice ambiental en un solo puntaje con reglas y umbrales justificados |
| Medición por ISR o hilo | Todas las lecturas están dentro de `loop()`; no hay ISR | La propuesta usa ISR para medir el pulso del HC-SR04 y mantiene las demás lecturas en el superloop. El DHT11 y el BMP180 no se deben leer desde una ISR; confirmar con el docente que la interpretación híbrida cumple el requisito |
| WLAN local | No hay Wi-Fi | Conectar como estación a la WLAN autorizada para la entrega; el AP directo se deja solo como modo de demostración |
| Servidor embebido y tablero | No hay servidor ni página | Servir la página desde el ESP32, con valores actuales, historial reciente, avisos y control para silenciar alarma |
| Alarma y visualización in situ | LCD, LED RGB y buzzer ya existen en el sketch | Integrar su estado con la fusión y el control de silencio desde el tablero |

## Retroalimentación que deben cerrar

La rúbrica del primer corte indicó falta de evidencia y justificación para el cálculo de evaporación y los umbrales; fusión no explicada desde diseño; diagramas sin interfaces; ausencia de esquemático de interconexión y estándares; documentación parcial del código; citas IEEE no ubicadas en contexto; porcentaje de escritura con IA sin declarar; y actas no adjuntas. El profesor también pidió claridad al explicar la lógica de fusión en la sustentación.

La nueva rúbrica está en blanco para el equipo y pondera 60 % diseño, 30 % comunicación y 10 % aprendizaje. Incluye diseño con restricciones y estándares, validación del prototipo, Wiki técnica con citas IEEE y actas/evaluación del equipo.

## Documento de Terán y Aranda (2017)

El artículo se titula *IoT-based System for Indoor Location using Bluetooth Low Energy*. Sirve como referencia general para organizar un sistema IoT por módulos (adquisición, agregación, comunicación y visualización) y para pensar en validación experimental. No trata monitoreo hídrico ni fusión ambiental. Su arquitectura describe un servidor central/plataformas externas y usa BLE; no se debe trasladar esa arquitectura al prototipo porque el reto exige que el servidor web esté dentro del microcontrolador, use la WLAN local y no use MQTT ni Raspberry Pi.

## Entregables y fechas que aparecen en el enunciado

- Wiki/documentación en Teams: 28 de septiembre de 2026.
- Sustentación: el encabezado dice 29 de septiembre a la 1:00 p. m.; una sección posterior dice 1:30 p. m. y además describe un espacio de 10 minutos junto con 3 minutos de pitch y 15 minutos de preguntas. Confirmar horario y duración con el profesor.
- Video de validación: máximo 5 minutos, reproducible desde Teams y con todos los integrantes participando y en cámara.
- Incluir roles, actividades, contribuciones, actas, anexos y declaración de uso de IA. Para IA: herramienta, enlace o instrucciones exactas, cómo se verificó/aplicó y porcentaje de escritura asistida.

## Pendientes prácticos del equipo

- [ ] Reunir y volver a montar las piezas prestadas; confirmar modelo de placa y cableado real.
- [ ] Comprobar que ECHO del HC-SR04 entra al ESP32 mediante divisor de tensión (el sensor puede entregar 5 V y el ESP32 usa GPIO de 3.3 V).
- [ ] Medir la distancia sensor-superficie con recipiente lleno, nivel de aviso y nivel crítico; reemplazar los valores provisionales del sketch.
- [ ] Calibrar luz oscura/clara y registrar lecturas repetibles.
- [ ] Definir WLAN autorizada, credenciales fuera del repositorio y dispositivos de prueba.
- [ ] Ejecutar la matriz de pruebas del equipo; guardar datos, capturas, fotos y video como evidencia.
- [ ] Completar diagramas, esquemático, criterios, estándares, referencias IEEE en contexto, actas y contribuciones.
- [ ] Confirmar con el profesor si el requisito de ISR/hilo admite ISR para el sensor de nivel y lecturas ambientales en el superloop.

## Cómo nombrar la arquitectura al sustentar

Describirla como “superloop de la aplicación en `loop()` + ISR GPIO para capturar ECHO, sin tareas adicionales creadas por el equipo”. Evitar llamarla bare-metal puro: el core Arduino-ESP32 ejecuta `loop()` dentro de una tarea de FreeRTOS.
