#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "IM18_H";
const char* password = "!i980693Iggf";

// MQTT Broker
const char* server = "192.168.86.88"; // Replace with your laptop's IP running Mosquitto
const char* reed_switch_topic = "sensor/reed";

// Get MAC Address for unique client ID
String macAddr = WiFi.macAddress();
String host = "arduino-reed-" + macAddr.substring(12); // Shorten to avoid long names

// GPIO pin for reed switch
#define REED_SWITCH_PIN D4

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

bool lastState = false;

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
  reconnect();

  pinMode(REED_SWITCH_PIN, INPUT_PULLUP);
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT Broker... ");
    char hostChar[host.length() + 1];
    host.toCharArray(hostChar, host.length() + 1);

    if (mqttClient.connect(hostChar, "mqtt_reed", "passwordREED")) {
      Serial.println("MQTT Connected");
    } else {
      Serial.print("MQTT Connection Failure, State: ");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  bool reedSwitchState = digitalRead(REED_SWITCH_PIN) == LOW; 
  
  if (reedSwitchState != lastState) {
    lastState = reedSwitchState;
    mqttClient.publish(reed_switch_topic, reedSwitchState ? "on" : "off");
    Serial.println(reedSwitchState ? "Reed Switch ON" : "Reed Switch OFF");
  }

  delay(500);
}
