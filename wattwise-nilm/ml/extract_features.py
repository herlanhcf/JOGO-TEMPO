#!/usr/bin/env python3
"""Extrai o vetor de 20 features de eventos NILM.

A definição DEVE ser idêntica à do firmware (firmware/src/dsp/features.*), senão o
modelo treinado aqui não vale nada no dispositivo. Layout:

  0 delta_p  1 delta_q  2 delta_s  3 pf_before  4 pf_after  5 sign
  6 inrush_ratio  7 t_settle_ms  8..14 H1..H7  15..19 reservado

Entrada esperada: CSVs com colunas de forma de onda por evento (v[], i[]) e um rótulo,
OU CSVs já sumarizados com P/Q/harmônicas. Este script cobre o caminho a partir de
janelas brutas; adapte `load_events` ao seu formato de coleta.
"""
import argparse
import glob
import os
import numpy as np
import pandas as pd

N_FEATURES = 20
N_HARMONICS = 7


def harmonics(current: np.ndarray, samples_per_cycle: int) -> np.ndarray:
    """Magnitudes relativas das N_HARMONICS harmônicas ímpares da corrente."""
    n = len(current)
    spectrum = np.fft.rfft(current * np.hanning(n))
    mag = np.abs(spectrum)
    # bin da fundamental ~ n / samples_per_cycle
    fund = max(1, round(n / samples_per_cycle))
    out = np.zeros(N_HARMONICS)
    base = mag[fund] if mag[fund] > 1e-9 else 1e-9
    for h in range(N_HARMONICS):
        b = fund * (2 * h + 1)
        out[h] = (mag[b] / base) if b < len(mag) else 0.0
    return out


def features_from_event(ev: dict, samples_per_cycle: int) -> np.ndarray:
    f = np.zeros(N_FEATURES, dtype=np.float32)
    f[0] = ev["delta_p"]
    f[1] = ev["delta_q"]
    f[2] = np.hypot(ev["delta_p"], ev["delta_q"])
    f[3] = ev["pf_before"]
    f[4] = ev["pf_after"]
    f[5] = 1.0 if ev["delta_p"] >= 0 else -1.0
    f[6] = ev.get("inrush_ratio", 0.0)
    f[7] = ev.get("t_settle_ms", 0.0)
    if "current_window" in ev:
        f[8:8 + N_HARMONICS] = harmonics(np.asarray(ev["current_window"]),
                                         samples_per_cycle)
    return f


def load_events(path: str):
    """Carrega eventos rotulados. Adapte ao formato real do seu coletor.

    Espera arquivos <label>_*.csv, cada linha um evento com colunas:
    delta_p, delta_q, pf_before, pf_after[, inrush_ratio, t_settle_ms].
    """
    files = glob.glob(os.path.join(path, "*.csv"))
    for fp in files:
        label = os.path.basename(fp).split("_")[0]
        df = pd.read_csv(fp)
        for _, row in df.iterrows():
            yield label, row.to_dict()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in", dest="inp", required=True, help="pasta com CSVs de eventos")
    ap.add_argument("--out", default="features.csv")
    ap.add_argument("--spc", type=int, default=66, help="amostras por ciclo")
    args = ap.parse_args()

    rows, labels = [], []
    for label, ev in load_events(args.inp):
        rows.append(features_from_event(ev, args.spc))
        labels.append(label)

    if not rows:
        raise SystemExit(f"nenhum evento encontrado em {args.inp}")

    X = np.vstack(rows)
    df = pd.DataFrame(X, columns=[f"f{i}" for i in range(N_FEATURES)])
    df["label"] = labels
    df.to_csv(args.out, index=False)
    print(f"{len(df)} eventos → {args.out}")


if __name__ == "__main__":
    main()
