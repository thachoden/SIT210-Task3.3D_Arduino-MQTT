#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <NewPing.h>

// WiFi credentials
const char* ssid = "Quang Huy";
const char* password = "Danang040903";

// MQTT broker details
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_topic_wave = "SIT210/wave"; //Topic for "wave"
const char* mqtt_topic_pat = "SIT210/pat"; //Topic for "pat"

// Ultrasonic sensor pins
#define TRIG_PIN 4
#define ECHO_PIN 5
#define PAT_DISTANCE 10
#define MAX_DISTANCE 20

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE); //declare new instance to use ultrasonic sensor

// LED pin
const int ledPin = 2;

// WiFi and MQTT clients
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message received: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();

  // Check if the message is for "wave" or "pat"
  if (strcmp(topic, mqtt_topic_wave) == 0) {
    // Blink LED 3 times for wave
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      delay(500);
      digitalWrite(ledPin, LOW);
      delay(500);
    }
  } else if (strcmp(topic, mqtt_topic_pat) == 0) {
    // Flash LED rapidly for pat
    for (int i = 0; i < 6; i++) { // Flash 6 times
      digitalWrite(ledPin, HIGH);
      delay(200); // Shorter delay for rapid flash
      digitalWrite(ledPin, LOW);
      delay(200);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoClient")) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_wave); // Subscribe to the "wave" topic
      client.subscribe(mqtt_topic_pat); // Subscribe to the "pat" topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned int distance = sonar.ping_cm();
  if (distance > PAT_DISTANCE && distance <= MAX_DISTANCE) {
    Serial.println("Wave detected!");
    String message = "MThac's Wave"; // Name print
    client.publish(mqtt_topic_wave, message.c_str());
    delay(1000); // Debounce delay
  }
  if (distance > 0 && distance <= PAT_DISTANCE) {
    Serial.println("Pat detected!");
    String message = "Mthac's Pat"; // Name print
    client.publish(mqtt_topic_pat, message.c_str());
    delay(1000); // Debounce delay
  }
}
