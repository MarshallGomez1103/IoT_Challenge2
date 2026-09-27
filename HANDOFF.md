# Estado y siguiente paso

## Hecho

- Revisados enunciado, las dos rúbricas, artículo de Terán y Aranda (2017) y sketch del primer corte.
- Revisados los pesos de fusión: el 80/20 no tiene una derivación empírica reproducible en los archivos; se documentó un proceso de calibración local y una plantilla de registro.
- El firmware estima la tendencia de distancia con muestras recientes y anticipa el único estado crítico mediante un horizonte aún provisional.
- Preparados el mapa de cableado para Proteus, la auditoría del sketch, el informe de brechas y las páginas-guion de la Wiki.
- Añadidos un registrador local de `/api/status` y un calibrador cronológico para cuando se reúnan observaciones reales; todavía no hay datos del tanque para estimar los pesos.
- El usuario reporta que cargó el ESP32 y verificó que el servidor web funciona. No se ha hecho una verificación independiente de sensores ni calibración en esta sesión.
- Guardada en `docs/PROPUESTA_EVAPORACION_OBSERVADA.md` una propuesta pendiente: un minuto real por hora simulada, mínimo tres lecturas para proyectar la siguiente hora y buffer lento separado de hasta 200 lecturas. No modifica el firmware actual.
- Consultado el estado de ambos repositorios: el repo 2 estaba vacío en la revisión inicial y el Wiki 2 no tiene `Home.md` accesible. Se clonó el Wiki del corte 1 en `reference/wiki_corte1/` para seguir su navegación de diez páginas.

## Activo

- Confirmar con el profesor si el requisito ISR acepta medir por flancos el HC-SR04 en ISR y leer DHT11/BMP180/LDR en el superloop. No afirmar cumplimiento total hasta recibir esa respuesta.
- Corregir y validar el esquemático de Proteus antes de rearmar: divisor en ECHO, divisor del LDR, resistencias del LED RGB y botón GPIO14 entre el pin y GND.
- Calibrar distancias, luz y nivel del recipiente con los sensores reales; mantener las credenciales WLAN solo en `secrets.h` local.

## Siguiente

1. Revisar/corregir el SVG de Proteus según `docs/ESQUEMA_PROTEUS.md` y compararlo con los módulos reales antes de energizarlos.
2. Completar `docs/plantilla_registro_calibracion.csv` con pruebas reales; calibrar el recipiente y el LDR antes de reemplazar valores provisionales.
3. Validar en hardware cada sensor conectado/desconectado, las alertas, el LCD y la respuesta del dashboard; guardar evidencia.
4. Discutir con el profesor y el grupo la propuesta de pérdida observada antes de modificar el firmware o sus unidades.
5. Completar la Wiki entre los integrantes con las mediciones, actas y esquemático corregido.
6. Confirmar con el profesor horario y duración de sustentación, discrepantes dentro del enunciado.
