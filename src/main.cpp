#include <mqtt.h>

#define RX_PIN D6
#define TX_PIN D7

#define IO_USERNAME  "CHANGEME"
#define IO_KEY       "CHANGEME"
#define MQTT_HOST    "CHANGEME"
#define MQTT_PORT    1883

HardwareSerial ATSerial(1);
MQTT mqttClient(ATSerial);

void setup() {
    Serial.begin(9600);
    mqttClient.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

    delay(20000);
    mqttClient.configureProtocolVersion(0, 3);
    mqttClient.openNetwork(0, MQTT_HOST, MQTT_PORT);
    mqttClient.connClient(0,"U1", IO_USERNAME, IO_KEY);
    mqttClient.subscribe(0, 1, IO_USERNAME "/feeds/temperature", 1);
    mqttClient.publish(0, 1, 1, 0, "/feeds/temperature", "Hello, World!");
    // Other MQTT operations
    delay(5000);
}

void loop() {
  if (ATSerial.available())
  {
    Serial.write(ATSerial.read());
  }
  if (Serial.available())
  {
    ATSerial.write(Serial.read());
  }
  // delay(1000);
}