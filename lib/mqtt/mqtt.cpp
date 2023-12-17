#include "mqtt.h"

#define OPENNETWORKTO 120000
#define CONNCLIENTTO 50000

void MQTT::begin(uint32_t baudRate, uint8_t parity, uint8_t rxPin, uint8_t txPin)
{
    mqttSerial.begin(baudRate, parity, rxPin, txPin);
    Serial.println("MQTT: Serial communication started");
}

bool MQTT::waitForResponse(const char *expectedResponse)
{
    unsigned long startTime = millis();
    String response = "";
    while (millis() - startTime < 3000)
    {
        if (mqttSerial.available())
        {
            response = mqttSerial.readString();
        }
        if (response.indexOf(expectedResponse) != -1)
        {
            return true;
        }
    }
    Serial.println("MQTT: Expected response not found");
    Serial.println("Got response: " + response);
    return false;
}

bool MQTT::waitForResponse(const char *expectedResponse, unsigned long timeout)
{
    unsigned long startTime = millis();
    String response;

    while (millis() - startTime < timeout)
    {
        delay(100);

        if (mqttSerial.available())
        {
            response += mqttSerial.readString();
            Serial.println("Got response: " + response);
            if (response.indexOf(expectedResponse) != -1)
            {
                return true;
            }

            // Check for known responses that may indicate an exit condition
            const bool exit = handleKnownResponse(response);
            if (exit)
            {
                return true;
            }
        }
    }

    Serial.println("MQTT: Expected response not found");
    Serial.println("Got response: " + response);
    return false;
}

String MQTT::configureProtocolVersion(int clientIdx, int version)
{
    String command = "AT+QMTCFG=\"version\"," + String(clientIdx) + "," + String(version);
    return command;
}

String MQTT::openNetwork(int clientIdx, const char *hostName, int port)
{
    return "AT+QMTOPEN=" + String(clientIdx) + ",\"" + String(hostName) + "\"," + String(port);
}

String MQTT::connClient(int clientIdx, const char *clientID, char *user, char *password)
{
    return "AT+QMTCONN=" + String(clientIdx) + ",\"" + String(clientID) + "\",\"" + String(user) + "\",\"" + String(password) + "\"";
}

String MQTT::subscribe(int clientIdx, int msgId, const char *topic, int qos)
{
    return "AT+QMTSUB=" + String(clientIdx) + "," + String(msgId) + ",\"" + String(topic) + "\"," + String(qos);
}

String MQTT::publish(int clientIdx, int msgId, int qos, int retained, const char *topic, const char *payload)
{
    return "AT+QMTPUB=" + String(clientIdx) + "," + String(msgId) + "," + String(qos) + "," + String(retained) + ",\"" + String(topic) + "\",\"" + String(payload) + "\"";
}

void MQTT::receiveMessage(int clientIdx, int recvId)
{
    char command[40];
    snprintf(command, sizeof(command), "AT+QMTRECV=%d,%d", clientIdx, recvId);

    Serial.printf("MQTT: Received message with recvId: %d\n", recvId);
}

String MQTT::disconnectClient(int clientIdx)
{
    return "AT+QMTDISC=" + String(clientIdx);
}

String MQTT::closeConnection(int clientIdx)
{
    return "AT+QMTCLOSE=" + String(clientIdx);
}



void MQTT::updateConnectedState(const bool connected)
{
    if (connected)
    {
        Serial.println("MQTT: Network connected");
    }
    else if (!connected)
    {
        Serial.println("MQTT: Network disconnected");
    }
    networkConnected = connected;
}

bool MQTT::handleKnownResponse(const String response)
{
    // Return false if response is empty
    if (response.length() == 0)
    {
        return false;
    }
    else if (response.startsWith("+QMTOPEN") == 0)
    {
        Serial.println("MQTT: Open network response received");
        return true;
    }
    else
    {
        return false;
    }
}

// AT+QMTCFG=\"version\",%d,%d"