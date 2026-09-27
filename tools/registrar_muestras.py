#!/usr/bin/env python3
"""Registra localmente las muestras publicadas por el WMS en /api/status."""

import argparse
import csv
import json
import math
import sys
import time
from datetime import datetime, timezone
from pathlib import Path
from urllib.error import URLError
from urllib.request import urlopen


FIELDS = [
    "timestamp_utc",
    "uptime_s",
    "distancia_cm",
    "indice_ambiental_pct",
    "temperatura_c",
    "humedad_pct",
    "presion_hpa",
    "luz_relativa_pct",
    "ultrasonico_valido",
    "dht_valido",
    "bmp_valido",
    "ldr_en_rango",
    "evento_llenado_o_extraccion",
    "observaciones",
]


def number_or_blank(value):
    if isinstance(value, (int, float)) and math.isfinite(value):
        return value
    return ""


def flag(value):
    return 1 if value is True else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", default="http://192.168.4.1/api/status")
    parser.add_argument("--interval", type=float, default=2.0, help="segundos entre consultas")
    parser.add_argument("--output", default="data/muestras_wms.csv")
    args = parser.parse_args()
    if args.interval <= 0:
        parser.error("--interval debe ser mayor que cero")

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    write_header = not output.exists() or output.stat().st_size == 0
    print(f"Registrando {args.url} en {output}. Ctrl+C para terminar.")

    try:
        with output.open("a", newline="", encoding="utf-8") as stream:
            writer = csv.DictWriter(stream, fieldnames=FIELDS)
            if write_header:
                writer.writeheader()
                stream.flush()

            siguiente = time.monotonic()
            while True:
                try:
                    with urlopen(args.url, timeout=max(3.0, args.interval + 1.0)) as response:
                        data = json.load(response)
                    sensors = data.get("sensors", {})
                    writer.writerow(
                        {
                            "timestamp_utc": datetime.now(timezone.utc).isoformat(timespec="seconds"),
                            "uptime_s": number_or_blank(data.get("uptime_s")),
                            "distancia_cm": number_or_blank(data.get("level_cm")),
                            "indice_ambiental_pct": number_or_blank(data.get("ambient_pct")),
                            "temperatura_c": number_or_blank(data.get("temp_c")),
                            "humedad_pct": number_or_blank(data.get("rh_pct")),
                            "presion_hpa": number_or_blank(data.get("pressure_hpa")),
                            "luz_relativa_pct": number_or_blank(data.get("light_pct")),
                            "ultrasonico_valido": flag(sensors.get("ultrasonic_valid")),
                            "dht_valido": flag(sensors.get("dht_valid")),
                            "bmp_valido": flag(sensors.get("bmp_valid")),
                            "ldr_en_rango": flag(sensors.get("ldr_in_calibrated_range")),
                            "evento_llenado_o_extraccion": "",
                            "observaciones": "",
                        }
                    )
                    stream.flush()
                    print(
                        f"{datetime.now().strftime('%H:%M:%S')}  "
                        f"d={data.get('level_cm')} cm  estado={data.get('state')}"
                    )
                except (URLError, TimeoutError, json.JSONDecodeError, OSError) as error:
                    print(f"No se pudo registrar esta muestra: {error}", file=sys.stderr)

                siguiente += args.interval
                time.sleep(max(0.0, siguiente - time.monotonic()))
    except KeyboardInterrupt:
        print("\nRegistro detenido.")


if __name__ == "__main__":
    main()
