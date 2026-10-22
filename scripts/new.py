#!/usr/bin/env python3
import re
import pathlib
import sys
import matplotlib.pyplot as plt


def load_numbers_from_txt(path: pathlib.Path):
    """Parse GDB output containing 1D and 2D arrays, including repeated rows."""
    text = path.read_text()
    arrays = []
    assignment_re = re.compile(r'\$\d+\s*=\s*\{')

    def split_top_level(raw):
        entries = []
        start = 0
        depth = 0

        for i, char in enumerate(raw):
            if char == '{':
                depth += 1
            elif char == '}':
                depth -= 1
            elif char == ',' and depth == 0:
                entries.append(raw[start:i].strip())
                start = i + 1

        entries.append(raw[start:].strip())
        return entries

    def expand_values(entry):
        """Parse integers and GDB's repeated-integer notation."""
        values = []
        token_re = re.compile(r'(\d+)\s*<repeats\s+(\d+)\s+times>|(\d+)')

        for match in token_re.finditer(entry):
            if match.group(1) is not None:
                values.extend([int(match.group(1))] * int(match.group(2)))
            else:
                values.append(int(match.group(3)))

        return values

    for match in assignment_re.finditer(text):
        start = match.end() - 1
        depth = 0
        end = None

        for pos in range(start, len(text)):
            if text[pos] == '{':
                depth += 1
            elif text[pos] == '}':
                depth -= 1
                if depth == 0:
                    end = pos
                    break

        if end is None:
            continue

        raw = text[start + 1:end]
        entries = split_top_level(raw)

        if any(entry.strip().startswith('{') for entry in entries):
            rows = []

            for entry in entries:
                entry = entry.strip()

                # Match a complete row, optionally followed by GDB repetition.
                row_match = re.fullmatch(
                    r'(\{.*\})\s*(?:<repeats\s+(\d+)\s+times>)?',
                    entry,
                    re.DOTALL
                )

                if row_match:
                    row_text = row_match.group(1)[1:-1]
                    row = []

                    for item in split_top_level(row_text):
                        row.extend(expand_values(item))

                    repeat_count = int(row_match.group(2) or 1)

                    # Repeat the entire row, not just its individual values.
                    rows.extend([row.copy() for _ in range(repeat_count)])
                else:
                    # Handle scalar entries if present.
                    values = expand_values(entry)
                    rows.extend([[value] for value in values])

            arrays.append(rows)

        else:
            values = []
            for entry in entries:
                values.extend(expand_values(entry))
            arrays.append(values)

    return arrays


if len(sys.argv) < 2:
    print("Usage: python new.py <gdb_output.txt>")
    sys.exit(1)

arrays = load_numbers_from_txt(pathlib.Path(sys.argv[1]))

if len(arrays) < 5:
    print(
        f"Expected at least 5 arrays (including foo_time2), "
        f"found {len(arrays)}"
    )
    sys.exit(1)

d1, d2, d3, d4 = arrays[0], arrays[1], arrays[2], arrays[3]
foo_time2 = arrays[4]

labels = [
    "$1 - foo_time(core1)",
    "$2 - Timestamp(core1)",
    "$3 - Timestamp(core0)",
    "$4 - Zeros(core0)"
]
colors = ["blue", "green", "green", "red"]

fig, axes = plt.subplots(2, 3, figsize=(15, 9))

# Existing individual plots.
for ax, data, label, color in zip(
    [axes[0, 0], axes[0, 1], axes[0, 2], axes[1, 0]],
    [d1, d2, d3, d4],
    labels,
    colors
):
    ax.plot(data, color=color)
    ax.set_title(label)
    ax.set_xlabel("Iteration")
    ax.set_ylabel("Cycles")

# New lower-middle plot: foo_time2[iteration][SET].
if foo_time2:
    num_sets = max(len(row) for row in foo_time2)

    for set_index in range(num_sets):
        set_values = [
            row[set_index] if set_index < len(row) else float("nan")
            for row in foo_time2
        ]
        axes[1, 1].plot(set_values, label=f"Set {set_index}")

    axes[1, 1].legend()

axes[1, 1].set_title("foo_time2 per Set")
axes[1, 1].set_xlabel("Iteration")
axes[1, 1].set_ylabel("Cycles")

# Existing Together plot.
axes[1, 2].plot(d1, label="foo_time(core1)", color="blue")

for i, timestamp in enumerate(d4):
    if timestamp != 0:
        axes[1, 2].axvline(i, color="red", alpha=0.5)

axes[1, 2].set_title("Together")
axes[1, 2].legend(["foo_time(core1)", "foo(core0)"])
axes[1, 2].set_xlabel("Iteration")
axes[1, 2].set_ylabel("Cycles")

plt.tight_layout()
plt.savefig("measurements.png")
plt.show()