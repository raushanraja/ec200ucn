#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <HardwareSerial.h>


class MQTT {
public:
    MQTT(HardwareSerial& serial) : mqttSerial(serial) {};

    void begin(uint32_t baudRate, uint8_t parity, uint8_t rxPin, uint8_t txPin);
    void configureProtocolVersion(int clientIdx, int version);
    void openNetwork(int clientIdx, const char* hostName, int port);
    void connClient(int clientIdx, const char* clientID, char* user, char* password);
    void subscribe(int clientIdx, int msgId, const char* topic, int qos);
    void publish(int clientIdx, int msgId, int qos, int retained, const char* topic, const char* payload);
    void receiveMessage(int clientIdx, int recvId);
    void disconnect(int clientIdx);

private:
    HardwareSerial& mqttSerial;
    bool waitForResponse(const char* expectedResponse);
    bool waitForResponse(const char* expectedResponse, unsigned long timeout);
    bool executeCommand(const char* command, const char* expectedResponse);
};

#endif
