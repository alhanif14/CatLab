#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Itu";
const char* password = "kagakada";

const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

int IN1 = D1;
int IN2 = D2;
int IN3 = D3;
int IN4 = D5;

int motorDelay = 2;

const int trigPin = D6;
const int echoPin = D7;

bool isStepperRunning = false;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void forwardMotor() {
  isStepperRunning = true;
  digitalWrite(IN4, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN1, LOW);
  delay(motorDelay);
  digitalWrite(IN4, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN1, LOW);
  delay(motorDelay);
  digitalWrite(IN4, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN1, LOW);
  delay(motorDelay);
  digitalWrite(IN4, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN1, HIGH);
  delay(motorDelay);
  isStepperRunning = false;
}

void backwardMotor() {
  isStepperRunning = true;
  digitalWrite(IN4, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN1, HIGH);
  delay(motorDelay);
  digitalWrite(IN4, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN1, LOW);
  delay(motorDelay);
  digitalWrite(IN4, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN1, LOW);
  delay(motorDelay);
  digitalWrite(IN4, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN1, LOW);
  delay(motorDelay);
  isStepperRunning = false;
}

void stopMotor() {
  digitalWrite(IN4, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN1, LOW);
  isStepperRunning = false;
}

void onForward(int x) {
  for (int step = 0; step < x; step++) {
    forwardMotor();
  }
}

void onBack(int x) {
  for (int step = 0; step < x; step++) {
    backwardMotor();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  int value = message.toInt();
  onForward(value);
}

void reconnect() {  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("WemosClient")) {
      Serial.println("connected");
      client.subscribe("Hanif/stepper");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (!isStepperRunning) {
    long duration, distance;
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distance = (duration / 2) / 29.1;

    Serial.print("Distance: ");
    Serial.println(distance);

    int percentage = map(distance, 2, 6, 100, 10);
    percentage = constrain(percentage, 10, 100);

    String percentageStr = String(percentage);
    client.publish("Hanif/sonic", percentageStr.c_str());

    delay(1000);
  }
}
