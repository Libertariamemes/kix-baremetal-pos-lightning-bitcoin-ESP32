#!/bin/bash
# --- ZLV BAREMETAL: DEPLOY FINAL ---

PORT="/dev/ttyACM0"
BOARD="esp32:esp32:esp32"

echo "[1/2] Compilando KIX com 4 núcleos..."

# Usamos o diretório atual (.) e as flags de compatibilidade
arduino-cli compile --fqbn $BOARD \
    --build-property "compiler.cpp.extra_flags=-fpermissive" \
    --jobs 4 \
    --verbose .

if [ $? -eq 0 ]; then
    echo "[2/2] Compilação perfeita. Subindo para o hardware..."
    arduino-cli upload -p $PORT --fqbn $BOARD .
    arduino-cli monitor -p $PORT --config baudrate=115200
else
    echo "!!! Erro na compilação !!!"
    exit 1
fi
