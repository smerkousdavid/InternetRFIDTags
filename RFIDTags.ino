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

#include <UIPEthernet.h> //Only needed for the cheaper ENC2j28 ethernet module

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
  IPAddress mgate(192,168,1,1);                         //GATEWAY = 192.168.1.1 (Default for most routers)
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
  
     for (byte i = 0; i < 6; i++) {   // Prepare the key which is 6 of 0xFF
        key.keyByte[i] = 0xFF;        // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
        }
    
    Serial.println(F("Scan a Card"));
    dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);     //Get key byte size Usually (6)
   timeout = 0;  
   delay(1000); //Wait for module bootup(They should already be done but just to be safe)
   Reset(); //Turn all lights off
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
        Error(); //Error light
        return; //Don't run anything below
    }
    
  byte status;
  byte buffer[18]; //Buffer amount of 18 bytes, which is a about a half a block worth
  byte size = sizeof(buffer);


  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: ")); //Remember the FFFFF... You need to identify the card before accessing
    //Serial.println(mfrc522.GetStatusCodeName(status)); error
    Error(); //Error light
    return;
  }

  
  // Read data from the block
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size); //Read the addr with a buffer 2 times (18bytes)
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    //Serial.println(mfrc522.GetStatusCodeName(status)); error
    Error();
  }
      // Halt PICC 
    mfrc522.PICC_HaltA();//Stop the SPI comm with the card so you can free the line to the ethernet controller
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
    
// AFTER DONE READING CARD SEND TO SERVER
      if (client.connect(IPAddress(192,168,1,100),23)) //port <1000 is priveleged, MAKE SURE FIREWALL IS OFF
        {
          timeout = millis()+1000; //Get current time (since boot) and add 1000ms for a 1 second timeout
          Serial.println("Client connected");
          const String ID = dump_byte_array(buffer, size);
          client.println(ID);
          Serial.println("sent :" + ID);
          delay(10);
          while(client.available()==0)
            {
              if (timeout - millis() < 0) //If greater than one second just leave
                goto close;
            }
          int size;
          while((size = client.available()) > 0) //if pass earlier test have a 30 second timeout
            {
              uint8_t* msg = (uint8_t*)malloc(size);
              size = client.read(msg,size); //Read the memorry allocated msg, with the allocation to (size = client)
              Serial.write(msg,size);
              if(size == sizeof("g") - 1)
              {
                    Pass(); //Finally you pass every test and get a response from the computer
              }
              else
              {
                    Error(); //Dang the card number didn't match any in the SQL server
              }
              free(msg);//Clear the allocated memory
            }
close:
          client.stop(); //Close the TCP socket
        }
        else
        {
    Serial.println("Couldn't connect to Server"); //If passed the 30 second timeout just show you coudn't connect to server
         ConnectionError(); //Flash repediatly to show there was an error connecting to the TCP server
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
    out.toUpperCase(); //Make all cards uppercase, because the buffered reading will make them lower case
    out.replace(" ", ""); //No spaces so cards don't get wacky length
    return out; //Return the dump_byte_array String which is the ID of the card
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

void ConnectionError()
{
  digitalWrite(red, LOW);
  delay(400);
  digitalWrite(red, HIGH);
  delay(400);
  digitalWrite(red, LOW);
  delay(400);
  digitalWrite(red, HIGH);
  delay(400);
  digitalWrite(red, LOW);
}
}
//END OF FILE
