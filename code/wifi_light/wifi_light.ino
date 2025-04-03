#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Define Wifi Network and Password
const char* ssid = "IM18_H";
const char* password  = "!i980693Iggf";

// MQTT Broker
const char* server = "192.168.86.88"; // Replace with your laptop's IP running Mosquitto
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
String state = "off";
bool reedSwitchState = false;

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

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname(host.c_str());
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);  // ON (Active Low)
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH); // OFF
    delay(250);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);

  mqttClient.setServer(server, 1883);
  mqttClient.setCallback(callback);
  reconnect();
  mqttClient.subscribe(distance_topic);
  mqttClient.subscribe(reed_switch_topic);
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

  if (String(topic) == reed_switch_topic) {
    reedSwitchState = (message == "on");
    Serial.println(reedSwitchState ? "Reed switch ON" : "Reed switch OFF");
    if (reedSwitchState) {
      allOff();
    }
  }

  if (String(topic) == distance_topic && !reedSwitchState) {
    int distance = message.toInt();
    Serial.print("Distance received: ");
    Serial.println(distance);
    controlLEDs(distance);
  }
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
  } else if (distance < 100) {
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
