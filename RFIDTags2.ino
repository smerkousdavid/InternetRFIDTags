/*
 * ----------------------------------
 *             MFRC522     Arduino   
 *             Reader/PCD  Nano v3   
 * Signal      Pin         Pin      
 * ----------------------------------
 * RST/Reset   RST        D9  
 * SPI SS      NSS        D10 
 * SPI MOSI    MOSI       D11  
 * SPI MISO    MISO       D12 
 * SPI SCK     SCK        D13  
 */


#include <UIPEthernet.h>

#include <SPI.h> //For the selection of the key
#include <MFRC522.h> //The RFID key library

#define RST_PIN         9           // Configurable, see typical pin layout above - This is for the Arduino Nano - For RFID
#define SS_PIN          8

  byte sector         = 0;
  byte blockAddr      = 0; ////////Access certain sector/blocks in the card, trailer block is the last block
  byte trailerBlock   = 1;

int red = 3;
int blue = 4; //Pins for RGB LED
int green = 5;

EthernetClient client;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key; //Set key instance

signed long timeout;

void setup()
{
   pinMode(red, OUTPUT);
   pinMode(blue, OUTPUT);
   pinMode(green, OUTPUT);
   digitalWrite(red, HIGH);
   digitalWrite(blue, HIGH);
   digitalWrite(green, HIGH);
  Serial.begin(9600); //Start computer connection with a rate of 9600 bits per second
  while(!Serial); //Wait until computer is connected
  SPI.begin();        // Init SPI bus
  uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
  IPAddress mip(10,0,0,150);
  IPAddress mdns(8,8,8,8);
  IPAddress mgate(10,0,0,1);
  IPAddress msubnet(255,255,255,0);
  Ethernet.begin(mac, mip, mdns, mgate , msubnet);
  delay(20);
  Serial.println("Succesful connection");
  mfrc522.PCD_Init(); // Init MFRC522 card
  
     for (byte i = 0; i < 6; i++) {   // Prepare the key (used both as key A and as key B)
        key.keyByte[i] = 0xFF;        // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
        }
    
    Serial.println(F("Scan a Card"));
    dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);     //Get key byte size
   timeout = 0;
   digitalWrite(red, LOW);
   digitalWrite(blue, LOW);
   delay(1000);
   digitalWrite(red, HIGH);
   digitalWrite(blue, HIGH);
   delay(1000);
   digitalWrite(red, LOW);
   digitalWrite(blue, LOW);
   delay(1000);
   digitalWrite(red, HIGH);
   digitalWrite(blue, HIGH);     // More of show, but also just incase let the RFID and ETHERNET modules boot up before a key command and it crashing
   delay(1000);
   digitalWrite(red, LOW);
   digitalWrite(blue, LOW);
   digitalWrite(green, LOW);
   delay(2000);
   digitalWrite(red, HIGH);
   digitalWrite(blue, HIGH);
   digitalWrite(green, HIGH);
}

void loop() //Run forever
{
  // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent())
    {
        digitalWrite(blue, LOW);
        return;
    }

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;

        digitalWrite(blue, HIGH);
    // Show some details of the PICC (that is: the tag/card)
    //Serial.print(F("PICC type: "));
    byte piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

    // Check for compatibility
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        //Serial.println(F("This only works with 320B cards 1k cards or 4k cards Classic cards."));
        digitalWrite(red, LOW);
        delay(700);
        digitalWrite(red, HIGH);
        return;
    }
    
  byte status;
  byte buffer[18];
  byte size = sizeof(buffer);


  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    digitalWrite(red, LOW);
    delay(700);
    digitalWrite(red, HIGH);
    return;
  }

  
  // Read data from the block
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    digitalWrite(red, LOW);
    delay(700);
    digitalWrite(red, HIGH);
    return;
  }
      // Halt PICC 
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
// Send to server
      if (client.connect(IPAddress(10,0,0,98),23))
        {
          timeout = millis()+1000;
          Serial.println("Client connected");
          const String ID = dump_byte_array(buffer, size);
          client.println(ID);
          Serial.println("sent :" + ID);
          delay(10);
          while(client.available()==0)
            {
              if (timeout - millis() < 0)
                goto close;
            }
          int size;
          while((size = client.available()) > 0)
            {
              uint8_t* msg = (uint8_t*)malloc(size);
              size = client.read(msg,size);
              Serial.write(msg,size);
              if(size == sizeof("g") - 1)
              {
                    digitalWrite(green, LOW);
                    delay(700);
                    digitalWrite(green, HIGH);
              }
              else
              {
                    digitalWrite(red, LOW);
                    delay(700);
                    digitalWrite(red, HIGH);
              }
              free(msg);
            }
close:
          client.stop();
        }
        else
        {
    Serial.println("Couldn't connect to Server");
         digitalWrite(red, LOW);
         digitalWrite(green, LOW);
        delay(700);
        digitalWrite(red, HIGH);
        digitalWrite(green, HIGH);
        }
   digitalWrite(red, HIGH);
   digitalWrite(blue, HIGH);
   digitalWrite(green, HIGH);
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
String dump_byte_array(byte *buffer, byte bufferSize) {
          String out = "";
    for (byte i = 0; i < bufferSize; i++) {
        //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        //Serial.print(buffer[i], HEX);
        out += String(buffer[i] < 0x10 ? " 0" : " ") + String(buffer[i], HEX);
    }
    out.toUpperCase();
    out.replace(" ", "");
    return out;
}
