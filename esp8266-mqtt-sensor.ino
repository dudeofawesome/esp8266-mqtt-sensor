// hacks for VS Code support
#ifndef ESP8266
#define ESP8266
#endif
// #ifndef ARDUINO
// #define ARDUINO 189
// #endif
#include "Arduino.h"

#include "EspMQTTClient.h"
#include <MHZ.h>

// pins for uart reading
#define MH_Z19_RX D7
#define MH_Z19_TX D6

#define CLIENT_NAME String("co2-sensor")

MHZ co2(MH_Z19_RX, MH_Z19_TX, MHZ14A);

EspMQTTClient
    client("ssid",      // WiFi SSID
           "password",  // WiFi password
           "broker-IP", // MQTT Broker server ip
           //"MQTTUsername", // Can be omitted if not needed
           //"MQTTPassword", // Can be omitted if not needed
           "co2-sensor", // Client name that uniquely identify your device
           1883 // The MQTT port, default to 1883. this line can be omitted
    );

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LOW and HIGH are backwards on ESP
  Serial.begin(9600);
  delay(100);

  // Optional functionalities of EspMQTTClient:

  // Enable debugging messages sent to serial output
  client.enableDebuggingMessages();
  // You can activate the retain flag by setting the third parameter to true
  char status[100];
  (CLIENT_NAME + String("/status")).toCharArray(status, 100);
  client.enableLastWillMessage(status, "offline");

  // enable debug to get additional information
  // co2.setDebug(true);

  if (co2.isPreHeating()) {
    Serial.println();
    Serial.print("Preheating");
  }
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished() {
  digitalWrite(LED_BUILTIN, HIGH); // LOW and HIGH are backwards on ESP
  char status[100];
  (CLIENT_NAME + String("/status")).toCharArray(status, 100);
  client.publish(status, "online");
}

void loop() {
  client.loop();

  if (co2.isPreHeating()) {
    Serial.print(".");
    delay(1000);
  } else if (co2.isReady()) {
    Serial.print("[");
    Serial.print(millis() / 1000);
    Serial.print("s]: ");

    int ppm_uart = co2.readCO2UART();
    Serial.print("PPM: ");
    Serial.print(ppm_uart);

    int temperature = co2.getLastTemperature();
    Serial.print(", Temperature: ");
    Serial.print(temperature);

    Serial.println();

    char ppm_update[100];
    (CLIENT_NAME + String("/ppm")).toCharArray(ppm_update, 100);
    client.publish(ppm_update, String(ppm_uart).c_str());
    char temp_update[100];
    (CLIENT_NAME + String("/temp")).toCharArray(temp_update, 100);
    client.publish(temp_update, String(temperature).c_str());

    delay(5000);
  }
}
