#include "mqtt.h"

#define OPENNETWORKTO 120000 
#define CONNCLIENTTO 50000

void MQTT::begin(uint32_t baudRate, uint8_t parity, uint8_t rxPin, uint8_t txPin) {
    mqttSerial.begin(baudRate, parity, rxPin, txPin);
    Serial.println("MQTT: Serial communication started");
}

bool MQTT::waitForResponse(const char* expectedResponse) {
    unsigned long startTime = millis();
    String response = "";
    while (millis() - startTime < 3000) {
        if (mqttSerial.available()) {
            response = mqttSerial.readString();
        }
        if (response.indexOf(expectedResponse) != -1) {
            return true;
        }
    }
    Serial.println("MQTT: Expected response not found");
    Serial.println("Got response: " + response);
    return false;
}

bool MQTT::waitForResponse(const char* expectedResponse, unsigned long timeout) {
    unsigned long startTime = millis();
    String response = "";
    while (millis() - startTime < timeout) {
        delay(100);
        if (mqttSerial.available()) {
            response = mqttSerial.readString();
        }
        if (response.indexOf(expectedResponse) != -1) {
            return true;
        }
        else{
            Serial.println("MQTT: Expected response not found");
            Serial.println("Got response: " + response);
        }
    }
    Serial.println("MQTT: Expected response not found");
    Serial.println("Got response: " + response);
    return false;
}

bool MQTT::executeCommand(const char* command, const char* expectedResponse) {
    mqttSerial.println(command);
    delay(500); // Give some time to process command
    Serial.printf("MQTT: Sent command: %s\n", command);
    return waitForResponse(expectedResponse);
}

bool MQTT::executeCommand(const char* command, const char* expectedResponse, unsigned long timeout) {
    mqttSerial.println(command);
    delay(500); // Give some time to process command
    Serial.printf("MQTT: Sent command: %s\n", command);
    return waitForResponse(expectedResponse, timeout);
}

void MQTT::configureProtocolVersion(int clientIdx, int version) {
    char command[50];
    snprintf(command, sizeof(command), "AT+QMTCFG=\"version\",%d,%d", clientIdx, version);
    executeCommand(command, "OK");
    Serial.println("MQTT: Protocol version configured");
}

void MQTT::openNetwork(int clientIdx, const char* hostName, int port) {
    char command[70];
    snprintf(command, sizeof(command), "AT+QMTOPEN=%d,\"%s\",%d", clientIdx, hostName, port);
    executeCommand(command, "+QMTOPEN: 0,0", OPENNETWORKTO);
    Serial.printf("MQTT: Network opened with host: %s, port: %d\n", hostName, port);
}

void MQTT::connClient(int clientIdx, const char* clientID, char* user, char* password) {
    char command[100];
    char expectedResponse[30];
    snprintf(expectedResponse, sizeof(expectedResponse), "+QMTCONN: %d,0,0", clientIdx);
    snprintf(command, sizeof(command), "AT+QMTCONN=%d,\"%s\",\"%s\",\"%s\"", clientIdx, clientID, user, password);
    executeCommand(command, expectedResponse, CONNCLIENTTO);
    Serial.printf("MQTT: Connected to client: %s\n", clientID);
}

void MQTT::subscribe(int clientIdx, int msgId, const char* topic, int qos) {
    char command[80];
    snprintf(command, sizeof(command), "AT+QMTSUB=%d,%d,\"%s\",%d", clientIdx, msgId, topic, qos);
    executeCommand(command, "+QMTSUB: 0,0,0");
    Serial.printf("MQTT: Subscribed to topic: %s, qos: %d\n", topic, qos);
}

void MQTT::publish(int clientIdx, int msgId, int qos, int retained, const char* topic, const char* payload) {
    char command[150];
    snprintf(command, sizeof(command), "AT+QMTPUB=%d,%d,%d,%d,\"%s\",\"%s\"", clientIdx, msgId, qos, retained, topic, payload);
    executeCommand(command, "+QMTPUB: 0,0,0");
    Serial.printf("MQTT: Published message to topic: %s, payload: %s\n", topic, payload);
}

void MQTT::receiveMessage(int clientIdx, int recvId) {
    char command[40];
    snprintf(command, sizeof(command), "AT+QMTRECV=%d,%d", clientIdx, recvId);
    executeCommand(command, "+QMTRECV");
    Serial.printf("MQTT: Received message with recvId: %d\n", recvId);
}

void MQTT::disconnect(int clientIdx) {
    char command[30];
    snprintf(command, sizeof(command), "AT+QMTDISC=%d", clientIdx);
    executeCommand(command, "OK");
    Serial.println("MQTT: Disconnected");
}
