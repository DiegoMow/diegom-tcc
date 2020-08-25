/**
 * ParkMe
 * Code to run the Park Me project as a Main controller of many park sensors
 *
 * Arduino:
 *  Uno R3
 *
 * Components:
 *  EthernetShield v1.0
 *  LDR - Light Dependent Resistor
 *  LED - Light Emissor Diode (colors: Red and Green)
 */

/* Libraries */
#include <SPI.h>
#include <Ethernet.h>

/* Default Values */
#define SENSOR_COUNT 4
#define DIGITAL_PORT_MIN 2
#define SET_POINT 20
#define SERVER_RESPONSE_TIMEOUT 1000

/* User Defined Types */
struct sensor {
  int analogPort;
  int redPort;
  int greenPort;
  int myValue;
  boolean myStatus;
};

/* Global Variables */
// sensor -> array with our sensor ports
sensor sensors[SENSOR_COUNT];
// lot -> status of each port
int lot[SENSOR_COUNT];

// mac -> Our mac address
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
// ip -> Our ip Address
IPAddress ip(192, 168, 0, 177);
// client -> our client connector
EthernetClient client;
// server -> name/ip of our domain
// char server[] = "http://parkmeeasy.mybluemix.net/sensor";
char server[] = "192.168.0.172";

/* Default Functions */
void setup() {
  // The Serial port is used to generate a log for us
  Serial.begin(9600);
  Serial.println("Serial Communication: OK");
  // Initialize the ethernet client
  connectToEthernet();
  // Initialize our sensors
  initializeSensors();
}

void loop() {
  // Read the value in the sensors
  readSensors();
  // Send the Data Via Json
  sendJson();
}

/* Custom Functions */
// connectToEthernet -> Only go out of this function if enable the MAC address
void connectToEthernet() {
  Serial.println("Tentativa de conexao na rede com Ip Statico");
  Ethernet.begin(mac, ip);
  delay(1000);
  Serial.println("Rede Disponivel");
}

// initializeSensors -> define the ports that have sensors in it
void initializeSensors() {
  Serial.println("Ligando sensores");
  int i = 0;
  for (i = 0; i < SENSOR_COUNT; i++) {
    sensors[i].analogPort = i;
    // The math below is used to assing digital ports to the LED usage
    sensors[i].redPort = DIGITAL_PORT_MIN + (i * 2);
    sensors[i].greenPort = DIGITAL_PORT_MIN + (i * 2) + 1;
    // Set the pinMode for Digital Pins
    pinMode(sensors[i].redPort, OUTPUT);
    pinMode(sensors[i].greenPort, OUTPUT);
    // Pre-sets value to read as an avaiable spot
    digitalWrite(sensors[i].greenPort, HIGH);
    digitalWrite(sensors[i].redPort, LOW);
    sensors[i].myStatus = false;
    sensors[i].myValue = 0;
    // Correct the status of the pins
    Serial.println("Sensor [" + String(i) + "] : " + String(sensors[i].analogPort));
  }
  Serial.println("");
}

// readSensors -> read the value for all the sensors
void readSensors() {
  int i = 0;
  String spotAvaiable;
  Serial.println("Lendo Sensores");
  for (i = 0; i < SENSOR_COUNT; i++) {
    sensors[i].myValue = analogRead(sensors[i].analogPort);
    if (SET_POINT > sensors[i].myValue) {
      digitalWrite(sensors[i].redPort, HIGH);
      digitalWrite(sensors[i].greenPort, LOW);
      sensors[i].myStatus = true;
      spotAvaiable = "Not avaiable :(";
    }
    else {
      digitalWrite(sensors[i].redPort, LOW);
      digitalWrite(sensors[i].greenPort, HIGH);
      sensors[i].myStatus = false;
      spotAvaiable = "Avaible!!! \\o/";
    }
    Serial.println("Sensor " + String(i) + ": " + spotAvaiable);
    Serial.println(" LDR: " + String(sensors[i].myValue));
    Serial.println(" LEDg: " + String(digitalRead(sensors[i].greenPort)));
    Serial.println(" LEDr: " + String(digitalRead(sensors[i].redPort)));
  }
}

// getMacAddres -> Convert the Bynary value of the Mac Address to a String
String getMacAddress() {
  String macString = "";
  for (int i = 0; i < 6; i++) {
    String hex = String(mac[i], HEX);
    if (hex.length() == 1) {
      hex = "0" + hex;
    }
    macString += hex;
    if (i < 5) {
      macString += ":";
    }
  }
  return macString;
}

// sendJson -> Send the Json to the server
void sendJson() {
  String key, value;
  String jsonString = getJsonString();
  Serial.println("Trying to connect to the server and send information");
  if (client.connect(server, 3001)) {
    Serial.println(F("Server Connected!! Preparing Json Post request"));

    client.println("POST /sensor HTTP/1.1");
    client.println("Content-Type: application/json");
    client.println("Host: " + String(server));
    client.println("User-Agent: arduino-ethernet");
    client.print("Content-Length: ");
    client.println(jsonString.length());
    client.println("Connection: close");
    client.println("");
    client.println(jsonString);
    client.println("");

    Serial.println("Date sent to the server \\o/");
  }

  serverResponse(SERVER_RESPONSE_TIMEOUT);
}

// getJsonString -> Return the json formated with the Values
String getJsonString() {
  String dataString = "{\"content\":{";
  for (int i = 0; i < SENSOR_COUNT; i++) {
    if (i != 0) {
      dataString += ",";
    }
    dataString += "\"Sensor_" + String(i) + "\":{";
    dataString += "\"status\":" + String(sensors[i].myStatus);
    dataString += ",\"analogPort\":\"" + String(sensors[i].analogPort) + "\"";
    dataString += ",\"analogPortValue\":" + String(sensors[i].myValue);
    dataString += ",\"redPort\":\"" + String(sensors[i].redPort) + "\"";
    dataString += ",\"redPortValue\":" + String(digitalRead(sensors[i].redPort));
    dataString += ",\"greenPort\":\"" + String(sensors[i].greenPort) + "\"";
    dataString += ",\"greenPortValue\":" + String(digitalRead(sensors[i].greenPort));
    dataString += "}";
  }
  dataString += "}}";
  Serial.println("Json to send: ");
  Serial.println(dataString);
  return dataString;
}

// serverResponse -> wait for the answer from our server
void serverResponse(long totalTimeOut) {
  String message = "";
  long timeoutStart = millis();
  while (!client.available()) {
    if ((millis() - timeoutStart) > totalTimeOut) {
      Serial.println("Timeout: waiting the response");
      client.stop();
      break;
    }
  }
  while (client.available()) {
    char c = client.read();
    message += c;
  }
  if (!client.connected()) {
    client.stop();
  }
  Serial.println(message);
}
