#!/usr/bin/env python3
import pathlib
import re
import math
import matplotlib.pyplot as plt


def load_all_arrays_from_txt(path: pathlib.Path) -> list[list[int]]:
    text = path.read_text()
    token_re = re.compile(r'(\d+)\s*<repeats\s+(\d+)\s+times>|(\d+)')
    arrays = []

    for match in re.finditer(r'\$\d+\s*=\s*\{(.+?)\}', text, re.DOTALL):
        raw = match.group(1)
        values = []
        for m in token_re.finditer(raw):
            if m.group(1) is not None:
                values.extend([int(m.group(1))] * int(m.group(2)))
            else:
                values.append(int(m.group(3)))
        if values:
            arrays.append(values)

    return arrays


def plot_all_and_mean(arrays: list[list[int]]):
    min_len = min(len(a) for a in arrays)
    trimmed = [a[:min_len] for a in arrays]
    n = len(trimmed)
    xs = range(min_len)

    mean = [sum(trimmed[i][j] for i in range(n)) / n for j in xs]

    fig, ax = plt.subplots(figsize=(14, 5))

    for i, arr in enumerate(trimmed):
        ax.plot(arr, linewidth=0.4, alpha=0.5, label=f"Run {i+1}")

    ax.plot(mean, linewidth=1.5, color="red", label="Mean", zorder=10)

    ax.set_xlabel("Index")
    ax.set_ylabel("Value (cycles)")
    ax.set_title(f"All runs + mean ({n} iterations)")
    ax.legend(loc="upper right", fontsize=7)
    fig.tight_layout()


def plot_all_and_std(arrays: list[list[int]]):
    min_len = min(len(a) for a in arrays)
    trimmed = [a[:min_len] for a in arrays]
    n = len(trimmed)
    xs = list(range(min_len))

    mean = [sum(trimmed[i][j] for i in range(n)) / n for j in xs]
    std  = [math.sqrt(sum((trimmed[i][j] - mean[j]) ** 2 for i in range(n)) / n)
            for j in xs]

    upper = [mean[j] + std[j] for j in xs]
    lower = [mean[j] - std[j] for j in xs]

    fig, ax = plt.subplots(figsize=(14, 5))

    ax.plot(mean,  linewidth=1.5, color="red",    label="Mean",       zorder=10)
    ax.plot(upper, linewidth=1.0, color="orange", linestyle="--", label="+1 std", zorder=9)
    ax.plot(lower, linewidth=1.0, color="orange", linestyle="--", label="-1 std", zorder=9)
    ax.fill_between(xs, lower, upper, color="orange", alpha=0.15, label="±1 std band")

    ax.set_xlabel("Index")
    ax.set_ylabel("Value (cycles)")
    ax.set_title(f"Mean ± std deviation ({n} iterations)")
    ax.legend(loc="upper right", fontsize=7)
    fig.tight_layout()

if __name__ == "__main__":
    path = pathlib.Path("result_analysis/data_output/spy_results2.txt")
    arrays = load_all_arrays_from_txt(path)
    print(f"Found {len(arrays)} arrays, lengths: {[len(a) for a in arrays]}")

    plot_all_and_mean(arrays)
    plot_all_and_std(arrays)
    plt.show()