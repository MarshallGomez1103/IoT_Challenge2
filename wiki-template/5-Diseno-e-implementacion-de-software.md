# 5. Diseño e implementación de software

## 5.1. Descripción general

- ¿Qué bibliotecas, versión del core ESP32 y herramientas utilizaron?
- ¿Cómo funciona el superloop y qué atiende en cada pasada?

## 5.2. Módulos de software

- ¿Qué entradas, salidas y responsabilidad tiene cada módulo?
- ¿Cómo documentaron cada módulo en el código?

## 5.3. Flujo general del programa

- Insertar diagrama de adquisición, fusión, alarma y tablero.
- ¿Qué pasa cuando falla un sensor o se pierde la WLAN?

## 5.4. Modelo UML conceptual de módulos

- Insertar el UML que corresponde a la arquitectura realmente implementada.
- ¿Qué interfaces y mensajes representa?

## 5.5. Procesamiento de las variables

- ¿Cómo calculan nivel, índice ambiental, riesgo fusionado e historial?
- ¿Qué filtro aplican y qué muestran cuando una lectura no es válida?
- ¿Por qué el fotoresistor es luz relativa y no radiación solar calibrada?

## 5.6. Interfaz local

- ¿Cómo conecta el ESP32 a la WLAN de la zona y cómo limitan el acceso?
- ¿Qué dirección local se abre desde PC/celular conectado a esa WLAN?
- ¿Qué valores e historial presenta el tablero? ¿Cómo se silencia el buzzer?
- ¿Qué diferencia hay entre el AP de demostración y la WLAN de entrega?

## 5.7. Lógica de fusión

- ¿Cómo normalizan el riesgo de nivel y el índice ambiental?
- ¿Cuál es la ecuación final y qué escenarios justifican sus pesos?
- ¿Qué estados y umbrales usan? ¿Cómo funciona la histéresis?
- ¿Qué alerta presenta el sistema cuando no puede confiar en una lectura?

## 5.8. Código fuente

- ¿Dónde está el sketch final, cómo se configura WLAN sin exponer credenciales y qué dependencias requiere?
- ¿Qué evidencia muestra que el firmware cargado coincide con esta versión?
