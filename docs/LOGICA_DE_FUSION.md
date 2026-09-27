# Auditoría y calibración de la lógica de riesgo

## Hallazgo: qué respalda hoy el 80/20

En el firmware de segundo corte aparecen `PESO_RIESGO_NIVEL = 0.80` y `PESO_RIESGO_AMBIENTAL = 0.20`. La ecuación ejecutada es:

`R_fusión = 0.80 × R_nivel + 0.20 × I_ambiental`

El código por sí solo no contiene un cálculo estadístico que produzca esos dos coeficientes. La proporción expresa una prioridad de diseño razonable para el prototipo —dar cuatro veces más influencia a la medición directa del nivel que al proxy ambiental—, pero el 80/20 todavía es una elección provisional. En los archivos revisados no aparece una serie histórica emparejada de nivel y ambiente, ni una hoja de cálculo, script o informe que estime o valide esa proporción. Por eso no debemos sustentarla diciendo que salió de veinte años de datos hasta que encontremos y podamos reproducir esos datos y ese cálculo.

El sketch de referencia del primer corte sí contiene un comentario que atribuye los pesos internos del índice ambiental a una serie NASA POWER 2000–2025 y a un análisis de sensibilidad inspirado en FAO-56. También consigna rangos Q25–Q75 para temperatura, humedad y presión. Sin embargo, en la carpeta de trabajo no están la descarga de la serie, la consulta exacta, el cálculo de sensibilidad ni una tabla de resultados. Esa nota deja una pista para investigar; no basta para afirmar que los valores se obtuvieron de forma reproducible.

Además, el historial del firmware actual guarda 60 muestras en RAM cada 2 segundos: aproximadamente 120 segundos. Ese historial reciente no es una fuente de veinte años.

### Qué aporta el artículo de monitoreo de ríos

El artículo que compartió el equipo describe un patrón útil: montar el ultrasonido a una altura de referencia fija, interpretar las variaciones de distancia como cambios del nivel y generar avisos por un umbral o una tendencia significativa. También propone cruzar los registros con datos meteorológicos para estudiar correlaciones. **No propone un porcentaje 80/20 ni publica coeficientes de fusión.** El ejemplo usa plataformas comerciales como Libelium/MaxBotix y Milesight con redes de campo y nube; sus alcances, precisión y frecuencia no se pueden atribuir al HC-SR04 conectado a nuestro ESP32. En un río que sube, la distancia sensor-agua disminuye; en nuestro recipiente, el riesgo de quedarse sin agua crece cuando esa distancia aumenta. La variable física es la misma y cambia el sentido de la alerta según el caso [7].

La guía de la Organización Meteorológica Mundial respalda usar umbrales de nivel y de velocidad de cambio, pero dice que deben estudiarse para el sitio y sus consecuencias; no deben copiarse como números universales. Su ejemplo de 25 cm/h corresponde a un río y **no es un valor para nuestro tanque** [8]. Aplicado al prototipo: calibrar el nivel mínimo según el recipiente y fijar la anticipación según el tiempo que realmente necesita el equipo para actuar. La guía británica para instalaciones con ultrasonido también enfatiza montaje, verificación en sitio y compensación de la velocidad del sonido con la temperatura [9].

Por tanto, el artículo sirve como referencia de arquitectura y operación, no como evidencia matemática de los pesos. No se encontró una regla técnica general que indique que todo monitor de agua deba pesar 80 % el nivel y 20 % el ambiente.

## Qué representan todos los pesos del código

El firmware contiene dos niveles de ponderación. El 80/20 divide el riesgo entre nivel y ambiente. Dentro del índice ambiental, los cuatro pesos suman 1.0000:

`I_ambiental = 0.7474 L + 0.1479 T + 0.1026 S + 0.0021 P`

`L`, `T`, `S` y `P` son factores normalizados entre 0 y 1: luz relativa, temperatura, aire seco y presión baja. Al sustituir esta ecuación en la fusión, las contribuciones máximas al puntaje total quedan así:

| Factor | Peso dentro del índice ambiental | Aporte máximo al riesgo fusionado |
|---|---:|---:|
| Riesgo por nivel | — | 80.000 puntos |
| Luz relativa del LDR | 74.74 % | 14.948 puntos |
| Temperatura | 14.79 % | 2.958 puntos |
| Aire seco, derivado de humedad relativa | 10.26 % | 2.052 puntos |
| Presión baja | 0.21 % | 0.042 puntos |
| **Total** | **100 % del componente ambiental** | **100 puntos** |

Así se explica “qué tiene que ver el resto”: temperatura, humedad, presión y luz componen el 20 % ambiental; no son porcentajes adicionales al 80/20. Con los coeficientes actuales, la presión tiene una contribución casi nula al resultado.

El código normaliza la temperatura entre 12.38 y 13.43 °C, la sequedad usando 83.89 y 87.34 % de humedad relativa, y la presión baja usando 758.8 y 759.8 hPa. Los valores fuera de cada intervalo se limitan a 0 o 1. La luz se representa con una conversión del ADC del LDR entre 3500 (oscuro) y 600 (claro). Es un porcentaje relativo del montaje, no una medida de irradiancia en W/m² ni un porcentaje de agua.

### Ejemplo numérico: de dónde salen 57.47 y 2.958 puntos

Este ejemplo reproduce la aritmética actual para explicar el efecto de cada coeficiente. Los valores de temperatura, humedad y presión son los puntos medios de los rangos configurados; son datos ilustrativos, **no una lectura real ni un resultado de calibración**. Tomamos luz relativa de 60 % y distancia de 12.5 cm:

| Entrada | Operación de normalización | Factor |
|---|---|---:|
| Temperatura `T = 12.905 °C` | `(12.905 − 12.38) / (13.43 − 12.38)` | 0.5 |
| Humedad `RH = 85.615 %` | `(87.34 − 85.615) / (87.34 − 83.89)` | 0.5 |
| Presión `P = 759.3 hPa` | `(759.8 − 759.3) / (759.8 − 758.8)` | 0.5 |
| Luz relativa | `60 / 100` | 0.6 |
| Riesgo por distancia `d = 12.5 cm` | `100 × (12.5 − 5) / (15 − 5)` | 75 puntos |

Sustituyendo los cuatro factores en el índice ambiental:

```text
I_ambiental = 100 × (0.7474×0.6 + 0.1479×0.5 + 0.1026×0.5 + 0.0021×0.5)
            = 100 × (0.44844 + 0.07395 + 0.05130 + 0.00105)
            = 57.474 puntos
```

Luego, la fusión que hoy ejecuta el sketch da:

```text
R_fusión = 0.80×75 + 0.20×57.474
         = 60 + 11.4948
         = 71.4948 puntos ≈ 71.5
```

La temperatura en este ejemplo aporta `100 × 0.20 × 0.1479 × 0.5 = 1.479` puntos al riesgo final. Su **máximo matemático** es `100 × 0.20 × 0.1479 × 1 = 2.958` puntos. Así se explica el “2.9”: es el producto de tres coeficientes configurados y la escala 0–100; no significa que una investigación haya medido que la temperatura causa exactamente 2.9 % del riesgo. Análogamente, 57.474 es la suma ponderada del ejemplo ambiental completo, no una contribución solo de temperatura. Un valor mostrado cerca de 58 no valida los pesos.

En el mismo ejemplo, los aportes finales son 60 puntos por nivel, 8.9688 por luz, 1.479 por temperatura, 1.026 por aire seco y 0.021 por presión. El cálculo es reproducible; la selección de los coeficientes aún no lo es.

## Qué hace hoy y qué no hace

La distancia `d` es del sensor a la superficie. Cuando el nivel baja, `d` aumenta. Con las referencias actuales de 5 y 15 cm:

`R_nivel = limitar(100 × (d − 5) / (15 − 5), 0, 100)`

A 7.5 cm, el riesgo por distancia es 25 %. El firmware actualizado estima la pendiente lineal de las muestras recientes: toma hasta 30 s del tramo continuo más reciente, con al menos 8 muestras y al menos 14 s de separación. La pendiente positiva significa que la distancia crece y el agua baja; una pendiente negativa significa que la distancia disminuye y el nivel sube. Antes de tener suficientes puntos, no se emite una predicción por velocidad.

El índice ambiental puede elevar el resultado, pero no sustituye a la pendiente del nivel. Por ejemplo, con `R_nivel = 25` e índice ambiental de 100, la fusión da 40; con índice ambiental de 0, da 20. El firmware marca preventivo desde 40 y crítico desde 70, con salidas de histéresis en 35 y 65. El buzzer se activa solo en crítico. Si falta una lectura ambiental necesaria y no hay alarma directa o anticipada de nivel, el firmware informa `FALLA` y no calcula una fusión normal. Si el agua ya alcanzó el límite o la proyección está dentro del horizonte configurado, conserva el estado crítico aunque falte el ambiente.

## Qué permite concluir la literatura

La evaporación de una superficie de agua abierta se relaciona con el balance de energía y el transporte de vapor. FAO explica que el método de Penman combina ambos y que los cálculos meteorológicos requieren, entre otros datos, temperatura, humedad, radiación y viento. El sensor actual mide temperatura y humedad con DHT11 y luz relativa con LDR, pero no mide viento ni radiación calibrada. La formulación FAO-56 de evapotranspiración de referencia también tiene un propósito distinto a medir directamente la pérdida de agua de este recipiente. Por eso el índice del sketch debe llamarse **índice ambiental relativo**, no tasa medida de evaporación.

Un estudio de USGS sobre un lago estima la pérdida de agua con términos de energía y aerodinámica y contrasta el método con mediciones. Esto respalda medir y validar el sistema, pero no respalda los coeficientes 80/20 ni los cuatro coeficientes concretos de nuestro código. NASA POWER sí ofrece series meteorológicas y solares históricas desde 1981 a escala espacial de rejilla; pueden describir el clima de una zona, pero no contienen el nivel de nuestro recipiente ni sus eventos de llenado y uso. Una serie meteorológica sola no puede descubrir qué proporción predice el agotamiento del tanque.

## Métricas recomendadas para una siguiente versión

La recomendación es **dejar de sumar nivel y ambiente en un porcentaje único**. Para este recipiente, la medición primaria debe ser el agua disponible y su tendencia. Temperatura, humedad, luz y presión deben aportar una interpretación ambiental explícita, sin aparentar una precisión que el montaje aún no tiene. Esta sección es una propuesta de diseño documentada; **no describe un cambio ya hecho en el sketch**.

### 1. Nivel disponible y riesgo de nivel

Medir y guardar dos referencias con el recipiente real:

- `d_lleno`: distancia entre el sensor y la superficie cuando el recipiente está en su máximo operativo seguro.
- `d_critico`: distancia cuando se alcanza el mínimo de agua aceptable para el uso previsto.

Con `d_actual` como la distancia válida entre el sensor y el agua:

```text
nivel_disponible_pct = limitar(100 × (d_critico − d_actual) / (d_critico − d_lleno), 0, 100)
riesgo_nivel_pct     = 100 − nivel_disponible_pct
```

Ejemplo con referencias provisionales `d_lleno = 5 cm`, `d_critico = 15 cm` y `d_actual = 12.5 cm`:

```text
nivel_disponible = 100 × (15 − 12.5) / (15 − 5) = 25 %
riesgo_nivel     = 100 − 25 = 75 %
```

Es la misma recta que ya usa el código para `riesgoNivelPct`, expresada además como porcentaje intuitivo de disponibilidad. Los 5 y 15 cm solo sirven para explicar la operación hasta medir los límites reales del recipiente.

### 2. Tendencia de descenso y tiempo hasta el mínimo

Calcular `v_d` como pendiente de distancia respecto al tiempo, usando varias lecturas válidas de una ventana breve. Mantener cm/min como unidad:

- `v_d > 0`: la distancia aumenta; el agua está bajando.
- `v_d < 0`: la distancia disminuye; el agua está subiendo.
- `v_d ≈ 0`: no hay descenso apreciable dentro de la resolución del montaje.

Si `v_d > 0`, calcular `ETA_min = (d_critico − d_actual) / v_d`. Por ejemplo, si `d_actual = 7.5 cm`, `d_critico = 15 cm` y `v_d = 1.5 cm/min`, entonces `ETA = (15 − 7.5) / 1.5 = 5 min`. La pendiente es una predicción local; se invalida si faltan lecturas continuas o si el recipiente se llena/extrae manualmente durante la ventana.

La única alarma física puede activarse si se cumple cualquiera de estas condiciones:

```text
alarma = (d_actual >= d_critico)
      OR (v_d > 0 AND ETA_min <= tiempo_de_respuesta)
```

`tiempo_de_respuesta` se define según cuánto tarda el equipo en rellenar o atender el recipiente. Cinco minutos puede usarse como ensayo inicial, pero no se presenta como valor científico hasta medirlo. Se recomienda una histéresis: apagar la alarma cuando el nivel se recupere con margen y el ETA quede por encima de un horizonte de salida mayor que el de entrada. Lectura de distancia inválida significa **falla de medición**, nunca cero riesgo. El equipo conserva una sola alarma; nivel inmediato y velocidad son dos causas posibles de activación.

### 3. Cómo sí aportarían las variables ambientales

**Temperatura.** Tiene una relación física directa con el tiempo de vuelo ultrasónico: modifica la velocidad del sonido en el aire. Como mejora posterior, puede usarse para corregir la distancia, siempre que la temperatura medida represente el aire del trayecto acústico. Una aproximación publicada para el aire es `c = 331.3 + 0.606×T` m/s; entonces `d_cm = t_echo_us × c / 20000` [10]. Ejemplo a 25 °C y `t_echo = 1000 μs`: `c = 346.45 m/s`; la corrección da `d = 1000×346.45/20000 = 17.3225 cm`. El factor fijo actual da `1000×0.0343/2 = 17.15 cm`; la diferencia de este ejemplo es 0.1725 cm. El HC-SR04 económico, la ubicación del DHT11 y la corta distancia del recipiente limitan cuánto mejoraría; hay que comparar contra una regla antes de implementarlo. La corrección **no está en el firmware actual**.

**Temperatura y humedad.** Juntas permiten calcular el déficit de presión de vapor (VPD), una medida de cuán lejos está el aire de saturarse, no una medición directa del agua perdida:

```text
e_s = 0.6108 × exp(17.27×T / (T + 237.3))  kPa
VPD = e_s × (1 − RH/100)                  kPa
```

Ejemplo ilustrativo para `T = 28 °C`, `RH = 60 %`: `e_s = 3.7799 kPa`; `e_a = 3.7799×0.60 = 2.2679 kPa`; por tanto `VPD = 3.7799 − 2.2679 = 1.5120 kPa`. Puede presentarse como “demanda evaporativa del aire” y ayudar a contextualizar una caída gradual, pero no se debe sumar como puntos de alarma ni llamar evaporación medida [11].

**Luz relativa del LDR.** Puede acompañar el VPD como contexto de exposición diurna, después de calibrar el montaje. El porcentaje del firmware solo ubica la lectura ADC entre dos extremos; no es radiación solar en W/m². Sin un sensor radiométrico o una calibración contra uno, no sirve para calcular energía disponible en una ecuación de evaporación.

**Presión atmosférica del BMP180.** Conviene mostrarla y guardar su tendencia como contexto meteorológico. Por sí sola no indica si el tanque se vacía. La formulación FAO Penman-Monteith combina temperatura, humedad, radiación y viento, además de términos de energía y transporte de vapor; la estación actual no mide viento ni radiación calibrada, y tampoco caracteriza temperatura de la superficie del agua. Por eso no es honesto presentar el índice existente como evaporación real. La presión puede participar en parámetros psicrométricos dentro de un método completo, pero no justifica el peso `0.0021` por sí mismo [1], [2], [11].

En el tablero, la versión recomendada mostraría **nivel disponible**, **riesgo de nivel**, **descenso en cm/min**, **ETA al mínimo**, **VPD**, **luz relativa**, **presión** y estado de cada sensor. La alarma se decide con el nivel y su tendencia. Las señales ambientales explican el contexto y quedan listas para una futura calibración, sin elevar artificialmente el riesgo.

### Qué debe cambiar en la implementación, cuando el equipo lo autorice

El sketch de hoy todavía calcula `0.80×R_nivel + 0.20×I_ambiental` para `fusion_pct`; esta revisión documental no lo modificó. Si luego se implementa la recomendación, conviene reemplazar el “riesgo fusionado” por métricas con unidades y nombres separados, y conservar un único estado/alarma. Primero se miden `d_lleno`, `d_critico` y el tiempo de respuesta; después se comprueba la distancia contra una referencia manual a varios niveles y se hacen ensayos de descenso, llenado y desconexión. Se registran ambiente y acciones manuales para distinguir evaporación, extracción y recarga. Solo si los datos muestran que una variable ambiental mejora la detección se añade al modelo; el criterio y el conjunto de evaluación se documentan antes de escoger coeficientes.

## Si se quiere probar una fusión ponderada con datos del tanque

La recomendación principal sigue siendo alarmar por nivel calibrado y tendencia, manteniendo las variables ambientales como contexto. Este proceso es un experimento opcional para comprobar, con datos reales, si el ambiente mejora las predicciones lo suficiente como para justificar algún peso. No ejecuté una estimación porque el proyecto no contiene observaciones reales emparejadas de distancia, ambiente y eventos de nivel crítico. Inventar resultados o generar registros sintéticos y presentarlos como históricos sería engañoso. La plantilla de columnas está en [`plantilla_registro_calibracion.csv`](plantilla_registro_calibracion.csv); dejé un registrador local y un calibrador reproducible en `tools/` para usar cuando el equipo tenga lecturas del montaje.

El equipo puede completar el cálculo local así:

1. **Calibrar el recipiente:** medir varias veces la distancia con el recipiente en el nivel operativo lleno y en el mínimo aceptable. Guardar esos valores como `d_operativo` y `d_critico`; los 5 y 15 cm del sketch son provisionales.
2. **Registrar datos sincronizados:** con el PC conectado a la misma red del ESP32, iniciar el registrador que consulta `/api/status` cada dos segundos. Guarda fecha/hora UTC, distancia, temperatura, humedad, presión, luz relativa y banderas de validez. No mezclar periodos con sensores desconectados como si fueran ceros.

   ```bash
   python3 tools/registrar_muestras.py --url http://192.168.4.1/api/status --interval 2 --output data/prueba_01.csv
   ```

   En modo estación, sustituir `192.168.4.1` por la IP del ESP32. Detener con Ctrl+C. Completar en la columna de observaciones si se llenó o extrajo agua; decidir y documentar cómo tratar esos periodos antes de ajustar.
3. **Definir el resultado que queremos anticipar:** por ejemplo, `evento=1` si durante los siguientes cinco minutos la distancia alcanza `d_critico`, y `evento=0` si no. El horizonte debe corresponder al tiempo real que necesita la persona para actuar; cinco minutos aquí es un ejemplo, no un valor medido.
4. **Construir los factores de entrada:** calcular `R_nivel` con las distancias calibradas y conservar `I_ambiental` como el valor relativo que genera el firmware. Revisar antes las unidades y las señales inválidas.
5. **Buscar el coeficiente con observaciones anteriores:** el calibrador etiqueta `evento=1` si en los siguientes cinco minutos una lectura continua y válida llega a `d_critico`; si no llega, la etiqueta es 0. Prueba `alpha` de 0 a 1 en pasos de 0.01 en `R = alpha × R_nivel + (1 − alpha) × I_ambiental`, usando el umbral crítico que se le indique (por defecto 70). Selecciona por exactitud balanceada en el tramo inicial; si empatan, prefiere el mayor `alpha` para dar prioridad a la medición directa. No escoge el coeficiente mirando el resultado final.
6. **Separar el tiempo, no barajar filas:** el script ajusta con el tramo cronológico inicial (70 % por defecto) y evalúa una vez en el tramo final. Excluye ejemplos cuyo horizonte cruza el corte y deja una brecha temporal de un horizonte entre ajuste y prueba. Reporta positivos/negativos, falsos negativos, falsos positivos, sensibilidad, especificidad y exactitud balanceada. Requiere que haya eventos positivos y negativos en ambos tramos; si no, pide más datos y no ofrece coeficientes.

   ```bash
   python3 tools/calibrar_peso_fusion.py data/prueba_01.csv --d-operativo 5 --d-critico 15 --horizonte-min 5 --umbral-riesgo 70
   ```
7. **Aceptar el peso solo si se repite:** si un conjunto posterior no confirma el resultado, mantener el valor como heurístico o cambiar el modelo. Una serie dominada por acciones manuales de llenado/extracción no puede atribuir esos cambios al clima; esos eventos deben registrarse o excluirse con una regla acordada.

La guía de NIST sobre mínimos cuadrados ponderados aclara que sus pesos describen la precisión relativa de las observaciones; eso no es lo mismo que un coeficiente de importancia 80/20 entre variables. Para series temporales, `TimeSeriesSplit` documenta que se conserva el orden temporal y que barajar no corresponde a la evaluación hacia el futuro. El script propuesto hace una búsqueda temporal sencilla, no un modelo sofisticado. En esta etapa, la afirmación honesta es: **80/20 es una proporción provisional elegida para priorizar nivel directo; todavía no está estimada ni validada con datos del prototipo**.

## Una sola alarma que considera nivel y velocidad

Para detectar que el agua **está bajando**, se estima la pendiente de varias distancias válidas recientes, en vez de comparar una lectura con la anterior:

- `v_d = pendiente de distancia en cm/min` sobre una ventana corta y suavizada, por ejemplo 20–30 s.
- `v_d > 0`: la distancia crece y el nivel está bajando.
- `v_d < 0`: la distancia cae y el nivel está subiendo.
- Si `v_d > 0`, estimar `t_crit = (d_critico − d_actual) / v_d` en minutos.

El firmware deja una sola salida crítica para la alarma: la activa si la distancia ya llegó al límite (`d_actual ≥ d_critico`), si el puntaje fusionado entró en crítico, o si `t_crit` queda dentro del horizonte de anticipación. El horizonte de entrada está configurado provisionalmente en 5 min; mientras la alarma ya está activa se conserva hasta que la proyección salga de 7.5 min. Por ejemplo, con `d_actual = 7.5 cm`, `d_critico = 15 cm` y una subida de distancia de `1.5 cm/min`, se estima que faltan 5 minutos para llegar al límite. Si la distancia disminuye, el nivel está recuperándose y no se predice una caída a partir de esa pendiente.

El horizonte de anticipación está en el firmware como parámetro de partida, no como valor “científico”: debe corresponder al tiempo de respuesta del equipo y validarse con mediciones. El filtro usa una regresión lineal sobre la ventana reciente, no una diferencia entre dos ecos consecutivos. La pérdida de señal se presenta como **falla de medición**, nunca como nivel normal. Si se quiere que la velocidad también participe en el porcentaje mostrado, hay que normalizarla e incluirla en el ajuste de pesos con el mismo procedimiento; no conviene añadirle otro coeficiente escogido a ojo.

## Referencias

[1] R. G. Allen, L. S. Pereira, D. Raes y M. Smith, *Crop Evapotranspiration: Guidelines for Computing Crop Water Requirements*, FAO Irrigation and Drainage Paper 56, cap. 2. [En línea]. Disponible: https://www.fao.org/4/x0490e/x0490e06.htm

[2] U.S. Geological Survey, *Evaporation from the interior of Lake Okeechobee—A large freshwater lake in Florida, 2013–16*, Scientific Investigations Report 2024–5040. [En línea]. Disponible: https://pubs.usgs.gov/publication/sir20245040/full

[3] NASA Langley Research Center, *POWER Daily API Documentation*. [En línea]. Disponible: https://power.larc.nasa.gov/docs/services/api/temporal/daily/

[4] National Institute of Standards and Technology, *Weighted Least Squares Regression*. [En línea]. Disponible: https://itl.nist.gov/div898/handbook/pmd/section4/pmd432.htm

[5] scikit-learn developers, *TimeSeriesSplit documentation*. [En línea]. Disponible: https://scikit-learn.org/stable/modules/generated/sklearn.model_selection.TimeSeriesSplit.html

[6] Firmware local, `firmware/ESP32_WMS_Challenge2/ESP32_WMS_Challenge2.ino`, constantes `PESO_RIESGO_*`, `PESO_*`, `calcularIndiceAmbiental()`, `calcularRiesgoNivel()` y `actualizarFusion()`; sketch de referencia `reference/corte1/ESP32_Sensores.ino`, comentario sobre NASA POWER y FAO-56.

[7] Manx Technology Group, “River Level Monitoring with IoT & Ultrasonic Sensors,” 9 feb. 2026. [En línea]. Disponible: https://manxtechgroup.com/iot-ultrasonic-sensors-revolutionising-river-level-monitoring/ (consultado: 27 sep. 2026).

[8] World Meteorological Organization, *Manual on Flood Forecasting and Warning*, WMO-No. 1072, 2011, sec. 8.4. La guía pide relacionar los activadores de nivel y velocidad de cambio con las condiciones y consecuencias locales. [En línea]. Disponible: https://old.wmo.int/extranet/pages/prog/hwrp/publications/flood_forecasting_warning/WMO%201072_en.pdf

[9] Environment Agency, “MCERTS: requirements for installing and using event duration monitors,” sec. 5, guía de instalación y compensación térmica para sensores ultrasónicos de nivel. [En línea]. Disponible: https://www.gov.uk/government/publications/mcerts-requirements-for-installing-and-using-event-duration-monitors/mcerts-requirements-for-installing-and-using-event-duration-monitors

[10] MaxBotix Inc., “How Noise and Temperature Can Affect Sensor Operation,” explicación de la compensación de distancia por velocidad del sonido y temperatura. [En línea]. Disponible: https://maxbotix.com/blogs/blog/noise-temperature-sensor-operation

[11] R. G. Allen, L. S. Pereira, D. Raes y M. Smith, *Crop Evapotranspiration: Guidelines for Computing Crop Water Requirements*, FAO Irrigation and Drainage Paper 56, cap. 3, cálculo de presión de vapor y VPD; cap. 2, variables requeridas por Penman-Monteith. [En línea]. Disponible: https://www.fao.org/4/x0490e/x0490e07.htm y https://www.fao.org/4/x0490e/x0490e06.htm
