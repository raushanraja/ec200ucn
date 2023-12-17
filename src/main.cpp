#include <Arduino.h>
#include <mqtt.h>
// Define constants for UART communication
#define LTE_RX_PIN D6    // Replace with your ESP32 pin connected to LTE module RX
#define LTE_TX_PIN D7    // Replace with your ESP32 pin connected to LTE module TX
#define BAUD_RATE 115200 // Replace with the appropriate baud rate

#define MESSAGE_LENGTH 100
#define QUEUESIZE 40

#define PrintATSerailResponse 0

// Function prototypes
void task1(void *parameter);
void task2(void *parameter);

HardwareSerial ATSerial(1);
MQTT mqttClient(ATSerial);

// Queues for inter-task communication
QueueHandle_t queue1;
QueueHandle_t queue2;

enum LastATCommand
{
  AT,
  QMTCGF,
  QMTOPEN
};

LastATCommand lastATCommand = AT;

struct QueueData
{
  char message[MESSAGE_LENGTH];
  int length;
};

bool task2Started = false;
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
  }
  else if (result == "1")
  {
    Serial.println("Wrong parameter");
  }
  else if (result == "2")
  {
    Serial.println("MQTT identifier is occupied");
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
  mqttClient.begin(115200, SERIAL_8N1, LTE_RX_PIN, LTE_TX_PIN);
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
        Serial.println("Received T2S from Task 1 via Queue 1, Checking AT");
        ATSerial.println("AT");
      }
      // check if it start with +CREG or +CEREG
      else if (message.startsWith("+CREG") || message.startsWith("+CGREG"))
      {
        Serial.println("Received +CREG or +CGuREG from Task 1 via Queue 1");
      }
      else if (message.startsWith("OK"))
      {
        switch (lastATCommand)
        {
        case LastATCommand::AT:
          Serial.println("Received OK from Task 1 via Queue 1 for Sending AT");
          lastATCommand = LastATCommand::QMTCGF;
          char command[50];
          snprintf(command, sizeof(command), "AT+QMTCFG=\"version\",%d,%d", 0, 3);
          ATSerial.println(command);
          break;
        case LastATCommand::QMTCGF:
          Serial.println("Received OK from Task 1 via Queue 1 for Sending QMTCGF");
          lastATCommand = LastATCommand::QMTOPEN;
          ATSerial.println("AT+QMTOPEN=0,\"io.adafruit.com\",1883");
          break;
        case LastATCommand::QMTOPEN:
          Serial.println("Received OK from Task 1 via Queue 1 for Sending QMTOPEN");
        default:
          break;
        }
      }
      else if (message.startsWith("+QMTOPEN"))
        processQMTOPEN(message);
      else if (message.startsWith("+QMTSTAT"))
        processQMTSTAT(message);
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
        xQueueSend(queue1, &update, portMAX_DELAY);
        // Send response to Task 1 via Queue 1
      }
    }
    sendTask2Started();
    vTaskDelay(10); // Adjust delay as needed
  }
}
