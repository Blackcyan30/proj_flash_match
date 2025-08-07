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
import sys, re, statistics, numpy as np
path = sys.argv[1]
values = []
with open(path) as f:
    for line in f:
        m = re.search(r"Mean latency:\s+([0-9.]+)", line)
        if m:
            values.append(float(m.group(1)))
if values:
    print("Summary over", len(values), "runs")
    print("Mean:", statistics.mean(values))
    print("Median:", statistics.median(values))
    print("p99:", float(np.percentile(values, 99)))
else:
    print("No latency values found.")
PY
