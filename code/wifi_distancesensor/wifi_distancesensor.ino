#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "NewPing.h"

// WiFi Credentials
const char* ssid = "IM18_H";
const char* password = "!i980693Iggf";

// MQTT Broker
const char* server = "192.168.86.88"; // Replace with your laptop's IP running Mosquitto
const char* distance_topic = "sensor/distance";
const char* reed_switch_topic = "sensor/reed";
const char* led_topic = "led/control";

// Get MAC Address for unique client ID
String macAddr = WiFi.macAddress();
String host = "arduino-sensor-" + macAddr.substring(12); // Shorten to avoid long names

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Define GPIO pins
#define TRIGGER_PIN D5
#define ECHO_PIN D6

// NewPing setup
#define MAX_DISTANCE 400
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

bool reedSwitchState = false;

void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println("Initializing...");

  pinMode(LED_BUILTIN, OUTPUT);  // Built-in LED

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
  mqttClient.subscribe(reed_switch_topic);
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT Broker... ");
    char hostChar[host.length() + 1];
    host.toCharArray(hostChar, host.length() + 1);
    
    if (mqttClient.connect(hostChar, "mqtt_dsensor", "passwordDS")) { // Fixed incorrect object reference
      Serial.println("MQTT Connected");
      mqttClient.subscribe(reed_switch_topic);
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
  }
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop(); // Check for new messages

  if (!reedSwitchState) {
    int distance = sonar.ping_cm();
    Serial.print("Distance = ");
    Serial.print(distance);
    Serial.println(" cm");

    char distanceStr[6];
    sprintf(distanceStr, "%d", distance);
    mqttClient.publish(distance_topic, distanceStr);
  } else {
    mqttClient.publish(led_topic, "off");
  }

  delay(500);
}
