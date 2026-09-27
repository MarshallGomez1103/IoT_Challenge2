# Contexto del Reto y Requisitos

---

## La Crisis Climática en Colombia: El Impacto de El Niño

Durante el presente año, el territorio colombiano ha enfrentado uno de los escenarios climáticos más complejos de las últimas décadas debido a la [confirmación oficial del inicio del **Fenómeno de El Niño**](https://www.minambiente.gov.co/gobierno-confirma-inicio-del-fenomeno-de-el-nino-y-alerta-sobre-su-alcance/). La escasez de precipitaciones y las anomalías térmicas han desencadenado alertas críticas en múltiples regiones, afectando directamente la seguridad hídrica y la estabilidad de los ecosistemas locales. 

Según los [planes de contingencia y reportes institucionales](https://www.cundinamarca.gov.co/), la situación en el departamento exige acciones inmediatas:

| Cifras de la Emergencia | Impacto Territorial |
| :--- | :--- |
| **Déficit de Lluvias:** Afectación severa y prolongada en las cuencas altas abastecedoras de Sabana Centro. | **Riesgo Inminente:** [20 municipios de Cundinamarca en nivel de riesgo extremo por desabastecimiento hídrico e incendios](https://caracol.com.co/2026/06/16/fenomeno-del-nino-los-municipios-en-cundinamarca-con-mayor-riesgo-de-desabastecimiento-de-agua/). |
| **Escenarios de Riesgo:** [16 tipificaciones de riesgo en Bogotá y la Sabana](https://www.infobae.com/colombia/2026/07/08/fenomeno-de-el-nino-en-colombia-estos-son-los-16-escenarios-de-riesgo-que-podria-vivir-bogota/), incluyendo caídas críticas en el nivel de los embalses. | **Impacto Sistémico:** Amenaza directa al suministro de agua potable comunitaria, la agricultura y la salud pública. |

---

> ### *"¿Cómo es posible que no estemos cuidando cada gota de agua?"*
> 
> En un territorio donde el recurso hídrico solía percibirse como inagotable, la crisis actual nos confronta con una realidad ineludible: la inacción ya no es una opción. Cada litro de agua perdido o mal administrado acelera el desabastecimiento de comunidades enteras. 
>
> A partir de esta pregunta y urgencia nace la idea fundamental del **Water Monitoring System (WMS)**, un prototipo IoT de bajo costo concebido para democratizar el monitoreo del agua y empoderar a las autoridades y comunidades locales para tomar decisiones de racionamiento y protección de manera anticipada.

---

## Filosofía de Diseño y Requisitos del Sistema

El desarrollo del WMS se rige por tres pilares fundamentales que garantizan su viabilidad operativa en entornos rurales, respondiendo de forma directa a las necesidades del clima actual:

* **Despliegue "Plug and Play"**
  El dispositivo está diseñado para una instalación inmediata en los puntos críticos de suministro y almacenamiento. Su puesta en marcha requiere un esfuerzo técnico mínimo, permitiendo que las autoridades locales activen el monitoreo rápidamente sin necesidad de configuraciones de red complejas.
* **Notificación "In-Situ"**
  La resiliencia comunitaria depende de la velocidad de reacción. El sistema integra alarmas físicas (sonoras/visuales) y una pantalla local que informan de manera instantánea a los habitantes de la zona sobre caídas críticas en los niveles de agua, sin depender de que revisen un teléfono o tengan acceso a internet.
* **Tablero de Control Local y Seguro (Dashboard)**
  Para un análisis exhaustivo de estadísticas e históricos, el sistema aloja un servidor web embebido. Las autoridades pueden conectarse mediante credenciales seguras exclusivamente a la red Wi-Fi (WLAN) de la zona para auditar datos, revisar eventualidades y desactivar alarmas remotamente de forma segura.

---

## Restricciones Técnicas y Decisiones de Ingeniería

El diseño del prototipo se enfrenta a limitaciones severas de infraestructura en zonas vulnerables. Para garantizar su funcionamiento, se establecieron las siguientes restricciones:

| Parámetro de Diseño | Restricción / Solución Implementada |
| :--- | :--- |
| **Conectividad Reducida** | **Sin acceso a internet global (No MQTT).** La operación está restringida exclusivamente a una red de área local inalámbrica (WLAN) de la Alcaldía. |
| **Consumo Energético** | Operación crítica en zonas rurales, exigiendo un sistema eficiente para priorizar el ahorro de batería y la sostenibilidad energética a largo plazo. |
| **Seguridad de Datos** | El acceso al tablero de control está bloqueado para dispositivos externos, protegiendo la información mediante el cruce exclusivo en la red local. |

### Justificación de Hardware: ESP32 vs. Raspberry Pi

Una de las decisiones de ingeniería más críticas fue la elección del "cerebro" del sistema. Aunque una microcomputadora (como Raspberry Pi) ofrece mayor capacidad de procesamiento, el **WMS opta por el microcontrolador ESP32** basándose en las siguientes razones de peso:

1. **Ahorro de Batería Extremo:** El ESP32 consume una fracción mínima de energía frente a una Raspberry Pi (que demanda alimentación constante por su sistema operativo). Esto es vital para las restricciones energéticas del proyecto en campo.
2. **Procesamiento Concurrente de Bajo Nivel:** A pesar de ser más ligero, el ESP32 permite ejecutar la medición de sensores en segundo plano utilizando Rutinas de Servicio de Interrupción (ISR) o hilos dedicados (vía FreeRTOS), evitando cuellos de botella en la ejecución principal sin gastar recursos extra.
3. **Costo-Beneficio y Escalabilidad:** Cumple estrictamente con el mandato de ser un sistema de "bajo costo", facilitando que el dispositivo sea escalable y replicable en los múltiples municipios afectados de la región Sabana Centro.

---
*Nota: La bibliografía técnica detallada y todos los documentos de referencia se encuentran listados de forma exhaustiva en la [Página 10: Referencias y anexos](10-Referencias-y-anexos).*

## Preguntas para completar esta página

### Contexto y pregunta guía

- ¿Qué fuentes recientes respaldan cada cifra o afirmación sobre la necesidad hídrica de Sabana Centro y cómo las citamos en formato IEEE?
- ¿Cuál es la pregunta guía del proyecto que integra nivel del agua, condiciones ambientales, fusión y tablero local?

### Requisitos y alcance

- ¿Qué evidencia observable podemos presentar para cada requisito de medición de nivel, índice ambiental, fusión, alarmas y tablero?
- ¿Cómo comprobamos que el usuario puede ver el historial y las notificaciones, y silenciar la alarma física?
- ¿Qué requisitos tomamos directamente del enunciado y cuáles decidimos agregar como equipo?
- ¿Qué partes del segundo corte terminamos, cuáles dejamos parciales y cuáles quedaron fuera de alcance?

### Conectividad y medición

- ¿Qué red Wi-Fi usamos en la entrega, cómo conectamos el celular y qué diferencia hay entre un punto de acceso de demostración y la WLAN autorizada?
- ¿Cómo implementamos la captura de ECHO mediante una ISR breve y procesamos las demás tareas en el superloop, sin crear hilos o tareas?
- ¿Qué respuesta nos dio el docente sobre esta división entre interrupción y superloop?
- ¿Qué piezas prestadas tuvimos que recuperar y rearmar después del primer corte, y qué inconvenientes encontramos?
