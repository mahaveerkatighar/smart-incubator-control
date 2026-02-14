/*
 * ═══════════════════════════════════════════════════════════════════════
 *  SMART INCUBATOR CONTROL SYSTEM v5.1
 *  Single Temperature Sensor Edition
 * ═══════════════════════════════════════════════════════════════════════
 *  
 *  Authors: Mahaveer Katighar, S. Hema Vaishnavi, S.K Asifa, 
 *           Samyuga, Akshara Mikkilineni
 *  
 *  License: MIT
 *  Repository: https://github.com/mahaveerkatighar/smart-incubator-control
 * 
 * ═══════════════════════════════════════════════════════════════════════
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

// ═══════════════════════════════════════════════════════════════════════
// HARDWARE PIN DEFINITIONS
// ═══════════════════════════════════════════════════════════════════════

#define I2C_SDA 21
#define I2C_SCL 22
#define LCD_ADDRESS 0x27

#define ONE_WIRE_BUS 4

#define HEATER_RELAY 32
#define COOLER_RELAY 33

#define RED_LED 26
#define GREEN_LED 27
#define YELLOW_LED 14

#define BUZZER 13
#define MODE_BUTTON 35

// ═══════════════════════════════════════════════════════════════════════
// CONTROL PARAMETERS
// ═══════════════════════════════════════════════════════════════════════

float targetTemp = 36.5;

const float CRITICAL_LOW_TEMP = 34.0;
const float CRITICAL_HIGH_TEMP = 38.5;
const float MIN_SAFE_TEMP = 35.0;
const float MAX_SAFE_TEMP = 37.5;
const float TEMP_TOLERANCE = 0.5;

float Kp = 1.0;
float Ki = 0.1;
float Kd = 0.5;

// ═══════════════════════════════════════════════════════════════════════
// GLOBAL OBJECTS & VARIABLES
// ═══════════════════════════════════════════════════════════════════════

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

enum ControlMode {
  MODE_MANUAL,
  MODE_AUTO,
  MODE_SAFETY_SHUTDOWN
};
ControlMode currentMode = MODE_AUTO;
const char* modeNames[] = {"MANUAL", "AUTO", "SAFETY"};

float currentTemp = 0.0;
bool heaterState = false;
bool coolerState = false;

String systemStatus = "INITIALIZING";
String alertMessage = "";
bool alertActive = false;
bool sensorFault = false;
bool emergencyShutdown = false;
bool firebaseReady = false;

float pidIntegral = 0.0;
float pidPreviousError = 0.0;
unsigned long lastPIDTime = 0;

unsigned long totalRunTime = 0;
unsigned long dataPoints = 0;

unsigned long lastTempRead = 0;
unsigned long lastControlUpdate = 0;
unsigned long lastFirebaseUpdate = 0;
unsigned long lastFirebaseRead = 0;
unsigned long lastButtonCheck = 0;
unsigned long lastSerialPrint = 0;
unsigned long lastHistoryLog = 0;
unsigned long lastLCDUpdate = 0;
unsigned long startTime = 0;

const unsigned long TEMP_READ_INTERVAL = 1000;
const unsigned long CONTROL_UPDATE_INTERVAL = 2000;
const unsigned long FIREBASE_UPDATE_INTERVAL = 2000;
const unsigned long FIREBASE_READ_INTERVAL = 3000;
const unsigned long BUTTON_CHECK_INTERVAL = 100;
const unsigned long SERIAL_PRINT_INTERVAL = 5000;
const unsigned long HISTORY_LOG_INTERVAL = 60000;
const unsigned long LCD_UPDATE_INTERVAL = 1000;

int lcdDisplayMode = 0;
const int LCD_MODES = 3;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

byte degreeSymbol[8] = {
  0b00110, 0b01001, 0b01001, 0b00110,
  0b00000, 0b00000, 0b00000, 0b00000
};

byte heaterIcon[8] = {
  0b00000, 0b10101, 0b10101, 0b01010,
  0b01010, 0b10101, 0b10101, 0b00000
};

byte coolerIcon[8] = {
  0b00100, 0b01110, 0b11111, 0b01110,
  0b00100, 0b01110, 0b10101, 0b00000
};

// ═══════════════════════════════════════════════════════════════════════
// SETUP FUNCTION
// ═══════════════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  printBootScreen();
  initializePins();
  initializeLCD();
  initializeSensors();
  connectWiFi();
  syncTime();
  connectFirebase();
  
  startTime = millis();
  lastPIDTime = millis();
  
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  
  Serial.println("\n╔════════════════════════════════════════════════╗");
  Serial.println("║           SYSTEM READY - AUTO MODE             ║");
  Serial.println("╚════════════════════════════════════════════════╝\n");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM READY!");
  lcd.setCursor(0, 1);
  lcd.print("AUTO MODE");
  delay(2000);
}

// ═══════════════════════════════════════════════════════════════════════
// MAIN LOOP
// ═══════════════════════════════════════════════════════════════════════

void loop() {
  unsigned long currentMillis = millis();
  
  totalRunTime = currentMillis - startTime;
  
  if (currentMillis - lastTempRead >= TEMP_READ_INTERVAL) {
    lastTempRead = currentMillis;
    readTemperatureSensor();
  }
  
  analyzeSafety();
  
  if (currentMillis - lastControlUpdate >= CONTROL_UPDATE_INTERVAL) {
    lastControlUpdate = currentMillis;
    if (currentMode == MODE_AUTO && !emergencyShutdown) {
      performAutomaticControl();
    }
  }
  
  if (firebaseReady && currentMillis - lastFirebaseUpdate >= FIREBASE_UPDATE_INTERVAL) {
    lastFirebaseUpdate = currentMillis;
    uploadToFirebase();
  }
  
  if (firebaseReady && currentMillis - lastFirebaseRead >= FIREBASE_READ_INTERVAL) {
    lastFirebaseRead = currentMillis;
    checkRemoteCommands();
  }
  
  if (firebaseReady && currentMillis - lastHistoryLog >= HISTORY_LOG_INTERVAL) {
    lastHistoryLog = currentMillis;
    logHistoricalData();
  }
  
  if (currentMillis - lastButtonCheck >= BUTTON_CHECK_INTERVAL) {
    lastButtonCheck = currentMillis;
    checkModeButton();
  }
  
  if (currentMillis - lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
    lastLCDUpdate = currentMillis;
    updateLCD();
  }
  
  if (currentMillis - lastSerialPrint >= SERIAL_PRINT_INTERVAL) {
    lastSerialPrint = currentMillis;
    printStatus();
  }
  
  updateAlertOutputs();
}

// ═══════════════════════════════════════════════════════════════════════
// LCD FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════

void initializeLCD() {
  Serial.println("→ Initializing LCD...");
  
  lcd.begin();
  lcd.backlight();
  
  lcd.createChar(0, degreeSymbol);
  lcd.createChar(1, heaterIcon);
  lcd.createChar(2, coolerIcon);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INCUBATOR v5.1");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  Serial.println("  ✓ LCD initialized");
}

void updateLCD() {
  static unsigned long lastModeChange = 0;
  if (millis() - lastModeChange >= 5000) {
    lastModeChange = millis();
    lcdDisplayMode = (lcdDisplayMode + 1) % LCD_MODES;
  }
  
  lcd.clear();
  
  switch (lcdDisplayMode) {
    case 0:
      displayTemperatureScreen();
      break;
    case 1:
      displayControlScreen();
      break;
    case 2:
      displayStatusScreen();
      break;
  }
}

void displayTemperatureScreen() {
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print(currentTemp, 1);
  lcd.write(0);
  lcd.print("C");
  
  lcd.setCursor(13, 0);
  if (heaterState) {
    lcd.write(1);
  } else if (coolerState) {
    lcd.write(2);
  } else {
    lcd.print("   ");
  }
  
  lcd.setCursor(0, 1);
  lcd.print("Target:");
  lcd.print(targetTemp, 1);
  lcd.write(0);
  lcd.print("C");
}

void displayControlScreen() {
  lcd.setCursor(0, 0);
  lcd.print("Mode: ");
  lcd.print(modeNames[currentMode]);
  
  lcd.setCursor(0, 1);
  lcd.print("H:");
  lcd.print(heaterState ? "ON " : "OFF");
  lcd.print(" C:");
  lcd.print(coolerState ? "ON " : "OFF");
}

void displayStatusScreen() {
  lcd.setCursor(0, 0);
  if (systemStatus == "NORMAL") {
    lcd.print("Status: NORMAL");
  } else if (systemStatus == "EMERGENCY_SHUTDOWN") {
    lcd.print("EMERGENCY STOP!");
  } else if (systemStatus == "SENSOR_FAULT") {
    lcd.print("SENSOR FAULT!");
  } else {
    lcd.print("Alert: ");
    lcd.print(systemStatus.substring(0, 8));
  }
  
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(currentTemp, 1);
  lcd.write(0);
  lcd.print("C");
}

void displayCustomMessage(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// ═══════════════════════════════════════════════════════════════════════
// INITIALIZATION FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════

void printBootScreen() {
  Serial.println("\n\n╔═══════════════════════════════════════════════════════╗");
  Serial.println("║   SMART INCUBATOR CONTROL SYSTEM v5.1                ║");
  Serial.println("║   Authors: Mahaveer Katighar, S. Hema Vaishnavi,     ║");
  Serial.println("║            S.K Asifa, Samyuga, Akshara Mikkilineni   ║");
  Serial.println("╚═══════════════════════════════════════════════════════╝\n");
}

void initializePins() {
  Serial.println("→ Initializing GPIO...");
  
  pinMode(HEATER_RELAY, OUTPUT);
  pinMode(COOLER_RELAY, OUTPUT);
  digitalWrite(HEATER_RELAY, HIGH);
  digitalWrite(COOLER_RELAY, HIGH);
  
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  
  pinMode(MODE_BUTTON, INPUT_PULLUP);
  
  Serial.println("  ✓ GPIO initialized");
}

void initializeSensors() {
  Serial.println("→ Initializing sensors...");
  
  sensors.begin();
  int deviceCount = sensors.getDeviceCount();
  Serial.println("  ✓ DS18B20 sensors found: " + String(deviceCount));
  
  if (deviceCount < 1) {
    Serial.println("  ⚠ WARNING: No temperature sensor detected!");
  }
}

void connectWiFi() {
  Serial.println("→ Connecting to WiFi: " + String(WIFI_SSID));
  displayCustomMessage("WiFi Connect", WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("  ✓ WiFi Connected!");
    Serial.println("  ✓ IP: " + WiFi.localIP().toString());
    displayCustomMessage("WiFi Connected", WiFi.localIP().toString());
    delay(2000);
  } else {
    Serial.println("  ✗ WiFi Failed");
    displayCustomMessage("WiFi Failed", "Offline Mode");
    delay(2000);
  }
}

void syncTime() {
  Serial.println("→ Synchronizing time...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.println("  ✓ Time synced");
  } else {
    Serial.println("  ⚠ Time sync failed");
  }
}

void connectFirebase() {
  Serial.println("→ Connecting to Firebase...");
  displayCustomMessage("Firebase", "Connecting...");
  
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  int attempts = 0;
  while (!Firebase.ready() && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  Serial.println();
  
  if (Firebase.ready()) {
    Serial.println("  ✓ Firebase Connected!");
    firebaseReady = true;
    initializeFirebaseDevice();
    loadSettingsFromFirebase();
    displayCustomMessage("Firebase OK", "Cloud Ready");
    delay(2000);
  } else {
    Serial.println("  ✗ Firebase Failed");
    displayCustomMessage("Firebase Fail", "No Cloud");
    delay(2000);
  }
}

// ═══════════════════════════════════════════════════════════════════════
// SENSOR READING
// ═══════════════════════════════════════════════════════════════════════

void readTemperatureSensor() {
  sensors.requestTemperatures();
  currentTemp = sensors.getTempCByIndex(0);
  
  if (currentTemp != -127 && currentTemp > 0 && currentTemp < 50) {
    sensorFault = false;
  } else {
    sensorFault = true;
  }
}

// ═══════════════════════════════════════════════════════════════════════
// SAFETY ANALYSIS
// ═══════════════════════════════════════════════════════════════════════

void analyzeSafety() {
  alertActive = false;
  alertMessage = "";
  
  if (!sensorFault && (currentTemp < CRITICAL_LOW_TEMP || currentTemp > CRITICAL_HIGH_TEMP)) {
    emergencyShutdown = true;
    systemStatus = "EMERGENCY_SHUTDOWN";
    alertMessage = "CRITICAL TEMPERATURE";
    alertActive = true;
    
    setHeater(false);
    setCooler(false);
    return;
  }
  
  if (sensorFault) {
    emergencyShutdown = true;
    systemStatus = "SENSOR_FAULT";
    alertMessage = "Sensor malfunction";
    alertActive = true;
    return;
  }
  
  emergencyShutdown = false;
  
  if (currentTemp < MIN_SAFE_TEMP) {
    systemStatus = "TEMP_LOW";
    alertMessage = "Temperature below safe range";
    alertActive = true;
    return;
  }
  
  if (currentTemp > MAX_SAFE_TEMP) {
    systemStatus = "TEMP_HIGH";
    alertMessage = "Temperature above safe range";
    alertActive = true;
    return;
  }
  
  systemStatus = "NORMAL";
  alertMessage = "All parameters normal";
}

// ═══════════════════════════════════════════════════════════════════════
// AUTOMATIC CONTROL
// ═══════════════════════════════════════════════════════════════════════

void performAutomaticControl() {
  if (sensorFault) return;
  
  controlTemperature();
}

void controlTemperature() {
  float error = targetTemp - currentTemp;
  
  if (error > TEMP_TOLERANCE) {
    setHeater(true);
    setCooler(false);
  }
  else if (error < -TEMP_TOLERANCE) {
    setHeater(false);
    setCooler(true);
  }
  else {
    setHeater(false);
    setCooler(false);
  }
  
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - lastPIDTime) / 1000.0;
  
  if (deltaTime > 0) {
    pidIntegral += error * deltaTime;
    
    if (pidIntegral > 100) pidIntegral = 100;
    if (pidIntegral < -100) pidIntegral = -100;
    
    pidPreviousError = error;
    lastPIDTime = currentTime;
  }
}

// ═══════════════════════════════════════════════════════════════════════
// ACTUATOR CONTROL
// ═══════════════════════════════════════════════════════════════════════

void setHeater(bool state) {
  if (heaterState != state) {
    heaterState = state;
    digitalWrite(HEATER_RELAY, state ? LOW : HIGH);
  }
}

void setCooler(bool state) {
  if (coolerState != state) {
    coolerState = state;
    digitalWrite(COOLER_RELAY, state ? LOW : HIGH);
  }
}

// ═══════════════════════════════════════════════════════════════════════
// ALERT OUTPUTS
// ═══════════════════════════════════════════════════════════════════════

void updateAlertOutputs() {
  if (emergencyShutdown || sensorFault) {
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  }
  else if (alertActive) {
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  }
  else {
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }
  
  static unsigned long lastBuzzerToggle = 0;
  static bool buzzerState = false;
  
  if (alertActive) {
    if (millis() - lastBuzzerToggle >= 500) {
      lastBuzzerToggle = millis();
      buzzerState = !buzzerState;
      digitalWrite(BUZZER, buzzerState);
    }
  } else {
    digitalWrite(BUZZER, LOW);
  }
}

// ═══════════════════════════════════════════════════════════════════════
// MODE BUTTON
// ═══════════════════════════════════════════════════════════════════════

void checkModeButton() {
  static bool lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  bool buttonState = digitalRead(MODE_BUTTON);
  
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > 50) {
    if (buttonState == LOW && lastButtonState == HIGH) {
      if (currentMode == MODE_AUTO) {
        currentMode = MODE_MANUAL;
        pidIntegral = 0;
        Serial.println("→ Mode: MANUAL");
        displayCustomMessage("MODE:", "MANUAL");
        delay(1000);
      } else {
        currentMode = MODE_AUTO;
        Serial.println("→ Mode: AUTO");
        displayCustomMessage("MODE:", "AUTO");
        delay(1000);
      }
    }
  }
  
  lastButtonState = buttonState;
}

// ═══════════════════════════════════════════════════════════════════════
// FIREBASE FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════

void initializeFirebaseDevice() {
  String path = String(HOSPITAL_NAME) + "/devices/" + DEVICE_ID + "/info";
  
  FirebaseJson json;
  json.set("deviceId", DEVICE_ID);
  json.set("hospitalName", HOSPITAL_NAME);
  json.set("unit", NICU_UNIT);
  json.set("firmwareVersion", "5.1");
  json.set("ipAddress", WiFi.localIP().toString());
  
  Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json);
}

void loadSettingsFromFirebase() {
  String path = String(HOSPITAL_NAME) + "/devices/" + DEVICE_ID + "/settings";
  
  if (Firebase.RTDB.getFloat(&fbdo, (path + "/targetTemp").c_str())) {
    targetTemp = fbdo.floatData();
  }
  
  Serial.println("→ Settings loaded: Temp=" + String(targetTemp, 1) + "°C");
}

void uploadToFirebase() {
  if (!Firebase.ready()) return;
  
  String path = String(HOSPITAL_NAME) + "/devices/" + DEVICE_ID + "/current";
  
  FirebaseJson json;
  json.set("currentTemp", currentTemp);
  json.set("targetTemp", targetTemp);
  json.set("status", systemStatus);
  json.set("alertActive", alertActive);
  json.set("alertMessage", alertMessage);
  json.set("mode", modeNames[currentMode]);
  json.set("heaterState", heaterState);
  json.set("coolerState", coolerState);
  json.set("timestamp", getTimestamp());
  json.set("uptime", totalRunTime / 1000);
  
  Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json);
  dataPoints++;
}

void logHistoricalData() {
  String path = String(HOSPITAL_NAME) + "/devices/" + DEVICE_ID + "/history/" + getTimestamp();
  
  FirebaseJson json;
  json.set("currentTemp", currentTemp);
  json.set("targetTemp", targetTemp);
  json.set("heater", heaterState);
  json.set("cooler", coolerState);
  json.set("status", systemStatus);
  
  Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json);
}

void checkRemoteCommands() {
  String path = String(HOSPITAL_NAME) + "/devices/" + DEVICE_ID + "/commands";
  
  if (Firebase.RTDB.getJSON(&fbdo, path.c_str())) {
    FirebaseJson &json = fbdo.jsonObject();
    FirebaseJsonData result;
    
    if (json.get(result, "setTargetTemp")) {
      targetTemp = result.floatValue;
      displayCustomMessage("Remote Update", "New Target Set");
      delay(1500);
    }
    
    if (json.get(result, "setMode")) {
      String mode = result.stringValue;
      if (mode == "AUTO") {
        currentMode = MODE_AUTO;
        pidIntegral = 0;
      } else if (mode == "MANUAL") {
        currentMode = MODE_MANUAL;
      }
    }
    
    if (currentMode == MODE_MANUAL) {
      if (json.get(result, "heater")) {
        setHeater(result.boolValue);
      }
      if (json.get(result, "cooler")) {
        setCooler(result.boolValue);
      }
    }
    
    if (json.get(result, "emergencyStop")) {
      if (result.boolValue) {
        setHeater(false);
        setCooler(false);
        emergencyShutdown = true;
        displayCustomMessage("EMERGENCY", "STOP!");
      }
    }
    
    Firebase.RTDB.deleteNode(&fbdo, path.c_str());
  }
}

// ═══════════════════════════════════════════════════════════════════════
// UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════

String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return String(millis());
  }
  
  char timestamp[25];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", &timeinfo);
  return String(timestamp);
}

void printStatus() {
  Serial.println("\n╔════════════════════════════════════════════════╗");
  Serial.printf("║ Temp: %5.2f°C | Target: %5.2f°C              ║\n", currentTemp, targetTemp);
  Serial.printf("║ Mode: %-12s | Status: %-18s ║\n", modeNames[currentMode], systemStatus.c_str());
  Serial.printf("║ Heater: %-3s | Cooler: %-3s                    ║\n",
                heaterState ? "ON" : "OFF",
                coolerState ? "ON" : "OFF");
  Serial.println("╚════════════════════════════════════════════════╝");
}
