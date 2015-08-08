
#include <ESP8266.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

//Values most likely needed to be edited//
#define SSID        "David"       //Your router SSID
#define PASSWORD    "Alex12345"    //Your router Password
#define serverIP    "192.168.0.17" //Your modules IP
#define gateWay     "192.168.0.1"  //Gateway
#define PORT        8080          //The port that your using(Make sure your router is port forwarded for your phone)
#define debug       true          //Put false if youre arduino is not connected to your computer
#define COMBaud     115200        //Put the computer baudrate(make sure in Serial monitor it's also 115200)
#define ESPBaud     9600          //Don't mess with this unless either switching to hardware serial or ESP only supports 115200
#define RGBs        true          //If you have RGB LED connected   //MAKE SURE YOUR ESP AND ARDUINO HAVE THE SAME BAUDRATE!(Check first)
#define RedGB       3             //Pin for red LED
#define RGreenB     5             //Pin for Green LED
#define RGBlue      4             //Pin for Blue LED
#define Solenoid    11            //The solenoid pin
#define GarageOCT   8000          //The time in Ms for the garage open and close time (Usually 8 seconds)
#define defaultState false        //Once powered up what state will the garage be...(false=Open, true=Close)
#define ServerTimeout  13         //The time it takes for 
//End of most likely

SoftwareSerial ESPSerial(10, 11); //Arduino Rx to Tx on ESP, ESP Rx to Resistors to Arduino Tx

ESP8266 wifi(ESPSerial, ESPBaud); //This library was modified so just use the one I am using to make it work

bool state; //Set value above to global non static value
int first = 0;
int baudaddr = 1;
int states = 2;
byte firstes;
byte baudaddres;
byte stateses;

void setup()
{
    firstes = EEPROM.read(first);
    baudaddres = EEPROM.read(baudaddr);
    stateses = EEPROM.read(states);
    if(RGBs) //If RGBs are enabled open the pins
    {
      pinMode(RedGB, OUTPUT);
      pinMode(RGreenB, OUTPUT);
      pinMode(RGBlue, OUTPUT);
      off();
    }
    if(debug) //Option above for computer debugging
    {
    Serial.begin(COMBaud); //Start at Buadrate above
    while(!Serial);
    delay(500); //Since Serial is Asyncronous sometimes you might get some jumbled unwanted crap from the arduino
    Serial.println("Started Garage System\nGoing Station mode");
    Serial.println("Current Version: " + (String) wifi.getVersion().c_str()); //Get all the board info such as manufacturer 
    }
    else
    {
      wifi.getVersion().c_str(); //This has to be runned anyway
    }
    if (wifi.setOprToStation()) { //Make sure when switching modes everything goes right
        if(debug)
        {
        Serial.println("Station Mode okay");
        }
    } else {
        if(debug)
        Serial.println("Station Mode Not okay");
        error();
    }
    if(firstes != 5)
    {
      if(baudaddres != ESPBaud)
    {
      if(wifi.setUart(ESPBaud, 2))
      {
        if(debug)
        Serial.println("Welcome first I automatically changed the baud for you");
        
      }
      else
      {
        if(debug)
        Serial.println("Sorry first timer, there was an error changing the baud");
        error();
      }
      EEPROM.write(baudaddr, ESPBaud);
    }
    if(stateses != 1 || stateses != 2)
    {
      if(!defaultState)
      {
        EEPROM.write(states, 1);
        states = false;
      }
      else
      {
        EEPROM.write(states, 2);
        states = true;
      }
    }
    EEPROM.write(first, 5);
    }
    else
    {
      if(stateses == 1)
      {
        states = false;
      }
      else
      {
        states = true;
      }
    }
    if(debug)
       Serial.println("Attempting to connect to router");
    if (wifi.joinAP(SSID, PASSWORD)) { //Connect to a router with specified SSID and PASSWORD
        if(debug)
        {
        Serial.println("Connection succesful\nIp Address");
        Serial.println(wifi.getLocalIP().c_str()); //Print current address for easier use
        }
        else
        {
          wifi.getLocalIP().c_str(); //Just to be safe call this command even when debug isn't on
        }
    } else {
        if(debug)
        Serial.println("Connection failed");
        error();
    }
    if(debug)
    Serial.println("Setting mode to static");
    if(wifi.setStationIp(serverIP, gateWay, "255.255.255.0"))
    {
      if(debug)
      Serial.println("Success to moving into static");
    }
    else
    {
      if(debug)
      Serial.println("Error going into static mode");
      error();
    }
    if(debug)
       Serial.println("Trying to allow multiple clients");
    if (wifi.enableMUX()) { //AT+MUX= 0 or 1, 0 meaning only one client 1 meaning multiple
        if(debug)
        Serial.println("Multiple activated");
    } else {
        if(debug)
        Serial.println("Multiple activation error");
        error();
    }
    
    if (wifi.startTCPServer(PORT)) { //Start a TCP server the AT command for this is AT+CIPSTART="TCP", PORT
        if(debug)
        Serial.println("Started TCP server on port: " + (String) PORT);
    } else {
        if(debug)
        Serial.println("Failed to start on port: " + (String) PORT);
        error();
    }
    
    if (wifi.setTCPServerTimeout(ServerTimeout)) { //Set a timeout for a packet so it doens't just sit there stupidly
        //if(debug)
        //Serial.println("Set timeout for: " + (String) ServerTimeout);
    } else {
        if(debug)
        Serial.println("Couldn't set up timeout");
        error();
    }
    if(debug)
    Serial.println("Setup complete READY for input\n\n\nThe Current garage State is " + (String) state);
    start();
}
 
void loop()
{
    uint8_t buffer[128] = {0}; //The buffer (Array) to insert from TCP packet
    uint8_t Client_ID;         //Init var for the Client ID
    String packet = "";        //The final outcome of the packet
    uint32_t lengths = wifi.recv(&Client_ID, buffer, sizeof(buffer), 100); //Return the length of the packet
    if (lengths > 0) { //If larger than zero than move along
        if(debug) 
        Serial.println("Status" + (String) wifi.getIPStatus().c_str() +"\nFrom Client " + Client_ID); //Send current connection status
        for(uint32_t i = 0; i < lengths; i++) {
            packet += (String)((char)buffer[i]); //Place the lengths into a buffer
        }
        recP(); //Indicate with RGBs
        if(debug)
         Serial.println("The Packet was: " + packet);
         int ifNum = ifEqual(packet); //Look at method below
        if(ifNum == 0)
        {
          //Open packet
          for(int a = 0; a < GarageOCT; a++) //Wait for garage to change state
            delay(1);
          state = !state;
          sendPacket(1, Client_ID);
          pass();
        }
        else if(ifNum == 1)
        {
         //GetState packet
         int sends = 2; //Opened
          if(state) //If closed
          {
            sends = 3; //Closed
          }
          delay(800);
          sendPacket(sends, Client_ID);
          pass();
        }
        else if(ifNum == 2)
        {
          pass();
          if(debug)
          Serial.println("Device received correct packet");
        }
        else
        {
          
        }
        Serial.print("Status:[");
        Serial.print(wifi.getIPStatus().c_str());
        Serial.println("]");
    }
}

int ifEqual(String pack) { //Just a shorter way of making a comparison with strings and outputing a number for each index of a string array
  String values[3] = {"open", "getstate", "gotit"}; //Open = 0, getstate = 1, gotit = 2, addmore...
  for(int a = 0; a < 3; a++)
  {
    if(values[a] == pack)
    {
      if(debug)
      Serial.println("I got a " + values[a] + " Packet"); 
      return a;
      break;
    }
  }
  return 9;
}

bool sendPacket(int num, uint8_t Client_ID){  //A Function where you can integers back
      sendP();
      bool pass = false; 
      uint8_t buffer[1] = {0}; //Stick in a buffer that is only the size of 1;
      if(num == 0)
      {
      buffer[0] = (char)'0';
      }
      else if(num == 1)
      {
        buffer[0] = (char)'1';   //Putting directly got sometimes buggy so i did some if statements
      }
      else if(num == 2)
      {
        buffer[0] = (char)'2';
      }
      else if(num == 3)
      {
        buffer[0] = (char)'3';
      }
      
        if(wifi.send(Client_ID, buffer, 1)) { //wifi.send(<Id>,<buffer>,<size>)
         if(debug)
         Serial.println("Sent Back okay");
         pass = true;
        } else {
            if(debug)
            Serial.println("There was an error sending the packet");
            pass = false;
        }
        
        if (wifi.releaseTCP(Client_ID)) { //If using Java this is needed because stream reader has to get closed to actually finish reading if you modified this for the computer (Python, etc...) remove it because it will show an error
            if(debug)
            Serial.println("Realeased Client " + (String) Client_ID + "Success");
            pass = true;
        } else {
            if(debug)
            Serial.print("There was an Error trying to release Client " + Client_ID);
            pass = false;
        }
        stopP();
        return pass;
}

//Below are all the RGBs for "light" debugging
void error() {
  if(RGBs)
  {
  digitalWrite(RGBlue, HIGH);
  digitalWrite(RedGB, LOW);
  delay(900);
  digitalWrite(RedGB, HIGH);
  digitalWrite(RGBlue, LOW);
  }
}

void pass() {
  if(RGBs)
  {
  digitalWrite(RGBlue, HIGH);
  digitalWrite(RGreenB, LOW);
  delay(900);
  digitalWrite(RGreenB, HIGH);
  digitalWrite(RGBlue, LOW);
  }
}

void sendP() {
  if(RGBs)
  {
  digitalWrite(RedGB, LOW);
  }
}

void stopP() {
  if(RGBs)
  {
  digitalWrite(RedGB, HIGH);
  }
}

void recP() {
  if(RGBs)
  {
  digitalWrite(RGreenB, LOW);
  delay(500);
  digitalWrite(RGreenB, HIGH);
  }
}

void start() {
  if(RGBs)
  {
  digitalWrite(RGBlue, LOW);
  }
}

void off() {
  digitalWrite(RedGB, HIGH);
  digitalWrite(RGreenB, HIGH);
  digitalWrite(RGBlue, HIGH);
}
        
