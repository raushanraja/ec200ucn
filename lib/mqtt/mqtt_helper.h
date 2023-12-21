#include<Arduino.h>
#include<atserial.h>


#ifndef MQTT_HELPER_H
#define MQTT_HELPER_H


#define IO_USERNAME  ""
#define IO_KEY       ""
#define MQTT_HOST    "hivemq.cloud"
#define MQTT_PORT    8883

#define clientIdx 0
#define ssl_ctx_id 0
#define SSL_CERT_NAME "hive"


enum LastATCommand
{
  AT,
  QMTCFG_VERSION,
  QMTCFG_SSLENABLE,
  QMTCFG_RECV,
  QSSLCFG_SSLVER,
  QSSLCFG_CIPHER,
  QSSLCFG_SECLEVEL,
  QSSLCFG_CACERT,
  QSSLCFG_IGNOREINVALID,
  QSSLCFG_SNI,
  QMTOPEN,
  QMTCONN,
  QMTDISC,
  QMTCLOSE,
  QMTSUB,
  QMTPUBEX,
};


String PublishableATCommands[] = {  "+QNWINFO", "+CSQ", "+QCSQ", "+QSPN" };

LastATCommand lastATCommand = LastATCommand::AT;

void openConnection();
void closeConnection();
void connectClient();
void disconnectClient();
void processQMTRECV(String message);
void processQMTSTAT(String message);
void processQMTOPEN(String message);
void processQMTCLOSE(String message);
void processQMTCONNRET(String ret_code);
void processQMTCONN(String message);
void processQMTClientDisconnected(String message);
void setupMQTT();

#endif