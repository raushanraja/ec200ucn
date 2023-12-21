#include "mqtt.h"

#define OPENNETWORKTO 120000
#define CONNCLIENTTO 50000




String MQTT::configureProtocolVersion(int clientIdx, String version)
{
    return "AT+QMTCFG=\"version\"," + String(clientIdx) + "," + String(version);
}

String MQTT::configureRecvMode(int clientIdx, String recvmsgmode, String lenmode)
{
    return "AT+QMTCFG=\"recv/mode\"," + String(clientIdx) + "," + recvmsgmode + "," + lenmode;
}

String MQTT::configureSSLEnable(int clientIdx, bool enable, String ssl_ctx_id){
    if (enable) {
        return "AT+QMTCFG=\"SSL\"," + String(clientIdx) + ",1," + ssl_ctx_id;
    } else {
        return "AT+QMTCFG=\"SSL\"," + String(clientIdx) + ",0," + ssl_ctx_id;
    }
}

String MQTT::configureSSLVersion(int ssl_ctx_id, String version){
    return "AT+QSSLCFG=\"sslversion\"," + String(ssl_ctx_id) + "," + version;
}

String MQTT::configureSSLCipher(int ssl_ctx_id, String ciphers){
    return "AT+QSSLCFG=\"ciphersuite\"," + String(ssl_ctx_id) + "," + String(ciphers);
}

String MQTT::configureSSLCert(int ssl_ctx_id, String cert){
    return "AT+QSSLCFG=\"cacert\"," + String(ssl_ctx_id) + "," + "\"" + cert + "\"";
}

String MQTT::configureSSLSecLever(int ssl_ctx_id, int level){
    return "AT+QSSLCFG=\"seclevel\"," + String(ssl_ctx_id) + "," + String(level);
}

String MQTT::configureSSLIgnoreInValidCert(int ssl_ctx_id, bool ignore){
    if (ignore) {
        return "AT+QSSLCFG=\"ignoreinvalidcertsign\"," + String(ssl_ctx_id) + ",1";
    } else {
        return "AT+QSSLCFG=\"ignoreinvalidcertsign\"," + String(ssl_ctx_id) + ",0";
    }
}

String MQTT::configureSSLSNI(int ssl_ctx_id, String sni){
    return "AT+QSSLCFG=\"sni\"," + String(ssl_ctx_id) + "," + String(sni);
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

String MQTT::publish(int clientIdx, int msgId, int qos, int retained, const char *topic, int length)
{
    return "AT+QMTPUBEX=" + String(clientIdx) + "," + String(msgId) + "," + String(qos) + "," + String(retained) + ",\"" + String(topic) + "\","+ String(length);
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



// AT+QMTCFG=\"version\",%d,%d"
