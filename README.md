# InternetRFIDTags
Made for the academy high school management system 

##Before we Begin
Before we can start scanning cards and sending them to our Telnet/TCP server we need some libraries 
Get UIPEthernet here https://github.com/ntruchsess/arduino_uip

get MFRC522 here https://github.com/miguelbalboa/rfid

Put both of these libraries in Program Files(x86)/Arduino/Libraries/
Restart arduino

##Hardware
1. Arduino Nano (You can always modify your code to fit your device)
2. MFRC522 with MAIFARE cards
3. Jumper wires (Male to Male) (Male to Female)
4. enc28j60 ethernet module/sheild
5. RGB LED
6. 3V OR 5V greater than 700 mileamp AC-DC converter

##Setup
1. Attach arduino to breadboard (If nano or micro)
2. Look up online for the pinout of your board to find the SPI setup(Change values below)
3. Connect arduino pin 10 (SS) to ethernet module ss or CS
4. Connect arduino pin 12 (MISO) to rfid MISO and ethernet SO
5. Connect arduino pin 11 (MOSI) to rfid MOSI and ethernet SI
6. Connect arduino pin 13 (SCK) to rfid SCK and ethernet SCK
7. Connect arduino pin 9 to rfid RST pin
8. Connect arduino pin 8 to rfid SSN
9. Connect arduino pin 5 to green led, 4 to blue and 3 to red
10. Connect your AC to DC to the + and - on your breadboard
11. Ground your arduino to the ac to dc
12. Connect VCC and GND on both rfid and ethernet to the ac-dc (REMEMBER THESE DEVICES ARE ONLY 3v!!!!! do not supply 5v) If problem use resistors to bring the voltage to 3v
13. Wire VCC pin on LED to the arduino 3v or ac-dc 3v
14. Connect ethernet cable to module and make sure it is on the same network as your computer
15. Plug USB cable from computer to arduino
16. Connect computer to same network

##Modify code to fit your needs
1. The code is pretty well commented so you can just go in and modify certain parts but one thing for sure is the ehternet module
2. Mac address can stay the same (Unless you're planning to build multiple of these)
3. If you are using a 192.168. base ip network you can keep the ip
4. Again you can keep the dns, only time to change is to 8.8.4.4
5. run ipconfig to find your gateway, the default is 192.168.1.1(If you don't know it) (The code currently is 192.168.1.5)
6. run command prompt and type ipconfig to figure out your computers ip address
7. scroll down to find SEND TO SERVER, and input the ip of your computer or if your port forwarded your router to your public ip address
8. MAKE SURE YOU HAVE PYTHON AND RUN THE SERVER (Remember it's just an example code we used for our presentation to the school, so the python server was already premade and almost not modified you can use any Telnet/TCP server)

##Running the code
1. Plug AC-DC power to the wall
2. Make sure your arduino is currently connected to the computer
3. Make sure both device are on the same network 
4. Flash your modified arduino code or if the one I have works for you then great(Almost no chance you will have to modify it)
5. Start your Python, C++ script or whatever Telnet/TCP server on your network
6. Restart your arduino to be safe
7. Wait until the light turns blue and try scanning a card, if your server got the ID of the card then your ready to go
8. Remember what the lights means Purple/fading red means booting up
9. Red means any error such as the card was at a weird angle and/or the server didn't respond in time
10. Green means pass so the server responded with a go and you can read your next card
11. Blue means waiting/loading waiting for a card or response
12. If your arduino starts to lag out and takes over 30 seconds to show a red light means that the arduino ethernet module couldn't connect to the server at all. This could be caused by multiple things first your arduino doesn't have enough power and the arduino ethernet module is struggeling to send a packet or that your computer server isn't running or that your arduino and computer are not on the same network.
13. If your arduino keeps lagging out then try these tricks to fix them


1. Turn off windows firewall
2. go into advanced firewall settings and allow inbound/outbound port 23
3. port forward you router to your computer with port 23
4. If you are wireless connect the arduino straight into the router and your computer to the same one
5. Buy a more heavy duty AC-DC power adapter
6. Else maybe you input your ip address or connecting address wrong


14. Either than that contact me at smerkousdavid@gmail.com if there are any problems 
15. YOUR GOOD TO GO!!1




##### I am not responsible for damaged devices, if unhandled properly your device will be damaged. Make sure your careful because I have once literally toasted an arduino uno, and it was my fault not the guy giving the tutorial. So be careful!
