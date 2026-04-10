# ⚡ KIX Baremetal POS

A Sovereign Bitcoin Lightning Point-of-Sale for ESP32

---

## 🧠 Overview

KIX Baremetal is a minimalist, high-speed payment terminal designed for merchants who value stability and self-sovereignty.

By removing heavy UI frameworks like LVGL, this version provides a "Hardened Core" that is easy to audit, fast to boot, and incredibly reliable for real-world commerce.

---

## 🛠 Features

- **Zero Overhead**  
  No heavy graphics libraries. Pure `TFT_eSPI` for maximum performance.

- **LNbits Native**  
  Seamless integration with any LNbits instance (SaaS or Self-Hosted).

- **Hybrid Control**  
  Operate via a physical 4x4 matrix keypad or through headless serial commands.

- **Hardened Logic**  
  BOLT11 invoice generation and manual payment verification built-in.

- **Sovereign Diagnostics**  
  Includes wallet balance checking (Key `A`) to verify node connectivity instantly.

---

## 📟 Hardware Requirements

- **Controller:** ESP32 (WROOM / WROVER)  
- **Display:** ST7789 240x240 TFT (SPI)  
- **Keypad:** 4x4 Membrane or Button Matrix  

### 🔌 Pin Configuration

**TFT**
- CS: 5  
- DC: 16  
- RST: 23  
- BL: 4  
- MOSI: 23  
- SCLK: 18  

**Keypad Rows**
- 33, 25, 26, 27  

**Keypad Cols**
- 15, 13, 12, 2  

---

## 🚀 Quick Start (Arch Linux / Arduino CLI)

This project is optimized for a terminal-centric workflow.

### Compile

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 \
  --build-property "build.partitions=huge_app" \
  --build-property "compiler.cpp.extra_flags=-fpermissive" \
  --build-property "compiler.c.extra_flags=-fpermissive -Wno-implicit-function-declaration" \
  .
```

### Upload

```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 .
```

---

## ⌨️ Keypad Mapping

| Key  | Action                          |
|------|---------------------------------|
| 0–9  | Input SATS amount              |
| #    | Generate Invoice (BOLT11)      |
| C    | Verify Payment (Check Status)  |
| A    | Check Wallet Balance           |
| D    | Reset / Back to Home           |
| *    | Clear current input            |

---

## ⚙️ Configuration

Open `kix_fresh.ino` and update:

- `ssid` / `password` → Your local network  
- `lnbits_url` → Your LNbits instance  
  (e.g. https://legend.lnbits.com/api/v1/payments)  
- `lnbits_api_key` → Your Invoice/Wallet Key  

---

## 🛡 License & Philosophy

Project KIX is open-source software.

Built on the belief that tools for financial sovereignty should be:
- Accessible  
- Transparent  
- Impossible to shut down  

---

**Built by Zeloko Voider**  
Part of the KIX Sovereignty Framework
