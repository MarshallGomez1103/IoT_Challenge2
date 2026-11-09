# ACP conjunto de volumen y ambiente

El notebook [01_Fusion_ACP_volumen_ambiente.ipynb](01_Fusion_ACP_volumen_ambiente.ipynb) reconstruye las fuentes, genera una distancia simulada de 5–20 cm a partir del volumen del Neusa y ajusta un ACP conjunto con cuatro variables ambientales. Presenta los coeficientes de PC1 y su descomposición exacta en un bloque hídrico y otro ambiental.

El resultado es un **índice descriptivo con una distancia simulada**. Los porcentajes de los bloques corresponden a una normalización L1 explícita de los coeficientes. No son ponderaciones de riesgo validadas ni sustituyen directamente el 80/20 del firmware.

## Reproducción

Con Python 3.11, desde esta carpeta:

```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
python -m jupyterlab 01_Fusion_ACP_volumen_ambiente.ipynb
```

Seleccionar el kernel del entorno y ejecutar todas las celdas en orden. También se puede utilizar un entorno existente con las mismas bibliotecas. La extracción de tablas de los PDF requiere `pdftotext`, de Poppler, disponible en el `PATH`.

El análisis normal utiliza las fuentes originales incluidas y no requiere Internet. La variable `DESCARGAR_DE_NUEVO`, en la sección 2, permite obtener nuevamente las fuentes desde sus URL públicas. Las descargas van a una carpeta separada y conservan los originales. Los PDF CAR se comparan por SHA-256. NASA incluye una latencia de respuesta variable en `times`: se registra el hash de la descarga y se compara el JSON completo excluyendo únicamente ese campo. Las mediciones y todos los demás metadatos deben coincidir; en caso contrario se detiene la comprobación. El registro distingue una nueva descarga de una copia ya almacenada en caché.

## Datos incluidos

- **CAR:** 31 PDF de agosto de 2024, con 124 registros correspondientes a Neusa, Sisga, Tominé y El Hato.
- **NASA POWER:** JSON original con 1.096 días de 2023–2025 para el punto solicitado cerca del Neusa.
- **CSV derivados:** las tablas previas se contrastan con la reconstrucción desde los originales.
- **Manifiesto:** `datos/fuentes.json` contiene las URL y los hashes. El notebook muestra enlaces descargables a las 32 fuentes originales.

La matriz conjunta contiene las 31 fechas de agosto de 2024 con datos de ambas fuentes. No se imputa volumen para otros meses ni se asigna el clima de Neusa a otros embalses. Las series IDEAM del estudio previo no se mezclan con esta matriz debido a sus diferencias de fuente y a la descripción incompatible de la humedad histórica.

## Resultados

La carpeta `resultados/` contiene las tablas reconstruidas, la distancia simulada, medias y desviaciones, correlaciones, coeficientes, cargas, varianza explicada, reparto L1, sensibilidad temporal y figuras. `parametros_indice_descriptivo.json` reúne los parámetros necesarios para reproducir las puntuaciones.

`01_Fusion_ACP_volumen_ambiente.html` permite consultar el código, las explicaciones y las salidas sin abrir Jupyter. Para modificar o volver a ejecutar el análisis se utiliza el archivo `.ipynb`.

## Interpretación

El volumen máximo observado se representa con 5 cm y el mínimo con 20 cm. Son extremos del mes, no estados físicos de lleno y seco. La distancia es una transformación lineal declarada del volumen, sin una curva nivel–volumen ni geometría del sensor.

La expresión final utiliza una distancia **estandarizada** y un bloque ambiental nuevo que conserva signos negativos. No equivale a combinar dos riesgos positivos de 0–100. Tampoco se valida un umbral de alarma ni se demuestra evaporación. Para esas conclusiones se requieren mediciones locales y eventos de referencia independientes.

El enunciado y la rúbrica del segundo corte delimitan el alcance académico del anexo. Las pruebas físicas, la Wiki, el video y los demás artefactos del reto se evalúan por separado.
