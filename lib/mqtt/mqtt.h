#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>

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
    String publish(int clientIdx, int msgId, int qos, int retained, const char *topic, int length);
    void receiveMessage(int clientIdx, int recvId);
    String disconnectClient(int clientIdx);
    String closeConnection(int clientIdx);
};

#endif
