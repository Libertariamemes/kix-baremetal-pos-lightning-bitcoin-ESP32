#!/bin/bash

# --- CONFIGURATION ---
PORT="/dev/ttyACM0"
BOARD="esp32:esp32:esp32"
LIB_PATH="./TFT_KIX"
SKETCH_DIR="."

echo "-----------------------------------------------"
echo "  KIX SOVEREIGN FLASH - VERBOSE & EXPLICIT     "
echo "-----------------------------------------------"

# 1. Clean old build artifacts to force fresh header inclusion
echo "[1/4] Cleaning build cache..."
rm -rf ./build
rm -rf ./build_output

# 2. Compile with explicit library priority and verbosity
echo "[2/4] Compiling with local library: $LIB_PATH"
arduino-cli compile --fqbn $BOARD \
    --library $LIB_PATH \
    --clean \
    --verbose \
    --jobs 4 \
    $SKETCH_DIR

# Check if compile succeeded
if [ $? -ne 0 ]; then
    echo "!!! COMPILE FAILED !!! Check User_Setup_KIX.h paths."
    exit 1
fi

# 3. Upload to the ESP32
echo "[3/4] Uploading to $PORT..."
arduino-cli upload -p $PORT --fqbn $BOARD $SKETCH_DIR

if [ $? -ne 0 ]; then
    echo "!!! UPLOAD FAILED !!! Is the board in Bootloader mode?"
    exit 1
fi

# 4. Launch Serial Monitor
echo "[4/4] Launching Monitor (CTRL+C to exit)..."
arduino-cli monitor -p $PORT -c baudrate=115200
