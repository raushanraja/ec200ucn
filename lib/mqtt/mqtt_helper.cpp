#include<mqtt_helper.h>

void openConnection()
{
  Serial.println("Opening connection");
  ATSerial.println(mqttClient.openNetwork(0, MQTT_HOST, MQTT_PORT));
}

void closeConnection()
{
  Serial.println("Closing connection");
  lastATCommand = QMTCLOSE;
  ATSerial.println(mqttClient.closeConnection(0));
}

void connectClient()
{
  Serial.println("Connecting client");
  ATSerial.println(mqttClient.connClient(0, "U1", IO_USERNAME, IO_KEY));
}

void disconnectClient()
{
  Serial.println("Disconnecting client");
  ATSerial.println(mqttClient.disconnectClient(0));
}

void processQMTRECV(String message){
  int client_idx, recvId;
  char topic[40];
  int length;
  char data[40];

  sscanf(message.c_str(), "+QMTRECV: %d,%d,%[^,],%d,\"%[^]\"", &client_idx, &recvId, topic, &length, data);

  snprintf(topic, sizeof(topic), "%s", topic);
  snprintf(data, sizeof(data), "%s", data);
  Serial.printf("Client index: %d, recvId: %d, topic: %s, length: %d, data: %s\n", client_idx, recvId, topic, length, data);

  if(String(data).startsWith("AT")){
    ATSerial.println(data);
  }

}

void processQMTSTAT(String message)
{
  // format +QMTSTAT: <client_id>,<err_code>
  Serial.println("Received +QMTSTAT from Task 1 via Queue 1");
  // Add your processing logic here
  int pos = message.indexOf(",");
  String client_id = message.substring(0, pos);
  String err_code = message.substring(pos + 1, message.length() - 1);
  if (err_code == "1")
  {
    Serial.println("Connection closed by peer, re-open the connection using +QMTOPEN");
    lastATCommand = QMTOPEN;
    openConnection();
  }
  else if (err_code == "2")
  {
    Serial.println("PINGREQ packet timedout, Deactiave PDP, Activate PDP, Re-open the connection");
  }
  else if (err_code == "3")
  {
    Serial.println("Sending CONNECT timedout, check user cred, check client id is not resued, re-open connection");
  }
  else if (err_code == "4")
  {
    Serial.println("Receiving CONNACK timedout, check user cred, check client id is not resued, re-open connection");
  }
  else if (err_code == "5")
  {
    Serial.println("Client sent DISCONNECT, normal disconnection");
  }
  else if (err_code == "6")
  {
    Serial.println("Client sent DISCONNECT, due to packet sending failue, check data being sent, re-open connection");
  }
  else if (err_code == "7")
  {
    Serial.println("Server not available, Fix server IP, re-open connection");
  }
  else
  {
    Serial.println("Client closed connection for some unknown reason, re-open connection");
  }
}

void processQMTOPEN(String message)
{
  Serial.println("Received +QMTOPEN from Task 1 via Queue 1");
  // format: +QMTOPEN: <client_id>,<result>
  int pos = message.indexOf(",");
  String client_id = message.substring(0, pos);
  String result = message.substring(pos + 1, message.length() - 1);

  if (result == "0")
  {
    Serial.println("Network opened successfully");
    lastATCommand = QMTCONN;
    connectClient();
  }
  else if (result == "1")
  {
    Serial.println("Wrong parameter");
  }
  else if (result == "2")
  {
    Serial.println("MQTT identifier is occupied");
    lastATCommand = QMTDISC;
    disconnectClient();
  }
  else if (result == "3")
  {
    Serial.println("Failed to activate PDP");
  }
  else if (result == "4")
  {
    Serial.println("Failed to parse domain name");
  }
  else if (result == "5")
  {
    Serial.println("Network connection error");
  }
  else if (result == "-1")
  {
    Serial.println("Failed to open network");
  }
  else
  {
    Serial.println("Unknown error");
  }
}

void processQMTCLOSE(String message)
{
  Serial.println("Received +QMTCLOSE from Task 1 via Queue 1");
  // format: +QMTCLOSE: <client_id>,<result>
  int pos = message.indexOf(",");
  String client_id = message.substring(0, pos);
  String result = message.substring(pos + 1, message.length() - 1);

  if (result == "0")
  {
    Serial.println("Connection closed");
    openConnection();
  }
  else if (result == "-1")
  {
    Serial.println("Failed to close connection");
  }
  else
  {
    Serial.println("Unknown error");
  }
}

void processQMTCONNRET(String ret_code)
{
  if (ret_code == "0")
  {
    Serial.println("Connection Accepted");
  }
  else if (ret_code == "1")
  {
    Serial.println("Unacceptable Protocol Version");
  }
  else if (ret_code == "2")
  {
    Serial.println("Identifier Rejected");
  }
  else if (ret_code == "3")
  {
    Serial.println("Server Unavailable");
  }
  else if (ret_code == "4")
  {
    Serial.println("Bad User Name or Password");
  }
  else if (ret_code == "5")
  {
    Serial.println("Not Authorized");
  }
}
void processQMTCONN(String message)
{
  Serial.println("Received +QMTCONN from Task 1 via Queue 1" + message);
  // format: +QMTCONN: <client_idx>,<result>,<ret_code>
  int pos = message.indexOf(",");
  int pos2 = message.indexOf(",", pos + 1);
  String client_idx = message.substring(0, pos);
  String result = message.substring(pos + 1, pos2);
  String ret_code = message.substring(pos2 + 1, message.length() - 1);


  if (result == "0")
  {
    Serial.println("Connection successful");
    lastATCommand = LastATCommand::QMTSUB;
    ATSerial.println(mqttClient.subscribe(clientIdx, 1, "WaterPump",0));
  }
  else if (result == "1")
  {
    Serial.println("Packet retransmission");
  }
  else if (result == "2")
  {
    Serial.println("Failed to send packet");
  }
  else
  {
    Serial.println("Unknown error");
  }
  // if (ret_code.length() > 0)
  // {
  //   processQMTCONNRET(ret_code);
  // }
}

void processQMTClientDisconnected(String message)
{
  Serial.println("Received +QMTDISC from Task 1 via Queue 1");
  // format: +QMTDISC: <client_id>,<result>
  int pos = message.indexOf(",");
  String client_id = message.substring(0, pos);
  String result = message.substring(pos + 1, message.length() - 1);
  if (result == "0")
  {
    Serial.println("Connection closed");
    closeConnection();
  }
  else if (result == "-1")
  {
    Serial.println("Failed to close connection");
  }
  else
  {
    Serial.println("Unknown error");
  }
}