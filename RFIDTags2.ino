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

// THE FIRST LIBRARY THAT NEEDS TO BE INSTALLED IS UIP ETHERNET SECOND IS MFRC522 BOTH ARE ON GITHUB

#include <UIPEthernet.h>

#include <SPI.h> //For the selection of the key
#include <MFRC522.h> //The RFID key library

#define RST_PIN         9           // Configurable, see typical pin layout above - This is for the Arduino Nano - For RFID
#define SS_PIN          8 //WE ARE USING 8 FOR RFID BECAUSE THE ETHERNET MODULE USES 10

  byte sector         = 0;
  byte blockAddr      = 0; ////////Access certain sector/blocks in the card, trailer block is the last block
  byte trailerBlock   = 1;

int red = 3;
int blue = 4; //Pins for RGB LED
int green = 5;

EthernetClient client;  //ETHERNET INSTANCE

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key; //Set key instance

signed long timeout; //TIMEOUT SO IT DOESN'T SIT THERE FOREVER

void setup()
{
  //UI BEGIN
   pinMode(red, OUTPUT);
   pinMode(blue, OUTPUT); //Init the RGB LED
   pinMode(green, OUTPUT);
   Reset(); //Start with leds off

  Serial.begin(9600); //Start computer connection with a rate of 9600 bits per second
  //UI END

  //ETHERNET MODULE INITIAL
  SPI.begin();        // Init SPI bus
  uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};     //MAC = 000102030405
  IPAddress mip(192,168,1,160);                         //IP = 192.168.1.160
  IPAddress mdns(8,8,8,8);                              //DNS = 8.8.8.8
  IPAddress mgate(192,168,1,5);                         //GATEWAY = 192.168.1.5
  IPAddress msubnet(255,255,255,0);                     //SUBNET = 255.255.255.0
  Ethernet.begin(mac, mip, mdns, mgate , msubnet);      //CONNECT USING ABOVE
  Serial.println("Succesful connection");
  // END OF ETHERNET
  
  for(int t = 255; t > 0; t--)
  {
    analogWrite(red, t);           ////More of show but let at least a second between the SPI of the ethernet and RFID
    delay(10);
  }

  //RFID INITIAL
  mfrc522.PCD_Init(); // Init MFRC522 card
  
     for (byte i = 0; i < 6; i++) {   // Prepare the key (used both as key A and as key B)
        key.keyByte[i] = 0xFF;        // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
        }
    
    Serial.println(F("Scan a Card"));
    dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);     //Get key byte size
   timeout = 0;  
   delay(2000);
   Reset();
}
//END RFID INITIAL

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

        digitalWrite(blue, HIGH); //Show user that card has been read


    byte piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

    // Check for compatibility with Mifare card
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Error();
        return;
    }
    
  byte status;
  byte buffer[18];
  byte size = sizeof(buffer);


  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    Error();
    return;
  }

  
  // Read data from the block
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    Error();
  }
      // Halt PICC 
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
    
// AFTER DONE READING CARD SEND TO SERVER
      if (client.connect(IPAddress(192,168,1,100),23))
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
                    Pass();
              }
              else
              {
                    Error();
              }
              free(msg);
            }
close:
          client.stop();
        }
        else
        {
    Serial.println("Couldn't connect to Server");
         Error();
        }
        //END OF SENDING TO SERVER
        
   Reset(); //RESTART LOOP WITH NO LEDS ON
}

// TURN THE BUFFER ARRAY INTO A SINGLE STRING THAT IS UPPERCASE WHICH EQUALS OUR ID OF THE SECTOR AND BLOCK
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
//END DUMP_BYTE_ARRAY

//BELOW ARE THE LED METHODS
void Error()
{
  digitalWrite(red, LOW);
  delay(700);
  digitalWrite(red, HIGH);
}

void Pass()
{
  digitalWrite(green, LOW);
  delay(700);
  digitalWrite(green, HIGH);
}

void Reset()
{
   digitalWrite(red, HIGH);
   digitalWrite(blue, HIGH);
   digitalWrite(green, HIGH);
}
//END OF FILE
