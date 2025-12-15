#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="$SCRIPT_DIR/src"
OUTPUT_DIR="$SCRIPT_DIR/output"
EXECUTABLE="$SRC_DIR/wildwater"

START_TIME=$(python3 -c 'import time; print(int(time.time()*1000))')

show_duration() {
    END_TIME=$(python3 -c 'import time; print(int(time.time()*1000))')
    DURATION=$((END_TIME - START_TIME))
    echo "Duration: ${DURATION} ms"
}

error_exit() {
    echo "Error: $1" >&2
    show_duration
    exit 1
}

if [ $# -lt 2 ]; then
    error_exit "Usage: $0 <data_file> <command> [options]
Commands:
  histo max   - Histogram of max treatment capacity
  histo src   - Histogram of source captured volumes
  histo real  - Histogram of real treated volumes
  leaks <id>  - Calculate leaks for a specific plant"
fi

DATA_FILE="$1"
COMMAND="$2"

if [ ! -f "$DATA_FILE" ]; then
    error_exit "Data file not found: $DATA_FILE"
fi

mkdir -p "$OUTPUT_DIR"

if [ ! -f "$EXECUTABLE" ]; then
    echo "Compiling C program..."
    make -C "$SRC_DIR" || error_exit "Compilation failed"
fi

if [ ! -x "$EXECUTABLE" ]; then
    error_exit "Executable not found or not executable: $EXECUTABLE"
fi

generate_histogram_png() {
    local DATA_FILE="$1"
    local OUTPUT_BASE="$2"
    local TITLE="$3"
    local YLABEL="$4"

    local SMALL_PNG="${OUTPUT_BASE}_small.png"
    local LARGE_PNG="${OUTPUT_BASE}_large.png"

    tail -n +2 "$DATA_FILE" | sort -t';' -k2 -n | head -n 50 > "${OUTPUT_DIR}/tmp_small.dat"
    tail -n +2 "$DATA_FILE" | sort -t';' -k2 -n -r | head -n 10 > "${OUTPUT_DIR}/tmp_large.dat"

    gnuplot <<-EOF
set terminal png size 1200,600
set output '${SMALL_PNG}'
set title '${TITLE} - 50 Smallest Plants'
set xlabel 'Plant Identifier'
set ylabel '${YLABEL}'
set style data histograms
set style fill solid 0.8
set boxwidth 0.8
set xtics rotate by -45 font ",8"
set datafile separator ";"
set key off
plot '${OUTPUT_DIR}/tmp_small.dat' using 2:xtic(1) with boxes lc rgb "#4169E1"
EOF

    gnuplot <<-EOF
set terminal png size 1200,600
set output '${LARGE_PNG}'
set title '${TITLE} - 10 Largest Plants'
set xlabel 'Plant Identifier'
set ylabel '${YLABEL}'
set style data histograms
set style fill solid 0.8
set boxwidth 0.8
set xtics rotate by -45 font ",10"
set datafile separator ";"
set key off
plot '${OUTPUT_DIR}/tmp_large.dat' using 2:xtic(1) with boxes lc rgb "#DC143C"
EOF

    rm -f "${OUTPUT_DIR}/tmp_small.dat" "${OUTPUT_DIR}/tmp_large.dat"

    echo "Generated: $SMALL_PNG"
    echo "Generated: $LARGE_PNG"
}

case "$COMMAND" in
    histo)
        if [ $# -lt 3 ]; then
            error_exit "histo command requires a mode: max, src, or real"
        fi
        if [ $# -gt 3 ]; then
            error_exit "Too many arguments for histo command"
        fi

        MODE="$3"
        case "$MODE" in
            max)
                OUTPUT_FILE="$OUTPUT_DIR/histo_max.dat"
                TITLE="Max Treatment Capacity"
                YLABEL="Volume (k.m3.year-1)"
                ;;
            src)
                OUTPUT_FILE="$OUTPUT_DIR/histo_src.dat"
                TITLE="Source Captured Volume"
                YLABEL="Volume (k.m3.year-1)"
                ;;
            real)
                OUTPUT_FILE="$OUTPUT_DIR/histo_real.dat"
                TITLE="Real Treated Volume"
                YLABEL="Volume (k.m3.year-1)"
                ;;
            *)
                error_exit "Invalid histo mode: $MODE. Use max, src, or real"
                ;;
        esac

        "$EXECUTABLE" "$DATA_FILE" "$OUTPUT_FILE" "$MODE"
        if [ $? -ne 0 ]; then
            error_exit "C program failed"
        fi

        echo "Data file generated: $OUTPUT_FILE"

        if command -v gnuplot &> /dev/null; then
            generate_histogram_png "$OUTPUT_FILE" "${OUTPUT_DIR}/histo_${MODE}" "$TITLE" "$YLABEL"
        else
            echo "Warning: gnuplot not found, skipping PNG generation"
        fi
        ;;

    leaks)
        if [ $# -lt 3 ]; then
            error_exit "leaks command requires a plant identifier"
        fi
        if [ $# -gt 3 ]; then
            error_exit "Too many arguments for leaks command"
        fi

        PLANT_ID="$3"
        OUTPUT_FILE="$OUTPUT_DIR/leaks_history.dat"

        "$EXECUTABLE" "$DATA_FILE" "$OUTPUT_FILE" "leaks" "$PLANT_ID"
        if [ $? -ne 0 ]; then
            error_exit "C program failed"
        fi

        echo "Leaks calculation completed for: $PLANT_ID"
        echo "Results appended to: $OUTPUT_FILE"
        tail -n 1 "$OUTPUT_FILE"
        ;;

    *)
        error_exit "Unknown command: $COMMAND. Use 'histo' or 'leaks'"
        ;;
esac

show_duration
exit 0
