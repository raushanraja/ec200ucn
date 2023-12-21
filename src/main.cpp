#include <mqtt_helper.cpp>
// Define constants for UART communication
#define LTE_RX_PIN D6    // Replace with your ESP32 pin connected to LTE module RX
#define LTE_TX_PIN D7    // Replace with your ESP32 pin connected to LTE module TX
#define BAUD_RATE 115200 // Replace with the appropriate baud rate

#define MESSAGE_LENGTH 100
#define QUEUESIZE 40

#define PrintATSerailResponse 0

#define clientIdx 0
#define ssl_ctx_id 0
#define SSL_CERT_NAME "hive"

// Function prototypes
void task1(void *parameter);
void task2(void *parameter);

// Queues for inter-task communication
QueueHandle_t queue1;
QueueHandle_t queue2;

struct QueueData
{
  char message[MESSAGE_LENGTH];
  int length;
};

bool task2Started = false;

void sendToQueue(QueueHandle_t queue, const char *msg, int len)
{
  QueueData update;
  update.length = len > MESSAGE_LENGTH ? MESSAGE_LENGTH : len;
  strncpy(update.message, msg, update.length);
  update.message[update.length] = '\0'; // Ensure null-terminated
  xQueueSend(queue, &update, portMAX_DELAY);
}

void sendTask2Started()
{
  if (!task2Started)
  {
    sendToQueue(queue1, "T2S", 3);
    task2Started = true;
  }
}

void setup()
{
  // Initialize Serial for debugging
  delay(1000);
  Serial.begin(9600);
  ATSerial.begin(115200, SERIAL_8N1, LTE_RX_PIN, LTE_TX_PIN);
  delay(5000);
  Serial.println("Initializing...");

  // Initialize Queues
  queue1 = xQueueCreate(QUEUESIZE, sizeof(QueueData)); // Queue for Task 1 updates
  queue2 = xQueueCreate(QUEUESIZE, sizeof(QueueData)); // Queue for Task 2 UART responses

  // Create Task 1
  xTaskCreate(task1, "Task1", 10000, NULL, 1, NULL);

  // Create Task 2
  xTaskCreate(task2, "Task2", 10000, NULL, 1, NULL);
}

void loop()
{
  // Empty as FreeRTOS is managing tasks
}

void task1(void *parameter)
{
  Serial.println("Task 1 started");
  // Add AT Ready Check to Queue 2 for Task 2 reference
  String readyCheck = "AT Ready Check";
  String command = "";
  xQueueSend(queue2, &readyCheck, portMAX_DELAY);

  while (1)
  {
    // Wait for updates from Task 2 via Queue 1
    QueueData update; // <-- Declaration of 'update' variable missing here
    if (xQueueReceive(queue1, &update, portMAX_DELAY) == pdPASS)
    {
      String message = String(update.message);
      if (message == "T2S")
      {
        Serial.println("Received T2S from Task 2 via Queue 1, Checking AT");
        ATSerial.println("AT");
      }
      // check if it start with +CREG or +CEREG
      else if (message.startsWith("+CREG") || message.startsWith("+CGREG"))
      {
        Serial.println("Received +CREG or +CGuREG from Task 2 via Queue 1");
      }
      else if (message.startsWith("OK"))
      {
        switch (lastATCommand)
        {
        case LastATCommand::AT:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending AT");
          lastATCommand = LastATCommand::QMTCFG_VERSION;
          ATSerial.println(mqttClient.configureProtocolVersion(clientIdx, "3"));
          break;
        case LastATCommand::QMTCFG_VERSION:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending QMTCFG");
          lastATCommand = LastATCommand::QMTCFG_SSLENABLE;
          ATSerial.println(mqttClient.configureSSLEnable(clientIdx, true, String(ssl_ctx_id)));
          break;
        case LastATCommand::QMTCFG_SSLENABLE:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending QMTCFG");
          lastATCommand = LastATCommand::QMTCFG_RECV;
          ATSerial.println(mqttClient.configureRecvMode(clientIdx, "0", "1"));
          break;
        case LastATCommand::QMTCFG_RECV:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending QMTCFG");
          lastATCommand = LastATCommand::QSSLCFG_SSLVER;
          ATSerial.println(mqttClient.configureSSLVersion(ssl_ctx_id, "4"));
          break;
        case LastATCommand::QSSLCFG_SSLVER:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending QSSLCFG_SSLVER");
          lastATCommand = LastATCommand::QSSLCFG_CIPHER;
          ATSerial.println(mqttClient.configureSSLCipher(ssl_ctx_id, "0XFFFF"));
          break;
        case LastATCommand::QSSLCFG_CIPHER:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending QSSLCFG_CIPHER");
          lastATCommand = LastATCommand::QSSLCFG_SECLEVEL;
          ATSerial.println(mqttClient.configureSSLSecLever(ssl_ctx_id, 0));
          break;
        case LastATCommand::QSSLCFG_SECLEVEL:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending QSSLCFG_SECLEVEL");
          lastATCommand = LastATCommand::QSSLCFG_CACERT;
          ATSerial.println(mqttClient.configureSSLCert(ssl_ctx_id, SSL_CERT_NAME));
          break;
        case LastATCommand::QSSLCFG_CACERT:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending QSSLCFG_CACERT");
          lastATCommand = LastATCommand::QSSLCFG_IGNOREINVALID;
          ATSerial.println(mqttClient.configureSSLIgnoreInValidCert(ssl_ctx_id, true));
          break;
        case LastATCommand::QSSLCFG_IGNOREINVALID:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending QSSLCFG_IGNOREINVALID");
          lastATCommand = LastATCommand::QSSLCFG_SNI;
          ATSerial.println(mqttClient.configureSSLSNI(ssl_ctx_id, "1"));
          break;
        case LastATCommand::QSSLCFG_SNI:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending QSSLCFG_SNI");
          lastATCommand = LastATCommand::QMTOPEN;
          openConnection();
          break;
        case LastATCommand::QMTOPEN:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending QMTOPEN");
          lastATCommand = LastATCommand::QMTCONN;
          break;
        case LastATCommand::QMTCLOSE:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending QMTCLOSE");
          lastATCommand = LastATCommand::QMTOPEN;
          openConnection();
          break;
        case LastATCommand::QMTDISC:
          Serial.println("Received OK from Task 2 via Queue 1 for Sending QMTDISC");
        default:
          break;
        }
      }
      else if (message.startsWith("+QMTRECV"))
        processQMTRECV(message);
      else if (message.startsWith("+QMTOPEN"))
        processQMTOPEN(message);
      else if (message.startsWith("+QMTSTAT"))
        processQMTSTAT(message);
      else if (message.startsWith("+QMTCONN"))
        processQMTCONN(message);
      else if (message.startsWith("+QMTDISC"))
        processQMTClientDisconnected(message);
      else if (message.startsWith("+QMTCLOSE"))
        processQMTCLOSE(message);
      else
      {
        Serial.println("Received update from Task 2 via Queue 1: " + String(update.message));
      }
      // Process updates received from Task 2
      // Add your processing logic here
    }
    vTaskDelay(10);
  }
}

void task2(void *parameter)
{
  Serial.println("Task 2 started");
  String response;

  while (1)
  {
    // Check for UART response from LTE module
    if (ATSerial.available())
    {
      response = ATSerial.readStringUntil('\n');

      // Check Length
      if (response.length() > 2)
      {
        if (PrintATSerailResponse)
          Serial.println("Received response from LTE module: " + response);

        // Make sure response length is less than MESSAGE_LENGTH
        if (response.length() > MESSAGE_LENGTH)
        {
          response = response.substring(0, MESSAGE_LENGTH);
        }

        QueueData update;
        update.length = response.length() + 1; // +1 for null terminator
        strncpy(update.message, response.c_str(), MESSAGE_LENGTH);
        update.message[MESSAGE_LENGTH - 1] = '\0'; // Ensure null-terminated
        // Send response to Task 1 via Queue 1
        xQueueSend(queue1, &update, portMAX_DELAY);

        // Check if respone if in PublisableATCommands
        for (int i = 0; i < sizeof(PublishableATCommands) / sizeof(PublishableATCommands[0]); i++)
        {
          if (response.startsWith(PublishableATCommands[i]))
          {
            Serial.println("Publishing AT command: " + response);
            String pubmessage = mqttClient.publish(clientIdx, 0, 0, 0, "ATINFO", response.length() + 1);
            Serial.println("Published message: " + pubmessage);
            ATSerial.println(pubmessage);
            delay(10);
            ATSerial.println(response);
          }
        }
      }
    }
    sendTask2Started();
    vTaskDelay(100); // Adjust delay as needed
  }
}
