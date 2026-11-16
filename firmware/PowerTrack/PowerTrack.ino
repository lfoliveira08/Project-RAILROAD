#include <Arduino.h>
#include <WiFi.h>
#include <ModbusIP_ESP8266.h>

#define WLAN_SSID       "YOUR_WIFI_SSID"
#define WLAN_PASS       "YOUR_WIFI_PASSWORD"

// Static IP 
IPAddress LOCAL_IP(192, 0, 2, 14);
IPAddress GATEWAY(192, 0, 2, 1);
IPAddress SUBNET(255, 255, 255, 0);

// Pins
const int RELAY_PIN = 15;
const int buttonPin = 13;
const int switchPin = 14;
const int potPin = 32;

// State
bool motorActive = false;
bool lastButtonState = HIGH;

// Modbus
ModbusIP modbus;

void setup() {
  Serial.begin(115200);
  delay(200);

  // Pins
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // WiFi
  WiFi.config(LOCAL_IP, GATEWAY, SUBNET);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Modbus TCP server
  modbus.server();
  modbus.addCoil(0, false);    // Track power command (TRUE=on, FALSE=off)
  modbus.addIsts(0, false);    // Track power status feedback
  modbus.addHreg(0, 0);        // Reserved for future speed control
  modbus.addIreg(0, 0);        // Reports potentiometer value

  Serial.println("Track Power Modbus slave ready on port 502");
}

void loop() {
  modbus.task();

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(1000);
    return;
  }

  // --- LOCAL CONTROL (button still works) ---
  bool currentButtonState = digitalRead(buttonPin);
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    motorActive = !motorActive;
    // Sync local button state to Modbus coil
    modbus.Coil(0, motorActive);
    delay(50);
  }
  lastButtonState = currentButtonState;

  // --- REMOTE CONTROL (OpenPLC overrides) ---
  bool modbusCmd = modbus.Coil(0);
  motorActive = modbusCmd;

  // --- READ POTENTIOMETER ---
  int potValue = analogRead(potPin);
  int targetSpeed = map(potValue, 0, 4095, 0, 100);
  modbus.Ireg(0, targetSpeed);  // Report pot value to OpenPLC

  // --- RELAY CONTROL ---
  if (motorActive && targetSpeed > 10) {
    digitalWrite(RELAY_PIN, HIGH);
    modbus.Ists(0, true);       // Tell OpenPLC: track power is ON
  } else {
    digitalWrite(RELAY_PIN, LOW);
    modbus.Ists(0, false);      // Tell OpenPLC: track power is OFF
  }
}
