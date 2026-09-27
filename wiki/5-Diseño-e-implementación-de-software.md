# 5. Diseño e implementación de software

## 5.1. Descripción general

- ¿Qué bibliotecas, versión del core ESP32 y herramientas utilizamos?
- ¿Cómo funciona nuestro superloop y qué atiende en cada pasada?

## 5.2. Módulos de software

- ¿Qué entradas, salidas y responsabilidad tiene cada módulo que implementamos?
- ¿Cómo documentamos cada módulo en el código?

## 5.3. Flujo general del programa

- ¿Qué diagrama incluiremos para mostrar adquisición, fusión, alarma y tablero?
- ¿Qué hacemos cuando falla un sensor o se pierde la WLAN?

## 5.4. Modelo UML conceptual de módulos

- ¿Qué UML incluiremos para representar la arquitectura que implementamos?
- ¿Qué interfaces y mensajes representa nuestro UML?

## 5.5. Procesamiento de las variables

- ¿Cómo calculamos el nivel, el índice ambiental, el riesgo fusionado y el historial?
- ¿Qué filtro aplicamos y qué mostramos cuando una lectura no es válida?
- ¿Por qué tratamos el fotoresistor como una medida de luz relativa y no como radiación solar calibrada?

## 5.6. Interfaz local

- ¿Cómo conectamos el ESP32 a la WLAN de la zona y cómo limitamos el acceso?
- ¿Qué dirección local abrimos desde el PC o celular conectado a esa WLAN?
- ¿Qué valores e historial presenta nuestro tablero y cómo silenciamos el buzzer?
- ¿Qué diferencia hay entre el AP de demostración y la WLAN que usamos en la entrega?

## 5.7. Lógica de fusión

- ¿Cómo normalizamos el riesgo de nivel y el índice ambiental?
- ¿Cuál es nuestra ecuación final y qué escenarios justifican los pesos?
- ¿Qué estados y umbrales usamos y cómo funciona la histéresis?
- ¿Qué alerta presenta nuestro sistema cuando no puede confiar en una lectura?

## 5.8. Código fuente

- ¿Dónde está nuestro sketch final, cómo configuramos la WLAN sin exponer credenciales y qué dependencias requiere?
- ¿Qué evidencia muestra que el firmware cargado en el ESP32 coincide con esta versión?

### Estado implementado en el sketch revisado

El sketch actual todavía calcula `R_fusión = 0.80 × R_nivel + 0.20 × I_ambiental`. Dentro del índice ambiental, luz relativa, temperatura, aire seco y presión baja usan `0.7474`, `0.1479`, `0.1026` y `0.0021`. El ejemplo de sustitución que explica los 57.474 puntos ambientales y los 2.958 puntos máximos de temperatura está en [`docs/LOGICA_DE_FUSION.md`](../docs/LOGICA_DE_FUSION.md).

No encontramos la serie emparejada, consulta, hoja de cálculo o análisis que reproduzca el 80/20. El comentario heredado del primer corte atribuye los pesos ambientales a NASA POWER y sensibilidad FAO-56, pero la evidencia del procedimiento falta. Por eso estos valores se describen como provisionales, no como resultado validado de veinte años de datos.

### Propuesta documentada para una siguiente versión

La propuesta es dejar de mezclar variables con funciones distintas en un único porcentaje. Mostrar `nivel_disponible_pct = 100 × (d_critico − d_actual) / (d_critico − d_lleno)` y `riesgo_nivel_pct = 100 − nivel_disponible_pct`; estimar por separado la velocidad de descenso en cm/min y el tiempo hasta el nivel crítico. La única alarma se activa si el límite ya se alcanzó o si la pendiente positiva proyecta que se alcanzará antes del tiempo de respuesta acordado. Es consistente con el uso de umbrales y tendencias del artículo [9] y con la recomendación de fijar activadores a partir de las condiciones locales [10]. Los valores de 5 y 15 cm y el horizonte de 5 minutos siguen siendo ejemplos hasta calibrar el recipiente y medir el tiempo de acción.

También proponemos usar temperatura para compensar la velocidad del sonido en la medición ultrasónica [11], [12] y combinar temperatura/humedad para calcular VPD como contexto ambiental [1], [13]. Luz del LDR se mantiene como lectura relativa; presión se registra como contexto meteorológico. Estas métricas propuestas aún no están implementadas en el sketch. Los cálculos, supuestos y referencias están en [`docs/LOGICA_DE_FUSION.md`](../docs/LOGICA_DE_FUSION.md); el artículo de monitoreo de ríos respalda el uso de distancia, referencia local y tendencia, pero no entrega los coeficientes del proyecto.
