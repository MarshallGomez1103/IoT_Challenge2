# Water Monitoring System (WMS)
> **Solución IoT de Bajo Costo para la Resiliencia Hídrica ante el Fenómeno de El Niño**

---

<p align="center">
  <img src="https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge&logo=espressif" alt="Plataforma">
  <img src="https://img.shields.io/badge/Language-C%2B%2B%20(Arduino%20IDE)-00599C?style=for-the-badge&logo=c%2B%2B" alt="Lenguaje">
  <img src="https://img.shields.io/badge/Status-Functional%20Prototype-success?style=for-the-badge" alt="Estado">
</p>

---

## Presentación del Proyecto

El **Water Monitoring System (WMS)** es un sistema embebido de Internet de las Cosas (IoT) diseñado estratégicamente para el monitoreo en tiempo real del nivel, caudal y comportamiento de los recursos hídricos en la región de Sabana Centro. Ante la crisis climática agudizada por el **Fenómeno de El Niño**, el WMS actúa como una herramienta comunitaria y gubernamental de alerta temprana. 

A través de la integración de sensores de precisión, visualización local mediante **pantalla LCD (entre otros componentes)**, alarmas físicas *in situ* y un **servidor web embebido**, el sistema procesa variables hidráulicas y meteorológicas críticas para anticipar escenarios de desabastecimiento y sequía.

---

## Pilares Tecnológicos del WMS

| Monitoreo Multivariable | Servidor Web Embebido |
| :--- | :--- |
| Medición simultánea de variables físicas del agua (nivel y caudal) integradas con datos meteorológicos locales. | Tablero de control local alojado directamente en el microcontrolador, accesible de forma segura vía WLAN. |
| **Alertas In-Situ** | **Arquitectura Concurrente** |
| Notificaciones físicas inmediatas mediante alarmas sonoras/visuales y despliegue de datos en tiempo real en pantalla LCD. | Medición y procesamiento ejecutados fuera del hilo principal mediante rutinas de interrupción (ISR) o hilos dedicados. |

---

## Centro de Control de la Wiki (Navegación)

*Utilice este panel para explorar detalladamente cada una de las etapas de diseño, desarrollo y validación de nuestro prototipo:*

| Sección | Módulo de Documentación | Descripción Clave |
| :---: | :--- | :--- |
| **01** | **[Contexto del Reto y Requisitos](1-Contexto-del-reto-y-requisitos)** | Análisis de la problemática hídrica y desglose de requerimientos. |
| **02** | **[Equipo, Roles y Contribuciones](2-Equipo-roles-y-contribuciones)** | Estructura del equipo de desarrollo y matriz de responsabilidades. |
| **03** | **[Diseño de la Solución](3-Diseño-de-la-solución)** | Arquitectura general del sistema, restricciones y modelo de negocio. |
| **04** | **[Diseño e Implementación de Hardware](4-Diseño-e-implementación-de-hardware)** | Esquemáticos, sensores, actuadores y estándares de ingeniería aplicados. |
| **05** | **[Diseño e Implementación de Software](5-Diseño-e-implementación-de-software)** | Diagramas UML, lógica de fusión de sensores y desarrollo de hilos/ISR. |
| **06** | **[Configuración Experimental y Validación](6-Configuración-experimental-y-validación)** | Protocolo de pruebas técnicas y autoevaluación del sistema. |
| **07** | **[Resultados y Análisis](7-Resultados-y-análisis)** | Datos obtenidos, comportamiento del prototipo y video demostrativo. |
| **08** | **[Retos Presentados y Conclusiones](8-Retos-presentados-y-conclusiones)** | Lecciones aprendidas durante el desarrollo y trabajo futuro. |
| **09** | **[Uso de Inteligencia Artificial](9-Uso-de-inteligencia-artificial)** | Declaración obligatoria de herramientas de IA, prompts y validación. |
| **10** | **[Referencias y Anexos](10-Referencias-y-anexos)** | Código fuente documentado, enlaces de interés y bibliografía técnica. |

---

## Demostración del Prototipo (Video de 5 Minutos)

> [!TIP]
> **¿Desea ver el WMS en acción?**
> En la sección de **[Resultados y Análisis](7-Resultados-y-análisis)** podrá acceder al video explicativo y demostrativo del funcionamiento del prototipo en tiempo real.
> 
> *[Acceder al video demostrativo](7-Resultados-y-análisis)*

---
<p align="center"><i>Universidad de la Sabana - Facultad de Ingeniería - Internet de las Cosas (2026-2)</i></p>

## Preguntas para completar la presentación del equipo

- ¿Cómo identificamos esta versión del prototipo y qué evidencia muestra su funcionamiento?
- ¿Qué variables medimos y dónde las mostramos?
- ¿Qué necesidad concreta de Sabana Centro buscamos atender y qué fuentes la respaldan con citas IEEE?
- ¿Qué hace ahora el ESP32 que no hacía en el primer corte?
- ¿Qué resultado validamos y qué limitaciones conserva nuestro prototipo?
- ¿Cuál es nuestro objetivo y qué pruebas mostrarán que lo cumplimos?
- ¿Cuál es el enlace al repositorio de código y al video de validación?
- ¿Nuestro video dura máximo cinco minutos, se reproduce desde Teams y nos muestra a todos con la cámara encendida?
- ¿La arquitectura que presentamos refleja la decisión final de capturar ECHO con una ISR breve y procesar lo demás en el superloop, sin hilos ni tareas creados por el equipo?
