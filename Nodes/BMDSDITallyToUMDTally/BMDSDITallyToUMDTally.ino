#include <BMDSDIControl.h>                                // need to include the library
#include "UMDTally.h"

#ifdef TEENSYDUINO
  #include <NativeEthernet.h>
  #include <NativeEthernetUdp.h>
#else
  #include <UIPEthernet.h>
#endif

#define UNIT1 1
#define MAX_CAMERA_CHANNELS 5

struct Options
{
  IPAddress ip;
  IPAddress gateway;
  IPAddress dns;
  IPAddress subnet;
  IPAddress umdIP;
};

#ifdef UNIT1
  //Unit 1
  byte mac[6] = { 0x90, 0xA2, 0xDA, 0xD0, 0x30, 0xE7 };

  Options settings = 
  {
    IPAddress(10,10,4,54),
    IPAddress(10, 10, 4, 254),
    IPAddress(10, 10, 4, 254),
    IPAddress(255, 255, 255,0),
    IPAddress(10, 10, 4, 255)
  };
#else
  //Unit 2
  byte mac[6] = { 0x90, 0xA2, 0xDA, 0xF3, 0x21, 0xEC }; 

  Options settings = 
  {
    IPAddress(10,10,4,55),
    IPAddress(10, 10, 4, 254),
    IPAddress(10, 10, 4, 254),
    IPAddress(255, 255, 255,0),
    IPAddress(10, 10, 4, 255)
  };
#endif

struct Tally
{
  bool program;
  bool preview;
};

Tally tallyStatus[5];

BMD_SDITallyControl_I2C sdiTallyControl(0x6E);            // define the Tally object using I2C using the default shield address
UMDTally umd;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  
  //Setup Ethernet
  //start the Ethernet connection and the server:
  Ethernet.begin(mac, settings.ip, settings.dns, settings.gateway, settings.subnet);

  Serial.print("IP:");
  Serial.println(Ethernet.localIP());

  //Setup SDI Shield  
  sdiTallyControl.begin();                                 // initialize tally control
  sdiTallyControl.setOverride(true);                       // enable tally override

  umd.begin(settings.umdIP);

  Serial.println("STARTED");
}

void loop() {

  delay(250);
  for(int channel=1;channel<=MAX_CAMERA_CHANNELS;channel++)
  {
    bool program, preview;
    sdiTallyControl.getCameraTally(channel, program, preview);

    Tally* currentTally = &tallyStatus[channel - 1];
    if(currentTally->program != program || currentTally->preview != preview)
    {
      String label = "Camera  " + String(channel);
      
      Serial.print("Tally:");
      Serial.print(label);
      Serial.print(preview);
      Serial.println(program);
      
      umd.sendTally(channel, program, preview, false, false, label.c_str(), label.length());

      currentTally->program = program;
      currentTally->preview = preview;
    }  
  } 
}

