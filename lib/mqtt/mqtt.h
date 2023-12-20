#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <HardwareSerial.h>

enum MQTT_STATE
{
    OPENNETWORKTO,
    CONNCLIENTTO
};

enum KnownResponses
{
    OPENNETWORKRESPONSE,
    CONNCLIENTRESPONSE
};

class MQTT
{
public:
    MQTT(HardwareSerial &serial) : mqttSerial(serial){};

    void begin(uint32_t baudRate, uint8_t parity, uint8_t rxPin, uint8_t txPin);
    String configureProtocolVersion(int clientIdx, String version);
    String configureRecvMode(int clientIdx, String recvmsgmode, String lenmode);
    String configureSSLEnable(int clientIdx, bool enable, String ssl_ctx_id);
    String configureSSLVersion(int ssl_ctx_id, String version);
    String configureSSLCipher(int ssl_ctx_id, String cipher);
    String configureSSLCert(int ssl_ctx_id, String cert);
    String configureSSLSecLever(int ssl_ctx_id, int level);
    String configureSSLIgnoreInValidCert(int ssl_ctx_id, bool ignore);
    String configureSSLSNI(int ssl_ctx_id, String sni);
    String openNetwork(int clientIdx, const char *hostName, int port);
    String connClient(int clientIdx, const char *clientID, char *user, char *password);
    String subscribe(int clientIdx, int msgId, const char *topic, int qos);
    String publish(int clientIdx, int msgId, int qos, int retained, const char *topic, const char *payload);
    void receiveMessage(int clientIdx, int recvId);
    String disconnectClient(int clientIdx);
    String closeConnection(int clientIdx);
    void setResponseHandler(bool (*handler)(const String &))
    {
        responseHandler = handler;
    }

private:
    HardwareSerial &mqttSerial;
    bool networkConnected = false;

    void updateConnectedState(const bool connected);
    bool waitForResponse(const char *expectedResponse);
    bool waitForResponse(const char *expectedResponse, unsigned long timeout);
    bool executeCommand(const char *command, HardwareSerial &serial);
    bool handleKnownResponse(const String response);
    bool (*responseHandler)(const String &); // Corrected function pointer type // Function pointer for response handler
};

#endif
