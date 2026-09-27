# Estado de los repositorios y de la Wiki

Registro de consulta realizado antes de la primera publicación del repositorio Challenge 2.

- `IoT_Challenge1` responde como repositorio público. Su README identifica el proyecto como WMS, enlaza el código `ESP32_Sensores/ESP32_Sensores.ino` y enlaza una Wiki. El sketch de GitHub coincide con la copia local examinada.
- `IoT_Challenge2` ya existe, es público, está en `main` y tenía tamaño cero al consultar sus metadatos: aún no había archivos ni commits.
- La Wiki del primer corte sí se clonó en `reference/wiki_corte1/`. Su navegación organiza diez páginas: contexto/requisitos; equipo; diseño; hardware; software; configuración experimental/validación; resultados/análisis; retos/conclusiones; IA; referencias/anexos.
- La consulta de `Home.md` para `IoT_Challenge2.wiki` respondió 404; además, `git clone` del remoto `.wiki.git` devolvió “Repository not found”. El repo 2 estaba vacío al consultar. No se inicializó ni publicó una Wiki nueva.
- La plantilla de `wiki-template/` sigue la secuencia de diez páginas del primer Wiki y añade preguntas para tablero, WLAN, historial, notificaciones, fusión y medición. No copia la redacción del Wiki anterior.
- El primer intento de `git ls-remote` no resolvió DNS; el clon del primer Wiki se completó después con acceso ampliado de solo lectura.
- En el momento de esta consulta todavía no se había hecho ningún commit ni push. Este archivo conserva ese estado histórico; la publicación inicial se realiza después, el 27 de septiembre de 2026.
