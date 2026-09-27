# Cableado de referencia para Proteus

Este mapa coincide con los pines configurados en `firmware/ESP32_WMS_Challenge2/ESP32_WMS_Challenge2.ino`. Es un **esquema propuesto a partir del firmware**, no una certificación del montaje físico: antes de energizarlo hay que cotejar el modelo exacto de cada módulo y sus etiquetas VCC/GND/SDA/SCL/DATA. El dibujo está en [`esquema_wms.svg`](esquema_wms.svg).

## Tabla de pines

| ESP32 Dev Module | Conectar a | Alimentación y nota |
|---|---|---|
| GPIO18 | TRIG del HC-SR04 | La entrada TRIG recibe la salida de 3.3 V del ESP32. |
| GPIO19 | ECHO del HC-SR04 **a través del divisor** | El HC-SR04 se alimenta a 5 V. Nunca lleves su ECHO de 5 V directo al GPIO. |
| 5V/VIN | VCC del HC-SR04 | Usar la salida de 5 V de la placa solo si se alimenta por USB y la placa la expone como 5V/VIN. |
| GPIO27 | DATA del DHT11 | VCC a 3V3 y GND común. Sensor desnudo: pull-up de 10 kΩ entre DATA y 3V3; un módulo puede traerla incorporada. |
| GPIO34 / ADC | Nodo del divisor del LDR | Orientación para la escala del código: 3V3 → resistencia fija 10 kΩ → nodo GPIO34 → LDR → GND. Con más luz, la resistencia del LDR baja y el ADC baja. |
| GPIO21 / SDA | SDA del BMP180 y del LCD I²C | Bus I²C compartido. El firmware inicializa `Wire.begin(21, 22)` y usa LCD `0x27`. |
| GPIO22 / SCL | SCL del BMP180 y del LCD I²C | Revisa que no haya dos módulos que fuercen el bus a niveles incompatibles. |
| 3V3 | VCC del DHT11, BMP180 y lado ESP32 del divisor LDR | Verifica que el módulo BMP180 admita 3.3 V. Los breakout boards varían. |
| GPIO25 | Canal R del LED RGB | El código espera LED de ánodo común. Un resistor de 220–330 Ω en serie con cada canal. |
| GPIO26 | Canal G del LED RGB | Resistor individual de 220–330 Ω. |
| GPIO32 | Canal B del LED RGB | Resistor individual de 220–330 Ω. |
| GPIO33 | SIG/IN del módulo buzzer activo en LOW | En el código `LOW` activa el buzzer. Si es un buzzer desnudo o de corriente alta, usar transistor/driver, no cargar el GPIO directamente. |
| GPIO14 | Un terminal del botón | El otro terminal a GND. Se configura `INPUT_PULLUP`; el botón presionado se lee LOW. |
| GND | Tierra de ESP32 y todos los módulos | Debe existir tierra común, incluida la fuente de 5 V del HC-SR04. |

## Divisor de tensión de ECHO

Usa dos resistencias entre el ECHO de 5 V y GPIO19:

```text
HC-SR04 ECHO ── R1 1 kΩ ──┬── GPIO19 (ESP32)
                          │
                       R2 1.8 kΩ
                          │
                         GND
```

Con 5 V nominales, el nodo entrega `5 × 1.8 / (1 + 1.8) ≈ 3.21 V`. El ESP32 clásico especifica máximo absoluto de 3.6 V; el divisor protege el GPIO de la salida de 5 V habitual del HC-SR04. Comprueba los valores y tolerancias de tus resistencias y la tensión real de alimentación.

## LCD y bus I²C

El sketch usa LCD 16×2 con backpack I²C en la dirección `0x27`, compartido con el BMP180 en GPIO21/22. No conectes a ciegas un backpack de LCD alimentado a 5 V al bus del ESP32: algunos llevan resistencias pull-up de SDA/SCL a VCC. Si el backpack sube el bus a 5 V, inserta un conversor bidireccional de nivel I²C; alternativamente, alimenta el backpack a 3.3 V solo si su variante funciona correctamente con ese voltaje. Mantén la rama del BMP180 en el nivel admitido por su breakout. El pinout eléctrico exacto depende de los módulos que tengan en el laboratorio.

## LED, botón y buzzer

- **LED RGB:** la configuración del sketch declara ánodo común y conmuta las salidas invertidas. Conecta ánodo común a 3V3 y cada cátodo a su GPIO mediante su propio resistor limitador. Verifica el pinout del encapsulado RGB antes de conectarlo.
- **Botón:** GPIO14 a un terminal y GND al otro; no agregues pull-up externo salvo que el montaje lo requiera.
- **Buzzer:** el firmware presupone un módulo activo en LOW. Si el componente disponible tiene lógica activa en HIGH, el tipo de módulo no coincide con la configuración actual. Para un transductor desnudo utiliza una etapa de transistor adecuada a su corriente.

## Prompt listo para Claude/Proteus

> Dibuja un esquemático eléctrico legible, una sola hoja, del WMS basado en ESP32 Dev Module. Usa estos GPIO exactos: HC-SR04 TRIG GPIO18 y ECHO GPIO19 mediante divisor 1 kΩ en serie y 1.8 kΩ desde el nodo a GND; DHT11 DATA GPIO27; LDR en divisor 3V3–10 kΩ–GPIO34/ADC–LDR–GND; bus I²C LCD 16×2 dirección 0x27 y BMP180 compartido con SDA GPIO21 y SCL GPIO22; LED RGB de ánodo común con canales R GPIO25, G GPIO26 y B GPIO32, cada uno con resistor de 220–330 Ω; buzzer activo en LOW con señal GPIO33 y driver si es un transductor desnudo; botón entre GPIO14 y GND usando pull-up interno. Alimenta HC-SR04 a 5 V, DHT11 y lado ESP32 del LDR a 3.3 V; confirma la entrada del módulo BMP180 por su variante. Une todas las tierras. Advierte con símbolo que ECHO de 5 V nunca va directo al ESP32 y agrega conversor I²C bidireccional si el backpack LCD sube SDA/SCL a 5 V. Etiqueta cada pin, voltaje, resistor, GND y dirección I²C. No inventes otros sensores ni afirmes que el esquema representa un montaje físico verificado.

## Antes de dibujar el montaje final

1. Confirma si el ESP32 físico es DevKit con módulo ESP32 clásico o una variante distinta.
2. Fotografía frente y reverso de cada breakout y anota sus etiquetas y tensión admitida.
3. Confirma dirección I²C con el módulo real; `0x27` es la dirección escrita en el sketch.
4. Mide el divisor de ECHO antes de conectarlo al GPIO.
5. Añade a la entrega una captura del esquemático de Proteus y una tabla que coincida con el prototipo armado.

## Referencias eléctricas

- Espressif Systems, *ESP32 Series Datasheet*, sección de límites eléctricos (GPIO, máximo absoluto de 3.6 V): https://documentation.espressif.com/esp32_datasheet_en.html
- HC-SR04, hoja técnica del módulo (alimentación nominal de 5 V y señal ECHO): https://leantec.es/wp-content/uploads/2019/06/Leantec.ES-HC-SR04.pdf
- Adafruit, *BMP180 Barometric Pressure/Temperature/Altitude Sensor*, características del breakout específico con regulador y conversor de nivel: https://www.adafruit.com/product/1603. Los módulos de otras marcas pueden diferir.
