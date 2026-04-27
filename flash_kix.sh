#!/bin/bash
# --- ZLV BAREMETAL: QR-CODE INTEGRATION ---

PORT="/dev/ttyACM0"
BOARD="esp32:esp32:esp32"

echo "[1/2] Compilando com suporte a QR Code (4 núcleos)..."

# O ponto (.) indica que estamos compilando a pasta atual do repositório
arduino-cli compile --fqbn $BOARD \
    --build-property "compiler.cpp.extra_flags=-fpermissive" \
    --library "/home/Mxlinux/Arduino/libraries/QRCode" \
    --jobs 4 \
    --verbose .

if [ $? -eq 0 ]; then
    echo "[2/2] Compilação OK! Subindo para o hardware..."
    arduino-cli upload -p $PORT --fqbn $BOARD .
    echo "--- Monitor Serial (115200) ---"
    arduino-cli monitor -p $PORT --config baudrate=115200
else
    echo "!!! Erro na compilação. Verifique se o #include <qrcode.h> está correto."
    exit 1
fi
