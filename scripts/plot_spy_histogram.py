#!/usr/bin/env python3
import pathlib
import struct
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import re


def load_numbers_from_txt(path: pathlib.Path) -> list[int]:
    """
    Parse GDB output like:
      $2 = {123, 456, 0 <repeats 12 times>, 789, ...}
    and return a flat list of integers.
    """
    text = path.read_text()

    match = re.search(r'\$\d+\s*=\s*\{(.+)\}', text, re.DOTALL)
    if not match:
        raise ValueError("Could not find a GDB array assignment in the file.")

    raw = match.group(1)

    values = []
    token_re = re.compile(r'(\d+)\s*<repeats\s+(\d+)\s+times>|(\d+)')
    for m in token_re.finditer(raw):
        if m.group(1) is not None:
            values.extend([int(m.group(1))] * int(m.group(2)))
        else:
            values.append(int(m.group(3)))

    return values


def load_numbers_from_bin(path: pathlib.Path) -> list[int]:
    data = path.read_bytes()
    if len(data) % 4 != 0:
        raise ValueError(f"{path} size is not divisible by 4 bytes")
    return [value for (value,) in struct.iter_unpack("<I", data)]


def color_for_value(value: int, threshold: int) -> str:
    return "green" if value < threshold else "red"


def plot_raw_scatter(times: list[int], bins: int = 80, title: str = "Value Distribution"):
    """Graph 1: histogram of all values — value on x, frequency on y."""
    fig, ax = plt.subplots(figsize=(12, 4))
    ax.hist(times, bins=bins, color="steelblue", alpha=0.7)
    ax.set_xlabel("Value (cycles)")
    ax.set_ylabel("Frequency")
    ax.set_title(title)
    fig.tight_layout()
    return fig

def plot_hit_miss_scatter(times: list[int],
                          threshold: int | None = None,
                          title: str = "Hit / Miss Scatter"):
    """
    Graph 2: scatter coloured green (hit) / red (miss).
    X = rolling average, Y = actual value.
    """
    if threshold is None:
        lo = min(times)
        hi = max(times)
        threshold = lo + (hi - lo) // 3

    window = 20
    smoothed = []
    for i in range(len(times)):
        start = max(0, i - window // 2)
        end = min(len(times), i + window // 2)
        smoothed.append(sum(times[start:end]) / (end - start))

    colors = ["green" if v < threshold else "red" for v in times]

    fig, ax = plt.subplots(figsize=(10, 6))
    ax.scatter(smoothed, times, c=colors, s=6, alpha=0.6)
    ax.axhline(threshold, color="black", linestyle="--", linewidth=1)
    ax.set_xlabel("Rolling average value (cycles)")
    ax.set_ylabel("Actual value (cycles)")
    ax.set_title(title)
    ax.legend(handles=[
        plt.Line2D([0], [0], marker='o', color='w',
                   markerfacecolor='green', markersize=8, label='Hit'),
        plt.Line2D([0], [0], marker='o', color='w',
                   markerfacecolor='red', markersize=8, label='Miss'),
        plt.Line2D([0], [0], color='black', linestyle='--',
                   label=f'Threshold = {threshold}'),
    ])
    fig.tight_layout()
    return fig


def plot_hit_miss_histogram(times: list[int],
                            threshold: int | None = None,
                            bins: int = 80,
                            title: str = "Hit / Miss Histogram"):
    """
    Graph 3: overlapping histograms of hits (green) and misses (red).
    A vertical dashed line marks the threshold.
    """
    if threshold is None:
        lo = min(times)
        hi = max(times)
        threshold = lo + (hi - lo) // 3

    hits   = [v for v in times if v <  threshold]
    misses = [v for v in times if v >= threshold]

    fig, ax = plt.subplots(figsize=(10, 5))

    ax.hist(hits,   bins=bins, color="green", alpha=0.6, label=f"Hits   (n={len(hits)})")
    ax.hist(misses, bins=bins, color="red",   alpha=0.6, label=f"Misses (n={len(misses)})")

    ax.axvline(threshold, color="black", linestyle="--", linewidth=1.5,
               label=f"Threshold = {threshold}")

    ax.set_xlabel("Value (cycles)")
    ax.set_ylabel("Count")
    ax.set_title(title)
    ax.legend()
    fig.tight_layout()
    return fig

def plot_raw(times: list[int], title: str = "Raw Access Times"):
    """Graph 1: line graph of all values in order."""
    fig, ax = plt.subplots(figsize=(12, 4))
    ax.plot(times, linewidth=0.5, color="steelblue")
    ax.set_xlabel("Index")
    ax.set_ylabel("Value (cycles)")
    ax.set_title(title)
    fig.tight_layout()
    return fig

if __name__ == "__main__":
    values = load_numbers_from_txt(
        pathlib.Path("result_analysis/data_output/spy_results.txt")
    )
    values = [x for x in values if x > 0]
    print(f"Loaded {len(values)} values")
    print(f"  min={min(values)}  max={max(values)}  mean={sum(values)/len(values):.1f}")

    fig1 = plot_raw_scatter(values)
    fig2 = plot_hit_miss_scatter(values)
    fig3 = plot_hit_miss_histogram(values)   # pass threshold=N to override
    fig4 = plot_raw(values)

    plt.show()