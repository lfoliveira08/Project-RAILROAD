#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include <ESP32Servo.h>

#define WLAN_SSID       "YOUR_WIFI_SSID"
#define WLAN_PASS       "YOUR_WIFI_PASSWORD"

#define SERVO_PIN       13
#define STRAIGHT_ANGLE  0
#define DIVERGE_ANGLE   180

// LCD: 16 columns, 2 rows, address 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

ModbusIP modbus;
Servo switchServo;
int currentAngle = STRAIGHT_ANGLE;
int targetAngle  = STRAIGHT_ANGLE;
unsigned long lastLcdUpdate = 0;

void setup() {
  Serial.begin(115200);
  delay(200);

  // LCD
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Track Switch v1");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  // Servo
  switchServo.attach(SERVO_PIN);
  switchServo.write(currentAngle);
  delay(500);

  // WiFi
  lcd.setCursor(0, 1);
  lcd.print("Connecting WiFi ");
  WiFi.config(IPAddress(192, 0, 2, 12), // SwitchTrack 1
              IPAddress(192, 0, 2, 1),
              IPAddress(255, 255, 255, 0));
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi connected!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000);

  // Modbus TCP server
  modbus.server();
  modbus.addCoil(0, false);
  modbus.addIsts(0, true);
  modbus.addHreg(0, STRAIGHT_ANGLE);
  modbus.addIreg(0, STRAIGHT_ANGLE);

  Serial.println("Modbus TCP slave ready on port 502");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modbus Ready:502");
  lcd.setCursor(0, 1);
  lcd.print("STRAIGHT   0deg");
}

void loop() {
  modbus.task();

  if (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("WiFi LOST!      ");
    lcd.setCursor(0, 1);
    lcd.print("Reconnecting... ");
    WiFi.reconnect();
    delay(1000);
    return;
  }

  // Read command from OpenPLC
  bool switchCmd = modbus.Coil(0);
  targetAngle = switchCmd ? DIVERGE_ANGLE : STRAIGHT_ANGLE;
  modbus.Hreg(0, targetAngle);

  // Instant snap servo movement â€” maximum speed and torque
  if (currentAngle != targetAngle) {
    modbus.Ists(0, false);
    currentAngle = targetAngle;
    switchServo.write(currentAngle);
    modbus.Ists(0, true);
    Serial.print("Switch: ");
    Serial.println(switchCmd ? "DIVERGE" : "STRAIGHT");
  }

  modbus.Ireg(0, currentAngle);

  // Update LCD every 250ms
  if (millis() - lastLcdUpdate > 250) {
    lastLcdUpdate = millis();

    // Line 1: Switch status
    lcd.setCursor(0, 0);
    if (switchCmd) {
      lcd.print("DIVERGE         ");
    } else {
      lcd.print("STRAIGHT        ");
    }

    // Line 2: Angle and connection
    lcd.setCursor(0, 1);
    lcd.print("Ang:");
    if (currentAngle < 100) lcd.print(" ");
    if (currentAngle < 10) lcd.print(" ");
    lcd.print(currentAngle);
    lcd.setCursor(10, 1);
    lcd.print("MB:OK ");
  }
}

