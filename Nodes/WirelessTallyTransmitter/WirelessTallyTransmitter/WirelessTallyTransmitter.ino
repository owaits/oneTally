/*
* Arduino Wireless Communication Tutorial
*     Example 1 - Transmitter Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/
#include <EEPROM.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"
#include <UIPEthernet.h>
#include "PanasonicRP120Protocol.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x1E };

struct Options
{
  IPAddress ip;
  IPAddress gateway;
  IPAddress subnet;
};

Options settings = 
{
  IPAddress(10,10,4,53),
  IPAddress(10, 10, 4, 254),
  IPAddress(255, 255, 255,0),
};

TServer server(80);
PanasonicRP120Protocol rp120;
RF24 radio(5, 7); // CE, CSN
const byte address[6] = "TLY01";

void setup() {
  Serial.begin(9600);
  printf_begin();

  pinMode(2, OUTPUT);    // Set pin 3 to output, tally RED
  pinMode(3, OUTPUT);    // Set pin 4 to output, tally GREEN
  pinMode(9, INPUT);    // IRQ
  pinMode(10, OUTPUT);  // Ethernet Chip Select
  pinMode(5, OUTPUT);  // RF25 Chip Enable
  pinMode(7, OUTPUT);  // RF25 Chip Select

  //Disable the ethernet chip so we can initialise the RF24 on SPI bus, they share logic buffers.  
  digitalWrite(10,HIGH);
  /*digitalWrite(5,LOW);
  digitalWrite(7,HIGH);*/

  //Reset to Factory Defaults when D3 and D5 are pulled low.
  pinMode(4, INPUT_PULLUP);
  if(digitalRead(4) == LOW)
  {
    setLocalIP(settings.ip, settings.subnet, settings.gateway);
  }
  else
  {
    EEPROM.get(0,settings);
  }

  if(radio.begin())
  {
    radio.openWritingPipe(address);
    //radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_MIN);
    radio.printDetails();
    radio.stopListening();
  }
  
  Ethernet.begin(mac, settings.ip, settings.gateway, settings.subnet);
  server.begin();

  Serial.print("IP:");
  Serial.println(Ethernet.localIP()); 
}

long sendTimer = 0;
bool lastTally = false;

void loop() {

  bool tally = rp120.GetTally();
  if(millis() > sendTimer || lastTally != tally)
  {

    sendTimer = millis() + 1000;
    
    const char tallyOn[] = "tally:red";
    const char tallyPreview[] = "tally:green";
    const char tallyOff[] = "tally:off";

    if(tally)
    {
      radio.write(&tallyOn, sizeof(tallyOn));  
      Serial.println(tallyOn);
    }
    else
    {
      radio.write(&tallyOff, sizeof(tallyOff));   
      Serial.println(tallyOff);  
    }
    
    digitalWrite(2,tally);
    digitalWrite(3,LOW);    
  }
  lastTally = tally;


  EthernetClient client = server.available();

  if (client) {    
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String request;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n') {
          if(request.startsWith("GET"))
          {
            HTTP_GET(client, request);
          }

          request = "";          
          if(currentLineIsBlank)
            break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    //delay(1);
    // close the connection:
    client.stop();
  }
}

void setLocalIP(const IPAddress& ipAddress, const IPAddress& subnetMask, const IPAddress& gatewayAddress)
{
  settings.ip = ipAddress;
  settings.subnet = subnetMask;
  settings.gateway = gatewayAddress;

  EEPROM.put(0,settings);
}

void printWebTextInputIP(const TClient& client, char* name, char* id, const IPAddress& ipAddress)
{
  client.print(F("<label for='"));
  client.print(id);
  client.print(F("'>"));
  client.print(name);
  client.print(F(":</label><input type='text' name='"));
  client.print(id);
  client.print(F("' value='"));
  client.print(ipAddress);
  client.print(F("' /><br>"));
}
void printWebTextInputInt(const TClient& client, char* name, char* id, int value)
{
  client.print(F("<label for='"));
  client.print(id);
  client.print("'>");
  client.print(name);
  client.print(F(":</label><input type='text' name='"));
  client.print(id);
  client.print(F("' value='"));
  client.print(value);
  client.print(F("' /><br>"));
}

void printWebSettingsHTML(const TClient& client)
{
       // start of web page
     client.println(F("HTTP/1.1 200 OK"));
     client.println(F("Content-Type: text/html"));
     client.println(F("<html><head></head><body>"));
     client.println();
     client.println(F("<h4>RP120 TALLY TRANSMITTER</h4>"));
     client.print(F("<form action='/ip' method=get>"));
     printWebTextInputIP(client,"IP Address", "ip", settings.ip);
     printWebTextInputIP(client,"Subnet", "sn", settings.subnet);
     printWebTextInputIP(client,"Gateway", "gt", settings.gateway);         
     client.print(F("<input type=submit value='Set IP'></form>"));
     
     client.print(F("</body></html>"));
}


void HTTP_GET(TClient& client,String rawRequest)
{
  Url request = Url::Parse(rawRequest,4);
  
  Serial.println(*request.GetUrl());  

    if(request.GetPath().startsWith("/cgi-bin/"))
    {
      if(rp120.TryProcessGET(request,client))
      {
          //AW Protocol Success
      }     
    }  
    else if(request.GetPath() == "/ip")
    {
      String ip, subnet, gateway;
      if(request.LookupArgument("ip",ip) && request.LookupArgument("sn",subnet) && request.LookupArgument("gt",gateway))
      {
          IPAddress ipAddress;
          ipAddress.fromString(ip);
          settings.ip = ipAddress;
          ipAddress.fromString(subnet);
          settings.subnet = ipAddress;
          ipAddress.fromString(gateway);
          settings.gateway = ipAddress;

          setLocalIP(settings.ip, settings.subnet, settings.gateway);
      }
      printWebSettingsHTML(client);
    }
    else
    {
       printWebSettingsHTML(client);
    } 
}
