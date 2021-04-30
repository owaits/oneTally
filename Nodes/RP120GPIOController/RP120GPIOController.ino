#include "uipethernet-conf.h"
#include <UIPEthernet.h>
#include <EEPROM.h>
#include <Url.h>

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
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xDE };

struct Options
{
  IPAddress ip;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress videoHubIP;
  IPAddress umdIP;
  byte videoHubCameraOffset;
  byte videoHubPreviewPort;
};

Options settings = 
{
  IPAddress(10,10,4,49),
  IPAddress(10, 10, 4, 254),
  IPAddress(255, 255, 255,0),
  IPAddress(10, 10, 4, 22),
  IPAddress(10, 10, 4, 35),
  1,
  1
};

byte cameraSelect = 0;
byte tallySelect = 0;

TServer server(80);   //web server port
TClient videoHubClient;
EthernetUDP udp; //UDP used for UMD tally

void setup() {
// Open serial communications and wait for port to open:
  Serial.begin(9600);

  //Reset to Factory Defaults when D3 and D5 are pulled low.
  pinMode(A7, INPUT_PULLUP);
  if(digitalRead(A7) == LOW)
  {
    setLocalIP(settings.ip, settings.subnet, settings.gateway);
  }
  else
  {
    EEPROM.get(0,settings);
  }
  
  //Setup Ethernet
  #ifdef YUN
    Bridge.begin();
  #else
    // start the Ethernet connection and the server:
    Ethernet.begin(mac, settings.ip, settings.gateway, settings.subnet);
  #endif
  server.begin();
  udp.begin(19522); 

  //Wait for the Ethernet to initialise
  delay(500);

  //Try and connect to the Video Router
    if(!videoHubClient.connected())
    {      
      videoHubClient.connect(settings.videoHubIP,9990);
    }

  Serial.print(F("IP:"));
  Serial.println(Ethernet.localIP());
  Serial.print(F("CAM Port:"));
  Serial.println(settings.videoHubCameraOffset);
  Serial.print(F("Preview Port:"));
  Serial.println(settings.videoHubPreviewPort);

  //Initialise the input pins for Tally.
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);

  //Initialise the input pins for camera select.
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
}

void setLocalIP(const IPAddress& ipAddress, const IPAddress& subnetMask, const IPAddress& gatewayAddress)
{
  settings.ip = ipAddress;
  settings.subnet = subnetMask;
  settings.gateway = gatewayAddress;

  EEPROM.put(0,settings);
}

void setVideoHubOptions(const IPAddress& ipAddress, byte cameraOffset, byte previewPort)
{
  settings.videoHubIP = ipAddress;
  settings.videoHubCameraOffset = cameraOffset;
  settings.videoHubPreviewPort = previewPort;

  EEPROM.put(0,settings);
}

void setUMDOptions(const IPAddress& ipAddress)
{
  settings.umdIP = ipAddress;

  EEPROM.put(0,settings);
}

void pingVideoHub()
{if(videoHubClient.connected())
{
    videoHubClient.println(F("PING:"));
    videoHubClient.println();
}
}


void videoHubCrosspoint(byte input, byte* output, int crossPointCount)
{

if(videoHubClient.connected())
{  
    videoHubClient.println(F("VIDEO OUTPUT ROUTING:"));
    for(int n=0;n<crossPointCount; n++)
    {
      videoHubClient.print(output[n] - 1);
      videoHubClient.print(" ");
      videoHubClient.println(input - 1);
      videoHubClient.println();
    }   
  }  
}

void printWebTextInputIP(const TClient& client, char* name, char* id, const IPAddress& ipAddress)
{
  client.print(F("<label for='"));
  client.print(id);
  client.print("'>");
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
  client.print(F("'>"));
  client.print(name);
  client.print(F(":</label><input type='text' name='"));
  client.print(id);
  client.print(F("' value='"));
  client.print(value);
  client.print(F("' /><br>"));
}

void loopWebServer()
{
    // Create a client connection
    String rawRequest;
  TClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        //read char by char HTTP request
        if (rawRequest.length() < 100) {

          //store characters to string
          rawRequest += c;
          Serial.print(c); //uncomment to see in serial monitor
        }

                //if HTTP request has ended
        if (c == '\n') {
          Url request;
          Url::Parse(rawRequest,request,4);

          if(request.GetPath() == "/ip")
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
          }

          if(request.GetPath() == "/videoHub")
          {
            String ip,umdip, cameraPort, previewPort;
            if(request.LookupArgument("vhip",ip) && request.LookupArgument("vhcp",cameraPort) && request.LookupArgument("vhpp",previewPort))
            {
                IPAddress ipAddress;
                ipAddress.fromString(ip);
                setVideoHubOptions(ipAddress, cameraPort.toInt(),previewPort.toInt());
            }
          }

          if(request.GetPath() == "/umd")
          {
            String ip;
            if(request.LookupArgument("umdip",ip))
            {
                IPAddress ipAddress;
                ipAddress.fromString(ip);
                setUMDOptions(ipAddress);
            }
          }
          
          ///////////////
          //Serial.println(readString);

        // start of web page
         client.println(F("HTTP/1.1 200 OK"));
         client.println(F("Content-Type: text/html"));
         client.println(F("<html><head></head><body>"));
         client.println();
         client.print(F("<form action='/ip' method=get>"));
         printWebTextInputIP(client,"IP Address", "ip", settings.ip);
         printWebTextInputIP(client,"Subnet", "sn", settings.subnet);
         printWebTextInputIP(client,"Gateway", "gt", settings.gateway);         
         client.print(F("<input type=submit value='Set IP'></form>"));

         client.print(F("<form action='/videoHub' method=get>"));
         printWebTextInputIP(client,"VideoHub IP", "vhip", settings.videoHubIP);
         printWebTextInputInt(client,"VideoHub CAM Port", "vhcp", settings.videoHubCameraOffset);
         printWebTextInputInt(client,"VideoHub Preview Port", "vhpp", settings.videoHubPreviewPort);       
         client.print(F("<input type=submit value='Update'></form>"));

         client.print(F("<form action='/umd' method=get>"));  
         printWebTextInputIP(client,"Monitor IP", "umdip", settings.umdIP);    
         client.print(F("<input type=submit value='Update'></form>"));
         
         client.print(F("</body></html>"));
         
         //stopping client
          delay(1);
          client.stop();
        }
       }
     }
   }
}

void umdTally(byte tallyOn, char* label, int labelLength)
{
  byte packet[] = {0x5A,0x1C,0x00,0x20,0x01,0x0FF,0x00,0x00,0x05,0x08,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};

  if(tallyOn)
    packet[9] = 0x18;

  strncpy(packet +10,label,labelLength);
  umdSend(packet, 26);
}

void umdSend(byte* data, int length)
{
  int checksum = 0;
  byte* dataPtr = data;
  for(int n=0;n<length;n++)
  {
    checksum = (checksum + *dataPtr) % 0x100;
    dataPtr ++;
  }
  
  if(udp.beginPacket(settings.umdIP,19523))
  {
      byte crc[] = {checksum, 0xdd};      

      udp.write(data,length);
      udp.write(crc,2);

      int sent = udp.endPacket();
      Serial.print(F("UDP: "));
      Serial.println(sent);
  }  
}

unsigned int pingDivider = 1;

void loop() {
    if (videoHubClient.connected() && videoHubClient.available()) {
      char c = videoHubClient.read();
      Serial.print(c);
    }

  byte cameraSelectRead = 0;
    cameraSelectRead = (cameraSelectRead | !digitalRead(A3)) << 1;
    cameraSelectRead = (cameraSelectRead | !digitalRead(A2)) << 1;
    cameraSelectRead = (cameraSelectRead | !digitalRead(A1)) << 1;
    cameraSelectRead = (cameraSelectRead | !digitalRead(A0));

    byte tallySelectRead = 0;
    if(!digitalRead(7)) tallySelectRead = 6;
    if(!digitalRead(6)) tallySelectRead = 5;
    if(!digitalRead(5)) tallySelectRead = 4;
    if(!digitalRead(4)) tallySelectRead = 3;
    if(!digitalRead(3)) tallySelectRead = 2;
    if(!digitalRead(9)) tallySelectRead = 1; 

    //If there has been a change of state send an update.
    if(cameraSelect != cameraSelectRead || tallySelect != tallySelectRead)
    {
       cameraSelect = cameraSelectRead;
       tallySelect = tallySelectRead;

      if(cameraSelect > 0)
      {
        
          //Set the crosspoint for preview ports.
          byte rp120PreviewPort[] = { settings.videoHubPreviewPort};
          videoHubCrosspoint(cameraSelectRead + settings.videoHubCameraOffset -1,rp120PreviewPort,sizeof(rp120PreviewPort));

          char label[8];
          sprintf(label,"Camera %d",cameraSelect);
          umdTally(cameraSelectRead == tallySelectRead,label,8);
      }

    }
    else
{
  pingDivider --;
  if(pingDivider == 0)
  {
    pingVideoHub();
    pingDivider = 0x1FFF;
  }
    
  
}

loopWebServer();    
}
