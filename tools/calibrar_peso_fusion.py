#!/usr/bin/env python3
"""Ajusta alpha del 80/20 con registros reales y una evaluación temporal final."""

import argparse
import csv
from dataclasses import dataclass
from datetime import datetime, timedelta
import math
from pathlib import Path


@dataclass(frozen=True)
class Sample:
    timestamp: datetime
    distance_cm: float | None
    ambient_pct: float | None
    sensors_valid: bool


def parse_number(value):
    try:
        number = float(value)
    except (TypeError, ValueError):
        return None
    return number if math.isfinite(number) else None


def parse_flag(value):
    return str(value).strip().lower() in {"1", "true", "yes", "si", "sí"}


def parse_timestamp(value):
    text = str(value).strip()
    if text.endswith("Z"):
        text = text[:-1] + "+00:00"
    return datetime.fromisoformat(text)


def load_samples(path):
    samples = []
    with path.open(newline="", encoding="utf-8-sig") as stream:
        reader = csv.DictReader(stream)
        required = {"timestamp_utc", "distancia_cm", "indice_ambiental_pct"}
        missing = required - set(reader.fieldnames or [])
        if missing:
            raise ValueError("Faltan columnas: " + ", ".join(sorted(missing)))

        for row in reader:
            try:
                timestamp = parse_timestamp(row["timestamp_utc"])
            except (TypeError, ValueError):
                continue
            distance = parse_number(row.get("distancia_cm"))
            ambient = parse_number(row.get("indice_ambiental_pct"))
            flags_present = all(
                name in row
                for name in ("ultrasonico_valido", "dht_valido", "bmp_valido", "ldr_en_rango")
            )
            valid = (
                flags_present
                and parse_flag(row.get("ultrasonico_valido"))
                and parse_flag(row.get("dht_valido"))
                and parse_flag(row.get("bmp_valido"))
                and parse_flag(row.get("ldr_en_rango"))
                and distance is not None
                and ambient is not None
            )
            samples.append(Sample(timestamp, distance, ambient, valid))

    samples.sort(key=lambda sample: sample.timestamp)
    return samples


def make_labeled_examples(samples, horizon, max_gap, d_operativo, d_critico):
    examples = []
    for i, sample in enumerate(samples):
        if not sample.sensors_valid:
            continue
        level_risk = max(
            0.0,
            min(100.0, 100.0 * (sample.distance_cm - d_operativo) / (d_critico - d_operativo)),
        )

        if sample.distance_cm >= d_critico:
            examples.append((sample.timestamp, level_risk, sample.ambient_pct, 1))
            continue

        end_time = sample.timestamp + horizon
        cursor_time = sample.timestamp
        label = None
        continuous = True
        for future in samples[i + 1 :]:
            gap = future.timestamp - cursor_time
            if gap.total_seconds() > max_gap.total_seconds():
                continuous = False
                break
            if future.timestamp > end_time:
                # Una lectura posterior dentro del máximo intervalo confirma cobertura del horizonte.
                if cursor_time + max_gap >= end_time:
                    label = 0
                break
            if not future.sensors_valid:
                continuous = False
                break
            if future.distance_cm >= d_critico:
                label = 1
                break
            cursor_time = future.timestamp

        if not continuous:
            continue
        if label is None and cursor_time + max_gap >= end_time:
            label = 0
        if label is not None:
            examples.append((sample.timestamp, level_risk, sample.ambient_pct, label))
    return examples


def confusion(examples, alpha, threshold):
    tp = fp = tn = fn = 0
    for _, level_risk, ambient, label in examples:
        predicted = alpha * level_risk + (1.0 - alpha) * ambient >= threshold
        if predicted and label:
            tp += 1
        elif predicted:
            fp += 1
        elif label:
            fn += 1
        else:
            tn += 1
    positives = tp + fn
    negatives = tn + fp
    sensitivity = tp / positives if positives else 0.0
    specificity = tn / negatives if negatives else 0.0
    balanced = (sensitivity + specificity) / 2.0 if positives and negatives else 0.0
    return tp, fp, fn, tn, sensitivity, specificity, balanced


def describe(name, examples, alpha, threshold):
    tp, fp, fn, tn, sensitivity, specificity, balanced = confusion(examples, alpha, threshold)
    print(
        f"{name}: n={len(examples)}; alpha_nivel={alpha:.2f}; "
        f"peso_ambiente={1.0-alpha:.2f}; sensibilidad={sensitivity:.3f}; "
        f"especificidad={specificity:.3f}; exactitud_balanceada={balanced:.3f}; "
        f"TP={tp} FP={fp} FN={fn} TN={tn}"
    )
    return balanced


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", type=Path)
    parser.add_argument("--d-operativo", type=float, default=5.0)
    parser.add_argument("--d-critico", type=float, default=15.0)
    parser.add_argument("--horizonte-min", type=float, default=5.0)
    parser.add_argument("--umbral-riesgo", type=float, default=70.0)
    parser.add_argument("--max-gap-s", type=float, default=6.0)
    parser.add_argument("--train-frac", type=float, default=0.70)
    args = parser.parse_args()

    if args.d_critico <= args.d_operativo:
        parser.error("--d-critico debe ser mayor que --d-operativo")
    if args.horizonte_min <= 0 or args.max_gap_s <= 0:
        parser.error("horizonte y separación máxima deben ser mayores que cero")
    if not 0.5 <= args.train_frac <= 0.9:
        parser.error("--train-frac debe estar entre 0.5 y 0.9")

    samples = load_samples(args.csv)
    if len(samples) < 20:
        parser.error("Hay menos de 20 filas; registra más pruebas antes de calibrar.")

    horizon = timedelta(minutes=args.horizonte_min)
    max_gap = timedelta(seconds=args.max_gap_s)
    examples = make_labeled_examples(
        samples, horizon, max_gap, args.d_operativo, args.d_critico
    )
    if len(examples) < 20:
        parser.error("Hay menos de 20 ejemplos continuos con sensores válidos y horizonte observado.")

    split_index = int(len(samples) * args.train_frac)
    split_time = samples[split_index].timestamp
    train = [
        example
        for example in examples
        if example[0] + horizon <= split_time
    ]
    test = [
        example
        for example in examples
        if example[0] >= split_time + horizon
    ]
    if not train or not test:
        parser.error("No alcanza la serie para separar ajuste y prueba con una brecha temporal.")
    if not any(item[3] == 0 for item in train) or not any(item[3] == 1 for item in train):
        parser.error("El tramo de ajuste necesita ejemplos con y sin llegada al nivel crítico.")
    if not any(item[3] == 0 for item in test) or not any(item[3] == 1 for item in test):
        parser.error("El tramo final necesita ejemplos con y sin llegada al nivel crítico.")

    best_alpha = None
    best_score = -1.0
    for step in range(101):
        alpha = step / 100.0
        score = confusion(train, alpha, args.umbral_riesgo)[-1]
        if score > best_score + 1e-12 or (
            abs(score - best_score) <= 1e-12 and (best_alpha is None or alpha > best_alpha)
        ):
            best_alpha = alpha
            best_score = score

    print(f"Filas originales: {len(samples)}; ejemplos etiquetados: {len(examples)}")
    print(f"Corte temporal: {split_time.isoformat()}; umbral de riesgo: {args.umbral_riesgo:.1f}")
    print("Ajuste: se elige alpha por exactitud balanceada solo en el tramo inicial.")
    describe("AJUSTE", train, best_alpha, args.umbral_riesgo)
    describe("PRUEBA FUTURA", test, best_alpha, args.umbral_riesgo)
    describe("BASE 80/20 EN PRUEBA", test, 0.80, args.umbral_riesgo)
    print("No adoptes el peso ajustado si el resultado futuro es peor/inestable o faltan eventos representativos.")


if __name__ == "__main__":
    main()
