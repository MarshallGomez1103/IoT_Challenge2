# 4. Diseño e implementación de hardware

## 4.1. Componentes del prototipo

- ¿Qué modelo de ESP32, sensores, pantalla, alarma, fuente y materiales instalaron?
- ¿Qué componentes son prestados y cómo se identificaron?

## 4.2. Conexiones principales

- Completar tabla de pin, señal, componente, alimentación y observación.
- ¿Cómo protegieron GPIO19 frente a ECHO del HC-SR04, que puede entregar 5 V?
- ¿Qué pines I²C usan LCD y BMP180 y cómo comparten el bus?

## 4.3. Arquitectura de conexiones

- Insertar esquemático legible con todas las interconexiones, alimentación y tierra común.
- ¿Cómo se conectan RGB, buzzer, botón, DHT11 y fotoresistor?

## 4.4. Medición de nivel de agua

- ¿Cuál es la distancia medida con el recipiente lleno y en el nivel crítico?
- ¿Cómo convirtieron la distancia en nivel o riesgo y qué incertidumbre observaron?
- ¿Qué registra la ISR en los flancos de ECHO y qué calcula luego el superloop?

## 4.5. Alimentación y rearmado

- ¿Cómo alimentaron el montaje y qué consumo midieron?
- ¿Qué piezas tuvieron que rearmar después del primer corte y qué problemas encontraron?
