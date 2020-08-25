/*
* Typical pin layout used:
* -----------------------------------------------------------------------------------------
*             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
*             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
* Signal      Pin          Pin           Pin       Pin        Pin              Pin
* -----------------------------------------------------------------------------------------
* RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
* SPI SS      SDA(SS)      10            53        D10        10               10
* SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
* SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
* SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*
*/

#include <stdlib.h>
#include <Time.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include <SimpleTimer.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

#define SS_PIN  53
#define BT_CLOSE_DOR  4
#define RST_PIN 5
#define SERVER_IP "192.168.147.29"
#define LOCKER_ID "A1"

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = SERVER_IP;
IPAddress timeServer(200, 160, 7, 186);
const int timeZone = -2;

Bounce debouncerButton = Bounce();


EthernetClient client;
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

//EthernetUDP Udp;
//unsigned int localPort = 8888;

void setup() {
  SPI.begin();                // Init SPI bus
  mfrc522.PCD_Init();         // Init MFRC522 card
  lcd.init();
  lcd.backlight();
  pinMode(BT_CLOSE_DOR, INPUT);      // Init button

  // After setting up the button, setup the Bounce instance :
  debouncerButton.attach(BT_CLOSE_DOR);
  debouncerButton.interval(5); // interval in ms

  lcd.home();
  lcd.clear();
  lcd.print(" Device Manager");
  lcd.setCursor(0, 1);
  lcd.print("     versao 1.0");
  delay(1000);
  //lcd.home();
  lcd.clear();
  lcd.print(" Armario...:  " + String(LOCKER_ID));
  delay (1000);
  lcd.clear();
  lcd.print("Inicializando...");

  Serial.begin(9600);
  Serial.println("Comecando");
  while (!Serial) {
  }

  Serial.println("Tentando pegar IP Via DHCP");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac);
  }
  Serial.print("My IP address: ");

  String sIP = "";
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
    sIP += String(Ethernet.localIP()[thisByte]);
    sIP += ".";
  }
  Serial.println();
  Serial.println("Finalizou DHCP");


  lcd.clear();
  lcd.print("IP:");
  lcd.setCursor(0, 1);
  lcd.println(sIP);
  delay(1500);

  lcd.clear();
  lcd.println("Aguardando...  ");
  lcd.blink();
  delay(500);

  //  Udp.begin(localPort);
  //  setSyncProvider(getNtpTime);

}


time_t prevDisplay = 0;

int buttonPressed;
SimpleTimer timer;
int timerId = timer.setInterval(5000, checkOpenDoor);
void loop() {
  returnRfid();

  // Update the Bounce instance :
  debouncerButton.update();

  // Start SimpleTimer.
  timer.run();

  // Get the updated value :
  buttonPressed = debouncerButton.read();

  // Turn on or off the LED as determined by the state :
  if (buttonPressed != LOW) {
    timer.disable(timerId);
  }
  else {
    timer.enable(timerId);
  }

  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
  
}

void sendJSON(String dataString) {
  String msg;

  Serial.println("connecting...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.println("POST /device-manager/device-manager/device_manager HTTP/1.1");
    client.print("Host: "); client.println(SERVER_IP);
    client.println("Accept: application/json");
    client.println("User-Agent: ESPDU");
    client.println("Content-Type: application/json");
    int thisLength = dataString.length();
    client.print("Content-Length: ");
    client.println(thisLength);
    client.println("Connection: close");
    client.println();
    client.println(dataString);
    Serial.println("enviado");
    Serial.println(dataString);
    Serial.println("---");
  }
  else {
    Serial.println("connection failed");
  }

  delay(1550);
  while (client.available()) {
    char c = client.read();
    msg += c;
    if (c == '\n') {
      msg = "";
    }

  }
  if (!client.connected()) {
    client.stop();
  }
  Serial.println(msg);

  String aux = msg.substring(msg.indexOf("\"msg\":\"") + 7, msg.length() - 3);

  Serial.println(aux);
  lcd.clear();
  lcd.home();
  lcd.print(aux);
}


/*
 * Helper routine to dump a byte array as hex values to Serial.
 */
char* dump_byte_array(byte *buffer, byte bufferSize) {
  char rfid[4];
  char *saida = (char*) calloc(60, sizeof(char));
  for (byte i = 0; i < bufferSize; i++) {
    sprintf(rfid, "%X", buffer[i]);
    strcat(saida, rfid);
  }
  return saida;
}

/**
 * Read RFID
 */
void returnRfid() {
  char id[50];
  char *dump;
  int n;

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  lcd.clear();
  lcd.println("RFID detectado");

  dump = dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  lcd.println(String(dump));
  n = sprintf (id, "{\"locker_id\": \"%s\", \"rfid\": \"%s\"}", LOCKER_ID, dump);

  sendJSON(id);
  //Serial.println(id);
  delay(3000);
}

void digitalClockDisplay() {

  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();


  lcd.clear();
  lcd.print("Data: ");
  lcd.print(hour());
  lcd.print(printDigits(minute()));
  lcd.print(printDigits(second()));
  lcd.setCursor(0, 1);
  lcd.print(day());
  lcd.print(".");
  lcd.print(month());
  lcd.print(".");
  lcd.print(year());
}

String printDigits(int digits) {
  // utility for digital clock display: prints preceding colon and leading 0
  String retorno = ":";

  if (digits < 10)
    retorno += "0";

  retorno += String(digits);
  return retorno;
}

/*-------- NTP code ----------*/
/*
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
*/

/**
 *  Check if the door is open after 5s open.
 */
void checkOpenDoor() {
  Serial.println("start: checkOpenDoor");
  sendJSON("Teste");
}
