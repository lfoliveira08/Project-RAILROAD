/*
 * ============================================================
 * ESP32 #1 â€” Crossing Gate (Obstacle Avoidance IR)
 * ============================================================
 * Project: Cybersecurity of Digitized Railroad Track Control
 * Role:    Crossing gate slave device
 * Master:  Raspberry Pi 4B running OpenPLC Runtime
 * Comm:    Modbus TCP over WiFi (port 502)
 *
 * Behavior:
 *   - Default state: gate UP, LED OFF
 *   - OpenPLC sends ON (Coil 0 = TRUE)  â†’ system activates
 *   - OpenPLC sends OFF (Coil 0 = FALSE) â†’ gate UP, LED OFF
 *   - IR Sensor 1 detects train â†’ LEDs blink + gate DOWN
 *   - IR Sensor 2 detects train cleared â†’ gate UP for 1 second
 *   - After 1 second: gate stays UP, LED OFF, ready for next train
 *
 * Obstacle IR sensor: LOW = detected, HIGH = clear
 *
 * State Machine:
 *   IDLE        â†’ gate UP, LED OFF, waiting (Sensor 1 watched)
 *   GATE_CLOSED â†’ LEDs blinking, gate DOWN (Sensor 2 watched)
 *   GATE_OPENING â†’ gate UP, LEDs stop after 1 second
 *
 * Modbus Register Map:
 *   Coil 0        â†’ System enable (OpenPLC â†’ ESP32)
 *   Discrete In 0 â†’ Gate status (false=up, true=down)
 *   Discrete In 1 â†’ Train approaching flag
 *   Discrete In 2 â†’ Train cleared flag
 * ============================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include <ESP32Servo.h>

// â”€â”€ WiFi credentials â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// â”€â”€ Static IP â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
#define STATIC_IP    192, 0, 2, 15
#define GATEWAY_IP   192, 0, 2, 1
#define SUBNET_MASK  255, 255, 255, 0

// â”€â”€ Pins â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
#define IR_SENSOR_1    35    // Train approaching (swapped)
#define IR_SENSOR_2    34    // Train cleared (swapped)
#define LED_RED        26    // RGB LED 1 red pin
#define LED_RED_2      19    // RGB LED 2 red pin
#define SERVO_PIN      25    // Gate servo 1
#define SERVO_PIN_2    21    // Gate servo 2

// â”€â”€ Servo 1 positions â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
#define GATE_UP        90
#define GATE_DOWN      0

// â”€â”€ Servo 2 positions (inverted) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
#define GATE_UP_2      0
#define GATE_DOWN_2    90

// â”€â”€ Blink timing â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
#define BLINK_INTERVAL 300   // ms between blinks

// â”€â”€ Modbus register addresses â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
#define COIL_ENABLE       0
#define ISTS_GATE_DOWN    0
#define ISTS_TRAIN_HERE   1
#define ISTS_TRAIN_CLEAR  2

// â”€â”€ State machine â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
enum GateState { IDLE, GATE_CLOSED, GATE_OPENING };
GateState gateState = IDLE;

// â”€â”€ Globals â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
ModbusIP modbus;
Servo gateServo;
Servo gateServo2;

bool ledBlinking  = false;
bool ledState     = false;
unsigned long lastBlink      = 0;
unsigned long gateOpenStart  = 0;

// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
void setLED(bool on) {
  digitalWrite(LED_RED, on ? HIGH : LOW);
  digitalWrite(LED_RED_2, on ? HIGH : LOW);
  ledState = on;
}

void stopBlinking() {
  ledBlinking = false;
  setLED(false);
}

void handleBlink() {
  if (!ledBlinking) return;
  unsigned long now = millis();
  if (now - lastBlink >= BLINK_INTERVAL) {
    lastBlink = now;
    ledState = !ledState;
    digitalWrite(LED_RED, ledState ? HIGH : LOW);
    digitalWrite(LED_RED_2, ledState ? HIGH : LOW);
  }
}

// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("=== Crossing Gate ESP32 #1 (Obstacle IR) ===");

  pinMode(IR_SENSOR_1, INPUT);
  pinMode(IR_SENSOR_2, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_RED_2, OUTPUT);
  setLED(false);

  // Gates UP by default
  gateServo.attach(SERVO_PIN);
  gateServo2.attach(SERVO_PIN_2);
  gateServo.write(GATE_UP);
  gateServo2.write(GATE_UP_2);
  Serial.println("Gates: UP â€” default position");

  WiFi.config(IPAddress(STATIC_IP),
              IPAddress(GATEWAY_IP),
              IPAddress(SUBNET_MASK));
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected! IP: ");
  Serial.println(WiFi.localIP());

  modbus.server();
  modbus.addCoil(COIL_ENABLE,      false);
  modbus.addIsts(ISTS_GATE_DOWN,   false);
  modbus.addIsts(ISTS_TRAIN_HERE,  false);
  modbus.addIsts(ISTS_TRAIN_CLEAR, false);
  Serial.println("Modbus TCP slave ready on port 502");
  Serial.println("Waiting for OpenPLC enable command...");
}

// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
void loop() {
  modbus.task();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost! Reconnecting...");
    WiFi.reconnect();
    delay(1000);
    return;
  }

  bool systemEnabled = modbus.Coil(COIL_ENABLE);

  if (!systemEnabled) {
    // System OFF â€” gates UP, LEDs OFF
    stopBlinking();
    gateServo.write(GATE_UP);
    gateServo2.write(GATE_UP_2);
    gateState = IDLE;
    modbus.Ists(ISTS_GATE_DOWN,   false);
    modbus.Ists(ISTS_TRAIN_HERE,  false);
    modbus.Ists(ISTS_TRAIN_CLEAR, false);
    delay(200);
    return;
  }

  bool trainApproaching = (digitalRead(IR_SENSOR_1) == LOW);
  bool trainCleared     = (digitalRead(IR_SENSOR_2) == LOW);

  modbus.Ists(ISTS_TRAIN_HERE,  trainApproaching);
  modbus.Ists(ISTS_TRAIN_CLEAR, trainCleared);

  switch (gateState) {

    case IDLE:
      // Gates UP, LEDs OFF â€” waiting for train
      if (trainApproaching) {
        Serial.println(">>> Train approaching! Gates closing, LEDs blinking...");
        ledBlinking = true;
        gateServo.write(GATE_DOWN);
        gateServo2.write(GATE_DOWN_2);
        gateState = GATE_CLOSED;
        modbus.Ists(ISTS_GATE_DOWN, true);
        Serial.println("State: IDLE â†’ GATE_CLOSED");
      }
      break;

    case GATE_CLOSED:
      // Gates DOWN, LEDs blinking â€” train crossing
      if (trainCleared) {
        Serial.println(">>> Train cleared! Gates opening...");
        gateServo.write(GATE_UP);
        gateServo2.write(GATE_UP_2);
        gateOpenStart = millis();
        gateState = GATE_OPENING;
        modbus.Ists(ISTS_GATE_DOWN, false);
        Serial.println("State: GATE_CLOSED â†’ GATE_OPENING");
      }
      break;

    case GATE_OPENING:
      // Gates UP â€” stop blinking after 1 second
      if (millis() - gateOpenStart >= 1000) {
        stopBlinking();
        gateState = IDLE;
        Serial.println(">>> Gates UP. Ready for next train.");
        Serial.println("State: GATE_OPENING â†’ IDLE");
      }
      break;

    default:
      gateState = IDLE;
      break;
  }

  handleBlink();
}

