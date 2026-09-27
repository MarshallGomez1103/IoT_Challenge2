# Propuesta futura: pérdida observada del nivel de agua

**Estado:** propuesta para discutir con el grupo. No está implementada en el firmware.

## Qué mediría

El HC-SR04 mide la distancia `d` entre el sensor y la superficie. Si esa distancia aumenta, el nivel del agua bajó. En un recipiente de paredes verticales, cada aumento de `1 cm` equivale a `10 mm` de descenso de la superficie.

Con este montaje se puede estimar la **pérdida aparente observada del nivel**. La lectura por sí sola no demuestra que toda el agua se evaporó: una fuga, extracción, movimiento del recipiente, salpicadura o variación del eco también puede cambiar el nivel.

No se debe presentar el resultado como evaporación potencial. Para este prototipo sería una tasa experimental de pérdida observada y, con una extrapolación sencilla, una proyección de corto plazo.

## Escala de tiempo propuesta para la demostración

La idea del grupo es comprimir el tiempo: **1 minuto real equivale a 1 hora simulada**. Por tanto:

```text
horas_simuladas = minutos_reales_transcurridos
factor_de_escala = 60 horas simuladas / hora real
```

Equivale a 1440 horas simuladas por cada día real. La escala debe aparecer siempre en la interfaz y en la sustentación. Una tasa de `0.25 mm/h simulada` con esta regla no significa que el tanque pierda `0.25 mm` en una hora real; representa el cambio medido durante un minuto real, expresado por hora simulada.

## Operaciones

Para dos lecturas válidas:

```text
descenso_observado_mm = 10 × (distancia_actual_cm − distancia_anterior_cm)
horas_simuladas = minutos_reales_entre_lecturas
tasa_mm_por_hora_simulada = descenso_observado_mm / horas_simuladas
```

Un valor negativo indica que el nivel subió. No se debe convertir automáticamente en evaporación; se muestra como subida o se excluye del tramo si el equipo confirma que rellenó el recipiente.

Para suavizar ruido y proyectar la siguiente hora simulada, se propone ajustar una recta con las últimas lecturas válidas:

```text
d(t) = a + b·t
```

`t` está en horas simuladas, `d` en centímetros y `b` es el cambio de distancia en cm por hora simulada. Con al menos 3 observaciones distintas:

```text
tasa_observada_mm_h_sim = max(0, 10 × b)
distancia_siguiente_h_sim_cm = a + b × (t_último + 1)
proyección_siguiente_h_sim_mm = max(0, 10 × (distancia_siguiente_h_sim_cm − distancia_última_cm))
```

La proyección es lineal y de un solo paso. No usa ponderajes ambientales ni inventa una ecuación de evaporación.

## Ejemplo numérico reproducible

Supongamos estas distancias filtradas, separadas por un minuto real. Cada minuto representa una hora simulada:

| Minuto real | Hora simulada | Distancia al agua |
|---:|---:|---:|
| 0 | 0 | 10.00 cm |
| 1 | 1 | 10.03 cm |
| 2 | 2 | 10.05 cm |

La regresión lineal de estos tres puntos da aproximadamente `b = 0.025 cm/h simulada`.

```text
tasa suavizada = 0.025 cm/h × 10 mm/cm = 0.25 mm/h simulada
distancia estimada en hora simulada 3 = 10.0767 cm
descenso proyectado desde la última lectura = (10.0767 − 10.05) cm × 10 mm/cm
                                            ≈ 0.267 mm en la siguiente hora simulada
```

En la pantalla conviene separar:

- **Pérdida observada reciente:** diferencia de nivel de la última ventana, indicando el minuto real que se usó.
- **Proyección próxima hora simulada:** valor calculado con la tendencia de al menos tres lecturas.
- **Muestras válidas:** por ejemplo, `3/200`.
- **Escala:** `1 min real = 1 h simulada`.

Antes de tener tres observaciones válidas, mostrar `Recolectando lecturas` y no una cifra estimada. Cuando llegue la cuarta observación, compararla con la proyección anterior y recalcular la tendencia con las lecturas disponibles.

## Memoria y frecuencia sugeridas

El firmware actual tiene un historial rápido de **60 muestras cada 2 segundos**, aproximadamente 120 segundos. Eso no equivale a 200 observaciones lentas.

Para esta propuesta se recomienda un buffer circular separado de hasta **200 lecturas válidas**, tomando una lectura de distancia filtrada cada minuto real. Cubriría hasta 200 minutos reales, que bajo la escala propuesta representan 200 horas simuladas. Separarlo del historial rápido evita enviar cientos de registros innecesarios en cada actualización del dashboard.

Cada registro necesita solo el tiempo y la distancia. El tamaño real se debe confirmar con el tipo de datos elegido, el heap disponible y la memoria que usa la respuesta JSON. Si el equipo se reinicia, el buffer en RAM se pierde; no constituye un histórico persistente.

La lectura de eco se sigue capturando con una ISR corta; el filtrado, el registro lento, la regresión y la respuesta web se calculan en el superloop. Ninguna operación de tendencia va dentro de la interrupción.

## Condiciones para que la demostración sea defendible

1. Mantener fijo el sensor y el recipiente, y documentar la altura útil y la forma del recipiente.
2. Tomar varias lecturas de eco por minuto y guardar una estadística estable (por ejemplo, la mediana) como observación de ese minuto.
3. Definir una tolerancia a partir de una prueba con el agua quieta; cambios dentro de ese ruido no se cuentan como descenso.
4. Registrar si se rellenó o extrajo agua, si hubo salpicaduras o si se movió el recipiente. Esos tramos no sirven para atribuir la pérdida a evaporación.
5. Comparar la proyección de la cuarta lectura con el valor que realmente mida el sensor y reportar el error; no validar el modelo únicamente con la misma serie que lo generó.
6. Usar en el dashboard las palabras `pérdida observada` y `proyección`. Solo llamar `evaporación` al experimento si el montaje controla o descarta fugas, extracción y otras causas de cambio.

## Decisiones pendientes del grupo

- ¿El profesor acepta la escala didáctica de un minuto real por una hora simulada si se explica claramente?
- ¿Se usará un recipiente con paredes verticales y qué altura tendrá?
- ¿Se conserva el buffer de 200 muestras por 200 minutos o se elige otra duración/frecuencia?
- ¿Qué estadística por minuto y qué tolerancia salen de la prueba de ruido del sensor real?
- ¿Qué frase mostrará el dashboard cuando haya menos de tres lecturas, un reinicio o un sensor desconectado?
- ¿Se reportará solo la tasa observada o también la proyección de la siguiente hora simulada?
