# Diseño de la Solución: Arquitectura y Modelo de Negocio

---

> ### ❖ Filosofía de Diseño: "Todo en Uno"
> A diferencia de los sistemas IoT fragmentados, el **Water Monitoring System (WMS)** fue concebido bajo una arquitectura centralizada. La premisa es simple pero poderosa: un único módulo robusto, hermético y de despliegue táctico (*Plug & Play*). No requiere ensamblajes complejos en campo; basta con fijarlo, energizarlo y el monitoreo comienza.

---

## Arquitectura General del Módulo Físico

El diseño del dispositivo integra la captura de datos, el procesamiento y la interfaz de usuario en un solo chasis. A continuación, se presenta el modelo 3D del prototipo y la distribución estratégica de sus componentes:

<p align="center">
<img width="1376" height="768" alt="modelo 3d" src="https://github.com/user-attachments/assets/91a25c84-689a-4682-8b60-813d07a89c59" />
</p>

### Desglose de la Interfaz y Sensores
* **◆ Interfaz Local:** Pantalla LCD superior y matriz de 4 botones (*Push Buttons*) para navegación in-situ y desactivación manual de alarmas.
* **◆ Percepción Hidráulica:** Sensor ultrasónico orientado hacia la superficie del agua para el cálculo preciso del nivel y caudal.
* **◆ Percepción Meteorológica:** Sensor de luz superior y ranuras de ventilación laterales (*Vents*) para la captura de temperatura ambiental.
* **◆ Alertas Físicas:** *Buzzer* integrado para la emisión de alarmas sonoras inmediatas ante caídas críticas del nivel hídrico.

---

## Ingeniería Creativa: Simulación de Variables Físicas

Uno de los retos más grandes en la fase de prototipado es la recreación de condiciones ambientales extremas. Para validar la lógica de fusión de sensores (temperatura, humedad y presión) sin depender de fluctuaciones climáticas reales, el equipo implementó una solución de entorno controlado:

**El Micro-Clima Encapsulado (Sensor en Globo):**
Como se observa en el diseño, el sensor de humedad y presión ha sido introducido dentro de un globo elástico acoplado al chasis. Esto permite al equipo inyectar aire o alterar la presión interna de forma manual, simulando variaciones atmosféricas drásticas (como las provocadas por el Fenómeno de El Niño) para probar la respuesta del sistema y la activación de las alertas en tiempo real.

---

## Estándares de Ingeniería: Modelado UML

Para garantizar que la transición entre el hardware físico y la lógica de software fuera impecable, todo el ciclo de vida del software fue diseñado utilizando el **Lenguaje Unificado de Modelado (UML)**. 

| Aplicación del Estándar UML | Beneficio en el Proyecto |
| :--- | :--- |
| **Diagramas de Casos de Uso** | Definición clara de las interacciones entre las autoridades locales (actores) y el tablero de control web. |
| **Diagramas de Actividad** | Mapeo de la lógica de fusión de sensores y el flujo de las Rutinas de Servicio de Interrupción (ISR). |
| **Diagramas de Componentes** | Estructuración modular del código C++ para el ESP32, separando la lectura de sensores del servidor web. |

---

## Justificación de Restricciones: Seguridad y Conectividad

El documento técnico del reto exige que el sistema **no utilice MQTT** y opere exclusivamente bajo una red WLAN local. Lejos de ser una limitación, esta restricción fue adoptada como una **arquitectura de máxima seguridad**:

1. **Aislamiento de Red (Air-Gapping Parcial):** Al no estar conectado a la internet global, el WMS es invulnerable a ciberataques externos.
2. **Control Gubernamental Exclusivo:** Solo los dispositivos (PCs o teléfonos celulares) conectados físicamente a la red Wi-Fi de la Alcaldía en las inmediaciones de la zona pueden acceder al *Dashboard*.
3. **Integridad de Datos:** Garantiza que las alarmas físicas solo puedan ser desactivadas por personal autorizado que se encuentre en el perímetro de la red local.

---

## Modelo de Negocio: Alianza Estratégica B2G

El WMS no está diseñado para el mercado de consumo masivo (B2C), sino que opera bajo un modelo **B2G (Business-to-Government)**. 

**Propuesta de Valor Institucional:**
El proyecto busca establecer alianzas directas con **Alcaldías Municipales, la Corporación Autónoma Regional (CAR) y el IDEAM**. Al ofrecer un sistema de muy bajo costo de producción, las entidades gubernamentales pueden adquirir e implementar decenas de estos módulos *Plug & Play* a lo largo de las cuencas hídricas de Sabana Centro. 

El modelo se basa en proveer la infraestructura de hardware a las autoridades, empoderándolas con una herramienta de vigilancia distribuida que complementa sus redes institucionales actuales, salvaguardando así la salud pública y el desarrollo agroindustrial de la región.

## Preguntas para completar esta página

### Enfoque y criterios

- ¿Cómo hicimos evolucionar el sistema del primer corte al segundo y qué necesidad guía ese cambio?
- ¿Qué criterios medibles fijamos para costo, seguridad, precisión, facilidad de uso y mantenimiento?
- ¿Qué límites, frecuencia de muestreo y reglas de alarma definimos, y qué mediciones o fuentes respaldan cada decisión?
- ¿Qué estándares de ingeniería aplicamos y dónde está la evidencia de su uso?

### Arquitectura y fusión

- ¿Qué diagrama de bloques incluiremos para mostrar hardware, software, conexiones y datos entre módulos?
- ¿Qué interfaces definimos para conectar adquisición, fusión, red, tablero, visualización y actuación?
- ¿Qué partes ejecutamos en el ESP32 y cuáles quedan en el celular o computador cliente?
- ¿Qué módulos componen nuestro firmware y qué diagrama UML coincide con el código que implementamos?
- ¿Cuál es la ecuación final de fusión, qué representa cada variable y cómo se normalizan sus valores?
- ¿Qué pruebas realizamos para mostrar que el nivel y las variables ambientales contribuyen al mismo indicador de riesgo?
- ¿Cómo justificamos pesos, umbrales, histéresis y el tratamiento de lecturas inválidas?

### Modelo y prototipo

- ¿Cuál es el costo aproximado de construcción y quién instalaría, operaría y mantendría el dispositivo?
- ¿Qué elementos podrían escalarse y cuáles limitan una instalación fuera del laboratorio?
- ¿Qué recipiente y soporte usamos en el prototipo físico y dónde están el render y la foto del montaje final?
- ¿Qué evidencia reunimos para respaldar las afirmaciones de seguridad y qué límites tiene operar solo en una red local?
