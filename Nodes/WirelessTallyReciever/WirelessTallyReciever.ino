/*
* Arduino Wireless Communication Tutorial
*       Example 1 - Receiver Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"
#include <UIPEthernet.h>

RF24 radio(5, 7); // CE, CSN
const byte address[6] = "TLY01";
void setup() {
  Serial.begin(9600);
  printf_begin();
  
  pinMode(2, OUTPUT);    // Set pin 3 to output, tally RED
  pinMode(3, OUTPUT);    // Set pin 4 to output, tally GREEN
  pinMode(9, INPUT);    // IRQ

  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.printDetails();   
  radio.startListening();
}
void loop() {
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
    
    if(strncmp(text,"tally:",6)==0)
    {
      digitalWrite(2,strncmp(text+ 6,"red",3)==0 || strncmp(text + 6,"yellow",6)==0);
      digitalWrite(3,strncmp(text + 6,"green",5)==0|| strncmp(text + 6,"yellow",6)==0);
    }
  }
}
