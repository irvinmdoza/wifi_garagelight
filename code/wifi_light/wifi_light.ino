#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

// EEPROM Size for Wi-Fi credentials
#define EEPROM_SIZE 96

// MQTT Broker
const char* server = "192.168.86.88"; // Replace with your laptop's IP running Mosquitto
const char* reset_topic = "device/reset";
const char* distance_topic = "sensor/distance";
const char* reed_switch_topic = "sensor/reed";

// Get MAC Address for unique client ID
String macAddr = WiFi.macAddress();
String host = "arduino-LED-" + macAddr.substring(12); // Shorten to avoid long names

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Initialize LED constants and blink mode flag
const int RED_LED = D5;
const int YELLOW_LED = D6;
const int GREEN_LED = D7;
bool blinkMode = false;
bool blinkModeDC = false;
String state = "off";
bool reedSwitchState = false;
char ssid[32] = "";
char pass[64] = "";

void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println("Initializing...");

  // Set Pins
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);  // Built-in LED
  allOff();

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, ssid);
  EEPROM.get(32, pass);

  // Check for stored credentials
  if (strlen(ssid) > 0) {
    Serial.println("Connecting to Wi-Fi: " + String(ssid));
    WiFi.hostname(host.c_str());
    WiFi.begin(ssid, pass);

    int timeout = 15;
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
      digitalWrite(LED_BUILTIN, LOW);  // ON (Active Low)
      delay(250);
      digitalWrite(LED_BUILTIN, HIGH); // OFF
      delay(250);
      Serial.print(".");
      timeout--;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected: " + String(ssid));
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
      startMQTT();
      return;
    } else {
      Serial.print("\nFailed to connect.");
    }
  } else {
    Serial.print("No Wi-Fi credentials stored.");
  }

  Serial.println(" Enter Credentials");
  requestWiFiCredentials();
}

void saveWiFiCredentials(const char *newSSID, const char *newPassword) {
    Serial.println("Saving WiFi credentials...");
    strncpy(ssid, newSSID, sizeof(ssid));
    strncpy(pass, newPassword, sizeof(pass));
    EEPROM.put(0, ssid);
    EEPROM.put(32, pass);
    EEPROM.commit();
    Serial.println("Saved! Rebooting...");
    delay(2000);
    ESP.restart();
}

void requestWiFiCredentials() {
    Serial.println("Enter SSID:");
    while (Serial.available() == 0) {
        delay(100);
    }
    String newSSID = Serial.readStringUntil('\n');
    newSSID.trim();

    Serial.println("Enter Password:");
    while (Serial.available() == 0) {
        delay(100);
    }
    String newPassword = Serial.readStringUntil('\n');
    newPassword.trim();

    saveWiFiCredentials(newSSID.c_str(), newPassword.c_str());
}

void startMQTT() {
    Serial.println("Starting normal operation...");
    mqttClient.setServer(server, 1883);
    mqttClient.setCallback(callback);
    reconnect();
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT Broker... ");
    char hostChar[host.length() + 1];
    host.toCharArray(hostChar, host.length() + 1);
    
    if (mqttClient.connect(hostChar, "mqtt_led", "passwordLED")) { // Fixed incorrect object reference
      Serial.println("MQTT Connected");
      mqttClient.subscribe(reed_switch_topic);
      mqttClient.subscribe(distance_topic);
    } else {
      Serial.print("MQTT Connection Failure, State: ");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  if (String(topic) == reset_topic && message == "led") {
    Serial.println("Reset command received via MQTT. Clearing Wi-Fi credentials...");
    clearWiFiCredentials();
    Serial.println("Restarting...");
    delay(2000);
    ESP.restart();
  } else if (String(topic) == reed_switch_topic) {
    reedSwitchState = (message == "on");
    Serial.println(reedSwitchState ? "Reed switch ON" : "Reed switch OFF");
    if (reedSwitchState) {
      allOff();
    }
  } else if (String(topic) == distance_topic && !reedSwitchState) {
    blinkModeDC = false;
    int distance = message.toInt();
    Serial.print("Distance received: ");
    Serial.println(distance);
    controlLEDs(distance);
  }
}

void clearWiFiCredentials() {
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    Serial.println("EEPROM cleared.");
}

// Shuts off all lights
void allOff() {
  blinkMode = false;
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  state = "off";
}

// Handles blinking when blinkmode is enabled.
void blink() {
  static unsigned long lastBlinkTime = 0;
  static bool ledState = false;
  if (blinkMode && millis() - lastBlinkTime > 500) {
    lastBlinkTime = millis();
    ledState = !ledState;
    digitalWrite(RED_LED, ledState);
    state = "blinking";
  }
}

// Handles blinking when there is a disconnect
void blink_DC() {
  static unsigned long lastBlinkDCTime = 0;
  static bool ledDCState = false;
  if (blinkModeDC && millis() - lastBlinkDCTime > 500) {
    lastBlinkDCTime = millis();
    ledDCState = !ledDCState;
    digitalWrite(YELLOW_LED, ledDCState);
    state = "blinking_DC";
  }
}

// Handles individual light control
void controlLEDs(int distance) {
  allOff();
  if (distance < 20) {
    blinkMode = true;
    state = "blinking";
  } else if (distance < 40) {
    blinkMode = false;
    digitalWrite(RED_LED, HIGH);
    state = "red";
  } else if (distance < 60) {
    blinkMode = false;
    digitalWrite(YELLOW_LED, HIGH);
    state = "yellow";
  } else {
    blinkMode = false;
    digitalWrite(GREEN_LED, HIGH);
    state = "green";
  }
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  blink();
}
