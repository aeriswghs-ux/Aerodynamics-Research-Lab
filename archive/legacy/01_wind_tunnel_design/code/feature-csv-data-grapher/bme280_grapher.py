#!/usr/bin/env python3
"""
BME280 CSV Grapher

Reads BME280 sensor data from a CSV file and generates an aesthetically styled
dashboard plot using matplotlib + seaborn.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

try:
    import matplotlib.pyplot as plt
    import pandas as pd
    import seaborn as sns
except ModuleNotFoundError as exc:
    missing = exc.name or "a required package"
    raise SystemExit(
        f"Missing dependency: {missing}. Install requirements with: "
        "pip install -r requirements.txt"
    ) from exc


DEFAULT_TIME_COLUMNS = ("timestamp", "time", "datetime", "date")
DEFAULT_TEMP_COLUMNS = ("temperature_c", "temperature", "temp_c", "temp")
DEFAULT_HUMIDITY_COLUMNS = ("humidity", "humidity_percent", "rh")
DEFAULT_PRESSURE_COLUMNS = ("pressure_hpa", "pressure", "press_hpa")


def _pick_column(df: pd.DataFrame, candidates: tuple[str, ...], required: bool = True) -> str | None:
    lower_to_real = {c.strip().lower(): c for c in df.columns}
    for candidate in candidates:
        if candidate in lower_to_real:
            return lower_to_real[candidate]
    if required:
        raise ValueError(f"Could not find required column. Expected one of: {', '.join(candidates)}")
    return None


def load_bme280_csv(path: Path) -> tuple[pd.DataFrame, str, str, str, str | None]:
    df = pd.read_csv(path)
    if df.empty:
        raise ValueError("CSV appears to be empty.")

    normalized = [c.strip() for c in df.columns]
    df.columns = normalized

    time_col = _pick_column(df, DEFAULT_TIME_COLUMNS, required=False)
    temp_col = _pick_column(df, DEFAULT_TEMP_COLUMNS)
    humidity_col = _pick_column(df, DEFAULT_HUMIDITY_COLUMNS)
    pressure_col = _pick_column(df, DEFAULT_PRESSURE_COLUMNS)

    if time_col:
        df[time_col] = pd.to_datetime(df[time_col], errors="coerce")
        if df[time_col].isna().all():
            raise ValueError(f"Found '{time_col}' column but failed to parse any timestamps.")
        df = df.dropna(subset=[time_col])
        df = df.sort_values(time_col).reset_index(drop=True)

    for col in (temp_col, humidity_col, pressure_col):
        df[col] = pd.to_numeric(df[col], errors="coerce")

    df = df.dropna(subset=[temp_col, humidity_col, pressure_col])
    if df.empty:
        raise ValueError("No valid rows after cleaning numeric BME280 columns.")

    return df, temp_col, humidity_col, pressure_col, time_col


def render_plot(
    df: pd.DataFrame,
    temp_col: str,
    humidity_col: str,
    pressure_col: str,
    time_col: str | None,
    output_path: Path,
    title: str,
) -> None:
    sns.set_theme(style="whitegrid", context="talk")
    plt.rcParams.update(
        {
            "axes.facecolor": "#f8fafb",
            "figure.facecolor": "#eef3f6",
            "axes.edgecolor": "#9fb0bc",
            "grid.color": "#d7e0e6",
            "axes.titleweight": "bold",
            "axes.labelweight": "semibold",
        }
    )

    fig, axes = plt.subplots(3, 1, figsize=(14, 10), sharex=True)
    fig.suptitle(title, fontsize=20, fontweight="bold", y=0.98)

    x = df[time_col] if time_col else df.index
    x_label = "Time" if time_col else "Sample #"

    sns.lineplot(ax=axes[0], x=x, y=df[temp_col], color="#e76f51", linewidth=2.2)
    axes[0].set_title("Temperature")
    axes[0].set_ylabel("deg C")

    sns.lineplot(ax=axes[1], x=x, y=df[humidity_col], color="#2a9d8f", linewidth=2.2)
    axes[1].set_title("Humidity")
    axes[1].set_ylabel("% RH")

    sns.lineplot(ax=axes[2], x=x, y=df[pressure_col], color="#264653", linewidth=2.2)
    axes[2].set_title("Pressure")
    axes[2].set_ylabel("hPa")
    axes[2].set_xlabel(x_label)

    for ax in axes:
        ax.margins(x=0.01)
        for spine in ax.spines.values():
            spine.set_alpha(0.5)

    fig.tight_layout(rect=[0, 0, 1, 0.965])
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=180, bbox_inches="tight")
    plt.close(fig)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate beautiful BME280 plots from CSV data.")
    parser.add_argument("csv", type=Path, help="Path to BME280 CSV file")
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=Path("bme280_dashboard.png"),
        help="Output image path (default: bme280_dashboard.png)",
    )
    parser.add_argument(
        "--title",
        type=str,
        default="BME280 Environmental Dashboard",
        help="Custom plot title",
    )
    return parser.parse_args()


def main() -> None:
    try:
        args = parse_args()
        df, temp_col, humidity_col, pressure_col, time_col = load_bme280_csv(args.csv)
        render_plot(df, temp_col, humidity_col, pressure_col, time_col, args.output, args.title)
        print(f"Saved graph to: {args.output}")
    except (FileNotFoundError, ValueError, OSError, pd.errors.ParserError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        raise SystemExit(1) from exc


if __name__ == "__main__":
    main()
