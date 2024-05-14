#pragma once

#ifdef TEENSYDUINO
  #include <NativeEthernet.h>
  #include <NativeEthernetUdp.h>
#else
  #include <UIPEthernet.h>
#endif

class UMDTally
{
  private:
    struct umd3_1
    {
      byte address;
      byte control;
      char label[16];
    };

    EthernetUDP udp;
    uint16_t destinationPort;
    IPAddress ipAddress;
    byte tallyStatus[5];
    bool connected;
    void send(byte* data, int length);


  
  public: 
  void begin(const IPAddress& ipAddress, uint16_t port = 40001);
  void loop();
  void sendTally(byte address, bool tally1, bool tally2, bool tally3, bool tally4, const char* label, int labelLength);
  bool getTally(byte address, bool& tally1, bool& tally2, bool& tally3, bool& tally4);
  int getProgramTally();
  int getPreviewTally();
  bool isConnected();
};
