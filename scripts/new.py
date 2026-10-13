#!/usr/bin/env python3
import re
import pathlib
import sys
import matplotlib.pyplot as plt

def load_numbers_from_txt(path: pathlib.Path) -> list[int]:
    """
    Parse GDB output like:
      $2 = {123, 456, 0 <repeats 12 times>, 789, ...}
    and return a flat list of integers (zeros stripped).
    """
    text = path.read_text()

    arrays = []
    for match in re.finditer(r'\$\d+\s*=\s*\{(.+?)\}', text, re.DOTALL):
        raw = match.group(1)
        values = []
        token_re = re.compile(r'(\d+)\s*<repeats\s+(\d+)\s+times>|(\d+)')
        for m in token_re.finditer(raw):
            if m.group(1) is not None:
                values.extend([int(m.group(1))] * int(m.group(2)))
            else:
                values.append(int(m.group(3)))
        arrays.append([v for v in values])

    return arrays


if len(sys.argv) < 2:
    print("Usage: python plot_measurements.py <gdb_output.txt>")
    sys.exit(1)

arrays = load_numbers_from_txt(pathlib.Path(sys.argv[1]))

if len(arrays) < 4:
    print(f"Expected 3 arrays, found {len(arrays)}")
    sys.exit(1)

d1, d2, d3 , d4 = arrays[0], arrays[1], arrays[2], arrays[3]
labels = ["$1 - foo_time(core1)", "$2 - Timestamp(core1)", "$3 - Timestamp(core0)", "$4 - Zeros(core0)"]
colors = ["blue", "green", "green", "red"]

fig, axes = plt.subplots(2, 3, figsize=(12, 8))

for ax, data, label, color in zip(axes.flat, [d1, d2, d3, d4], labels, colors):
    ax.plot(data, color=color)
    ax.set_title(label)
 

axes[1, 2].plot(d1, label="$1", color='blue')
for i, timestamp in enumerate(d4):
    if timestamp != 0:
        axes[1, 2].axvline(i, color='red', alpha=0.5)
axes[1, 2].set_title("Together")
axes[1, 2].legend(["foo_time(core1)", "foo(core0)"])

plt.tight_layout()
plt.savefig("measurements.png")
plt.show()
