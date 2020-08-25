//Programa : Sistema de Acesso com RFID - Arduino Uno
//Autor : Diego Ocko Martins
//Adaptado de: RFID - Controle de acesso do RFID (FLIPEFLOP)
 
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
 
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
 
LiquidCrystal lcd(6, 7, 5, 4, 3, 2); 
 
char st[20];
int Buzzer = 8; //Pino do Buzzer
 
void setup() 
{
  Serial.begin(9600);   // Inicia a serial
  SPI.begin();      // Inicia  SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522
  pinMode(Buzzer,OUTPUT);
  Serial.println("Start Ok!");
  Serial.println();
  //Define o número de colunas e linhas do LCD:  
  lcd.begin(16, 2);  
  mensageminicial();
}
 
void loop() 
{
  // Se tiver uma Tag
  if (mfrc522.PICC_IsNewCardPresent()) 
  {
    // Se ele conseguiu ler a Tag
    if (mfrc522.PICC_ReadCardSerial()) 
    {
      String conteudo = "";
      String conteudo2 = "";
      byte letra;
      //Mostra UID da TAG na serial
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
        //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        //Serial.print(mfrc522.uid.uidByte[i], HEX);
        conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
        conteudo2.concat(" " + String(mfrc522.uid.uidByte[i]));
      }
      conteudo.toUpperCase();
      Serial.print("UID direto :" + conteudo2+"\n");
      Serial.print("UID Hexa   :" + conteudo+"\n");
      mensagem("D"+conteudo2,"Hex"+conteudo);
      digitalWrite(Buzzer,HIGH);
      delay(1000);    
      digitalWrite(Buzzer,LOW);
      mensageminicial();
    }
  }
} 
 
void mensageminicial()
{
  lcd.clear();
  lcd.print(" Aproxime o seu");  
  lcd.setCursor(0,1);
  lcd.print("cartao do leitor");  
}

void mensagem(String Linha1,String Linha2)
{
  lcd.clear();
  lcd.print(Linha1);  
  lcd.setCursor(0,1);
  lcd.print(Linha2);  
}
