#!/bin/bash

## THIS IS AN AI GENERATED SCRIPT

# record_perf.sh - Record binary execution with perf and generate a flamegraph.
# Usage: ./record_perf.sh <binary_path> [binary_args...]

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <binary_path> [binary_args...]"
    exit 1
fi

BINARY_PATH=$1
shift
BINARY_ARGS=$@

if [ ! -f "$BINARY_PATH" ]; then
    echo "Error: Binary not found at $BINARY_PATH"
    exit 1
fi

if [ ! -x "$BINARY_PATH" ]; then
    echo "Error: $BINARY_PATH is not executable"
    exit 1
fi

# 1. Check for perf
if ! command -v perf &> /dev/null; then
    echo "Error: 'perf' not found. Please install linux-tools."
    exit 1
fi

# 2. Check for FlameGraph tools
FLAMEGRAPH_PL=$(command -v flamegraph.pl)
STACKCOLLAPSE_PL=$(command -v stackcollapse-perf.pl)

if [ -z "$FLAMEGRAPH_PL" ] || [ -z "$STACKCOLLAPSE_PL" ]; then
    # Fallback to common location
    FLAMEGRAPH_PL=~/FlameGraph/flamegraph.pl
    STACKCOLLAPSE_PL=~/FlameGraph/stackcollapse-perf.pl
fi

if [ ! -f "$FLAMEGRAPH_PL" ] || [ ! -f "$STACKCOLLAPSE_PL" ]; then
    echo "Error: FlameGraph tools not found. Please clone https://github.com/brendangregg/FlameGraph"
    echo "and add it to your PATH or place it in ~/FlameGraph"
    exit 1
fi

OUT_SVG="flamegraph.svg"
PERF_DATA="perf.data"

echo "Recording execution of '$BINARY_PATH $BINARY_ARGS'..."
# -g enables call-graph recording
perf record -g -F 9999 -- "$BINARY_PATH" $BINARY_ARGS

if [ ! -f "$PERF_DATA" ]; then
    echo "Error: perf record failed to produce $PERF_DATA"
    exit 1
fi

echo "Generating flamegraph..."
perf script | "$STACKCOLLAPSE_PL" | "$FLAMEGRAPH_PL" > "$OUT_SVG"

if [ -f "$OUT_SVG" ]; then
    echo "Success! Flamegraph generated: $OUT_SVG"
else
    echo "Error: Failed to generate $OUT_SVG"
    exit 1
fi
