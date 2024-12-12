// #include <Arduino.h>

// // Pins
// const int Button = 4; // Button on pin D4
// const int LED = 13;   // LED on pin D13
// const int Potentiometer = A0; // Potentiometer on pin A0

// // Functions
// bool check_switch() {
//   // Return true if the button is pressed (HIGH)
//   return digitalRead(Button) == HIGH;
// }

// void setup() {
//   // Initialize pin modes
//   pinMode(Button, INPUT);
//   pinMode(LED, OUTPUT);
  
//   // Initialize serial communication
//   Serial.begin(9600);
//   Serial.println("System Ready");
// }

// void loop() {
//   // Potentiometer value
//   int potValue = analogRead(Potentiometer);
//   float voltage = potValue * (5.0 / 1023.0); // Convert to voltage
//   Serial.print("Potentiometer Value: ");
//   Serial.print(potValue);
//   Serial.print(" (Voltage: ");
//   Serial.print(voltage);
//   Serial.println(" V)");

//   // LED control
//   Serial.println("Turn LED ON (1) or OFF (0):");
//   while (!Serial.available()); // Wait for user input
//   int userLED = Serial.parseInt();
//   if (userLED == 1) {
//     digitalWrite(LED, HIGH);
//     Serial.println("LED is ON");
//   } else {
//     digitalWrite(LED, LOW);
//     Serial.println("LED is OFF");
//   }

//   // Check switch state
//   if (check_switch()) {
//     Serial.println("Button Pressed");
//   } else {
//     Serial.println("Button Not Pressed");
//   }

//   delay(1000); // Delay for readability
// }

//-----------------------------
// Title: MQTT
//-----------------------------
// Program Details:
//-----------------------------
// Purpose: Conenct to broker.mqtt-dashboard.com, Publish and subscribe
// Dependencies: Make sure you have installed PubSubClient.h
// Compiler: PIO Version 1.72.0
// Atuhor: Originally an example called ESP8266-mqtt / slightly modified and cleaned up by Farid Farahmand
// OUTPUT: publishes 1,2,3,.... on outTopic every publishTimeInterval
// INPUT: Received value from the broker on inTopic  
// SETUP: To see the published values go to http://www.hivemq.com/demos/websocket-client/ 
//        subscribe to inTopic and outTopic. You can also create an APP using MQTT Dash
// Versions: 
//  v1: Nov-24-2022 - Cleaned up version 
//-----------------------------

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>

// WiFi variables
const char* ssid = <>;  // Enter your WiFi name
const char* password = <>;  // Enter WiFi password

// MQTT variables
const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* publishTopic = "testtopic/temp/outTopic/eriberto";   // outTopic where ESP publishes
const char* subscribeTopic = "testtopic/temp/inTopic/eriberto";  // inTopic where ESP has subscribed to
#define publishTimeInterval 10000 // in seconds 

// Definitions 
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
#define BUILTIN_LED 2 // built-in LED
char msg[MSG_BUFFER_SIZE];
int value = 0;
int ledStatus = 0;

WiFiClient espClient;
PubSubClient client(espClient); // define MQTTClient 

const int Potentiometer = A0; // Potentiometer on pin A0

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
  //------------------------------------------

void callback(char* topic, byte* payload, int length) {
  Serial.print("Message arrived ["); // Received the incoming message
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);  // the received value is in (char)payload[i]
  }
  Serial.println();
  // Switch on the LED if an 1 was received as first character
  // add your code here

}
  //------------------------------------------

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // ... and resubscribe
      //client.subscribe(subscribeTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 //------------------------------------------

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
//------------------------------------------
void loop() {

  if (!client.connected()) {
    reconnect(); // check for the latest value in inTopic 
  }
  client.loop();

// Publish to outTopic 
  unsigned long now = millis(); 
  if (now - lastMsg > publishTimeInterval) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "Number # %d", value); // prints Number # 1, Number # 2, .....
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(publishTopic, msg); // variables published 
    
        // Prepare message for potentiometer value
    int potValue = analogRead(Potentiometer);
    char potMessage[MSG_BUFFER_SIZE];
    snprintf(potMessage, MSG_BUFFER_SIZE, "Pot Value: %d", potValue);
    Serial.print("Publish potentiometer value: ");
    Serial.println(potMessage);
    
    // Publish the potentiometer value
    client.publish(publishTopic, potMessage); // Send pot value as a string      
  }
}
