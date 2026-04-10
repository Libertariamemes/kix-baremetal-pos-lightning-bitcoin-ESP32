#!/bin/bash
# --- ARCH LOCKDOWN v4: THE OUT-OF-SIGHT MANEUVER ---
PORT="/dev/ttyACM0"
BOARD="esp32:esp32:esp32"
KIX_DIR="/home/zeloko/kix_esp32_mxlinux/kix_fresh"
GLOBAL_LIB="/home/zeloko/Arduino/libraries/TFT_eSPI"
TEMP_HIDE="/tmp/TFT_eSPI_HIDDEN"

echo "[1/4] Moving global library to /tmp (Complete Isolation)..."
[ -d "$GLOBAL_LIB" ] && mv "$GLOBAL_LIB" "$TEMP_HIDE"
[ -d "${GLOBAL_LIB}_BAK" ] && mv "${GLOBAL_LIB}_BAK" "$TEMP_HIDE"

echo "[2/4] Compiling..."
# We use --libraries pointing ONLY to your local KIX lib
arduino-cli compile --fqbn $BOARD \
    --libraries "$KIX_DIR/TFT_KIX" \
    --verbose \
    --jobs 12 \
    "$KIX_DIR"

COMPILE_STATUS=$?

echo "[3/4] Restoring global library..."
[ -d "$TEMP_HIDE" ] && mv "$TEMP_HIDE" "$GLOBAL_LIB"

if [ $COMPILE_STATUS -eq 0 ]; then
    echo "[4/4] Uploading..."
    arduino-cli upload -p $PORT --fqbn $BOARD "$KIX_DIR"
    arduino-cli monitor -p $PORT --config baudrate=115200
else
    echo "!!! COMPILE FAILED !!!"
fi
