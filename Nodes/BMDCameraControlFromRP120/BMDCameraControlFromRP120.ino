#include <BMDSDIControl.h>                                // need to include the library
#include <UIPEthernet.h>
#include <Url.h>
#include "PanasonicRP120Protocol.h"

#ifdef YUN
  #include <Bridge.h>
  #include <YunServer.h>
  #include <YunClient.h>

  #define TClient BridgeClient
  #define TServer BridgeServer
#else
  #define TClient EthernetClient
  #define TServer EthernetServer
#endif

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
//byte mac[] = { 0x5F, 0x53, 0x2F, 0x6C, 0xB1, 0xD0 };
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFA, 0xED };  

//Unit 1
//IPAddress ip(10, 10, 4, 54);

//Unit 2
IPAddress ip(10, 10, 4, 55);

IPAddress gateway(10, 10, 4, 254);
IPAddress subnet(255, 255, 255,0);
TServer server(80);

BMD_SDITallyControl_I2C sdiTallyControl(0x6E);            // define the Tally object using I2C using the default shield address
BMD_SDICameraControl_I2C sdiCameraControl(0x6E);            // define the Tally object using I2C using the default shield address
PanasonicRP120Protocol rp120(&sdiCameraControl,&sdiTallyControl);

void setup() {
// Open serial communications and wait for port to open:
  Serial.begin(9600);

  //Set the last field of the MAC address to the last of the IP address. This avoids duplicate MAC addresses.
  //mac[5] = ip[3];
  
  //Setup Ethernet
  #ifdef YUN
    Bridge.begin();
  #else
    // start the Ethernet connection and the server:
    Ethernet.begin(mac, ip, gateway, subnet);
  #endif
  server.begin();

  Serial.print("IP:");
  Serial.println(Ethernet.localIP());

  //Setup SDI Shield  
  sdiTallyControl.begin();                                 // initialize tally control
  sdiTallyControl.setOverride(true);                       // enable tally override

  sdiCameraControl.begin();                                 // initialize tally control
  sdiCameraControl.setOverride(true);                       // enable tally override
}

void loop() {
    #ifdef YUN
  BridgeClient client = server.accept();
  #else
  EthernetClient client = server.available();
  #endif

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

    // close the connection:
    client.stop();
  }
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
}
