#!/usr/bin/env python3
"""Treina um MLP pequeno para classificar eventos NILM e o quantiza para int8 (TFLite).

Saídas:
  - model.tflite   : modelo quantizado para embarcar (TFLite Micro / ESP-DL)
  - norm_stats.h   : média/desvio por feature p/ colar no firmware (normalização)
  - labels.txt     : índice → nome do aparelho

Uso:
  python train_nilm.py --in features.csv --out model.tflite
"""
import argparse
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report
import tensorflow as tf

N_FEATURES = 20


def build_model(n_classes: int) -> tf.keras.Model:
    m = tf.keras.Sequential([
        tf.keras.layers.Input(shape=(N_FEATURES,)),
        tf.keras.layers.Dense(32, activation="relu"),
        tf.keras.layers.Dropout(0.3),
        tf.keras.layers.Dense(16, activation="relu"),
        tf.keras.layers.Dropout(0.3),
        tf.keras.layers.Dense(n_classes, activation="softmax"),
    ])
    m.compile(optimizer="adam", loss="sparse_categorical_crossentropy",
              metrics=["accuracy"])
    return m


def export_norm_header(mean, std, labels, path="norm_stats.h"):
    with open(path, "w") as fh:
        fh.write("// Gerado por train_nilm.py — cole em firmware/src/nilm/\n")
        fh.write("#pragma once\n")
        fh.write(f"#define N_FEATURES {N_FEATURES}\n")
        fh.write("static const float kMean[N_FEATURES] = {" +
                 ",".join(f"{v:.6g}" for v in mean) + "};\n")
        fh.write("static const float kStd[N_FEATURES] = {" +
                 ",".join(f"{v:.6g}" for v in std) + "};\n")
        fh.write("// labels: " + ", ".join(labels) + "\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in", dest="inp", required=True)
    ap.add_argument("--out", default="model.tflite")
    ap.add_argument("--epochs", type=int, default=200)
    args = ap.parse_args()

    df = pd.read_csv(args.inp)
    feat_cols = [f"f{i}" for i in range(N_FEATURES)]
    X = df[feat_cols].to_numpy(dtype=np.float32)
    labels = sorted(df["label"].unique())
    label_to_idx = {l: i for i, l in enumerate(labels)}
    y = df["label"].map(label_to_idx).to_numpy()

    # Normalização (guardada p/ o firmware aplicar a mesma transformação)
    mean = X.mean(axis=0)
    std = X.std(axis=0) + 1e-6
    Xn = (X - mean) / std

    Xtr, Xte, ytr, yte = train_test_split(Xn, y, test_size=0.2,
                                          stratify=y, random_state=42)

    model = build_model(len(labels))
    model.fit(Xtr, ytr, validation_data=(Xte, yte),
              epochs=args.epochs, batch_size=32, verbose=2)

    yp = model.predict(Xte).argmax(axis=1)
    print(classification_report(yte, yp, target_names=labels, zero_division=0))

    # Quantização int8 com dataset representativo
    def rep_data():
        for i in range(min(200, len(Xtr))):
            yield [Xtr[i:i + 1].astype(np.float32)]

    conv = tf.lite.TFLiteConverter.from_keras_model(model)
    conv.optimizations = [tf.lite.Optimize.DEFAULT]
    conv.representative_dataset = rep_data
    conv.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    conv.inference_input_type = tf.int8
    conv.inference_output_type = tf.int8
    tflite = conv.convert()

    with open(args.out, "wb") as fh:
        fh.write(tflite)
    export_norm_header(mean, std, labels)
    with open("labels.txt", "w") as fh:
        fh.write("\n".join(labels))
    print(f"modelo: {args.out} ({len(tflite)} bytes)  |  {len(labels)} classes")


if __name__ == "__main__":
    main()
