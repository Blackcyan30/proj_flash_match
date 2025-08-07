#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <dataset> <iterations> [core]" >&2
    exit 1
fi

dataset=$1
iterations=$2
core=${3:-0}
log="bench.log"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exe="${SCRIPT_DIR}/../orderbook_bench"
if [ ! -x "$exe" ]; then
    echo "Executable not found: $exe" >&2
    exit 1
fi

rm -f "$log"

for i in $(seq 1 "$iterations"); do
    echo "Run $i:" >> "$log"
    if command -v taskset >/dev/null 2>&1; then
        taskset -c "$core" "$exe" "$dataset" >> "$log"
    else
        "$exe" "$dataset" >> "$log"
    fi
done

python3 - "$log" <<'PY' | tee -a "$log"
import sys, re, statistics

path = sys.argv[1]
values = []

with open(path) as f:
    for line in f:
        match = re.search(r"Mean latency:\s+([0-9.]+)", line)
        if match:
            values.append(float(match.group(1)))

if values:
    mean = statistics.mean(values)
    median = statistics.median(values)
    try:
        p99 = statistics.quantiles(values, n=100)[98]
    except statistics.StatisticsError:
        p99 = values[-1]
    print(f"Summary over {len(values)} runs")
    print(f"Mean: {mean}")
    print(f"Median: {median}")
    print(f"p99: {p99}")
else:
    print("No latency values found.")
PY
