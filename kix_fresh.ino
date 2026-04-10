#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include "TFT_KIX/TFT_eSPI.h"

/* --- EMBEDDED KEYPAD LOGIC --- */
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'}, {'4','5','6','B'},
  {'7','8','9','C'}, {'*','0','#','D'}
};
byte rowPins[ROWS] = {33, 25, 26, 27}; 
byte colPins[COLS] = {15, 13, 12, 2}; 

/* --- CONFIGURATION --- */
const char* ssid = "TP-Link_46C2";
const char* password = "80165897";
String lnbits_url = "http://34.226.196.147:5004/api/v1/payments";
String lnbits_api_key = "2b273ca3d3a84db4a3857ff54776aa96";

/* --- GLOBAL STATE --- */
String current_payment_hash = "";
String input_amount = "";
TFT_eSPI tft = TFT_eSPI();
unsigned long lastKeyTime = 0;

void kickstartHardware() {
  pinMode(4, OUTPUT); digitalWrite(4, HIGH); 
  pinMode(23, OUTPUT); digitalWrite(23, HIGH); 
}

void logDisplay(String msg, uint16_t color = 0xFFFF) {
  tft.fillRect(0, 110, 240, 25, 0x0000); 
  tft.setCursor(0, 115);
  tft.setTextColor(color, 0x0000);
  tft.setTextSize(2);
  tft.println("> " + msg);
  Serial.println("[LOG] " + msg);
}

void displayInput() {
  tft.fillRect(0, 40, 240, 60, 0x0000);
  tft.setCursor(10, 50);
  tft.setTextColor(0xF81F, 0x0000); // Pink/Magenta for SATS
  tft.setTextSize(5); 
  tft.print(input_amount);
  tft.setTextSize(2);
  tft.print(" SATS");
}

char getCustomKey() {
  if (millis() - lastKeyTime < 250) return 0;
  for (int r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT); digitalWrite(rowPins[r], LOW);
    for (int c = 0; c < COLS; c++) {
      pinMode(colPins[c], INPUT_PULLUP);
      if (digitalRead(colPins[c]) == LOW) {
        lastKeyTime = millis(); pinMode(rowPins[r], INPUT); return keys[r][c];
      }
    }
    pinMode(rowPins[r], INPUT);
  }
  return 0;
}

void connectWiFi() {
  input_amount = ""; 
  tft.fillScreen(0x0000);
  tft.setTextColor(0xF81F, 0x0000); // Pink Title
  tft.setTextSize(3);
  tft.setCursor(0, 10);
  tft.println("PROJECT KIX");
  
  if(WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); kickstartHardware(); }
  }
  
  logDisplay("ONLINE", 0x07E0); 
  displayInput();
}

void checkBalance() {
  logDisplay("FETCHING...", 0xFFE0);
  HTTPClient http;
  http.begin("http://34.226.196.147:5004/api/v1/wallet");
  http.addHeader("X-Api-Key", lnbits_api_key);
  int code = http.GET();
  if (code == 200) {
    String raw = http.getString();
    Serial.println("[RAW_WALLET]: " + raw);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, raw);
    long balance = doc["balance_msat"].as<long>() / 1000;
    
    tft.fillScreen(0x0000);
    tft.setCursor(0, 10); tft.setTextColor(0xF81F); tft.setTextSize(2);
    tft.println("WALLET BALANCE:");
    tft.setCursor(0, 50); tft.setTextColor(0xFFFF); tft.setTextSize(4);
    tft.print(balance);
    tft.setTextSize(2); tft.println(" SATS");
    delay(5000);
    connectWiFi();
  } else {
    logDisplay("BAL_ERR: " + String(code), 0xF800);
  }
  http.end();
}

void createInvoice(int sats) {
  logDisplay("ORDERING...", 0x07FF);
  HTTPClient http;
  http.begin(lnbits_url);
  http.addHeader("X-Api-Key", lnbits_api_key);
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"out\": false, \"amount\":" + String(sats) + ", \"memo\": \"KIX-POS\"}";
  
  int httpCode = http.POST(payload);
  String response = http.getString();
  
  // RAW Spill to Serial as requested
  Serial.println("[RAW_INVOICE_RESPONSE]: " + response);

  if (httpCode == 200 || httpCode == 201) {
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, response);
    String bolt11 = doc["payment_request"].as<String>();
    current_payment_hash = doc["payment_hash"].as<String>();
    
    tft.fillScreen(0x0000);
    tft.setCursor(0, 0); tft.setTextColor(0x07FF); tft.setTextSize(1);
    tft.println("INVOICE (BOLT11):");
    tft.setTextColor(0xFFFF);
    tft.println(bolt11); // Show raw bolt11 on display
    
    tft.setCursor(0, 110); tft.setTextColor(0xF81F); tft.setTextSize(2);
    tft.println("Press 'C' to Verify");
  } else {
    // Spill error to display for debugging
    tft.fillScreen(0x0000);
    tft.setCursor(0, 0); tft.setTextColor(0xF800); tft.setTextSize(1);
    tft.println("HTTP ERROR: " + String(httpCode));
    tft.println("SERVER MSG:");
    tft.println(response);
    logDisplay("INV_ERROR", 0xF800);
  }
  http.end();
  kickstartHardware();
}

void manualCheck() {
  if (current_payment_hash == "") { logDisplay("NO INVOICE", 0xF800); return; }
  logDisplay("CHECKING...", 0xFFFF);
  HTTPClient http;
  http.begin(lnbits_url + "/" + current_payment_hash);
  http.addHeader("X-Api-Key", lnbits_api_key);
  int code = http.GET();
  if (code == 200) {
    String raw = http.getString();
    Serial.println("[RAW_STATUS]: " + raw);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, raw);
    if (doc["paid"].as<bool>()) {
      tft.fillScreen(0x07E0); tft.setTextColor(0x0000);
      tft.setCursor(20, 50); tft.setTextSize(4);
      tft.println("PAID OK!"); delay(5000); connectWiFi();
    } else { 
      logDisplay("NOT PAID YET", 0xF800); 
    }
  } else {
    logDisplay("CHECK_ERR: " + String(code), 0xF800);
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  kickstartHardware();
  tft.init(); tft.setRotation(1); tft.setTextWrap(true); tft.fillScreen(0x0000);
  connectWiFi();
}

void loop() {
  char key = getCustomKey();
  if (key) {
    if (key >= '0' && key <= '9') { input_amount += key; displayInput(); }
    else if (key == 'A') checkBalance();
    else if (key == 'C') manualCheck();
    else if (key == 'D') connectWiFi(); // Reset/Clear to Home
    else if (key == '*') { input_amount = ""; displayInput(); }
    else if (key == '#') { createInvoice(input_amount.toInt()); input_amount = ""; }
  }
  
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); input.trim();
    if (input.startsWith("I:")) { createInvoice(input.substring(2).toInt()); }
    else if (input.equalsIgnoreCase("C:")) manualCheck();
    else if (input.equalsIgnoreCase("L:")) checkBalance();
    else if (input.equalsIgnoreCase("H:")) connectWiFi();
  }
}
