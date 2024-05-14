#include "UMDTally.h"

void UMDTally::begin(const IPAddress& ip, uint16_t port = 40001)
{
  destinationPort = port;
  ipAddress = ip;

  udp.begin(port);
}

void UMDTally::loop()
{
  int packetSize = udp.parsePacket();
  if(packetSize)
  {
    umd3_1 packet;
    int size = udp.read((char*) &packet, sizeof(umd3_1));

    //If this is an invalid UMD packet return
    if((packet.address & 0x80) == 0 && size == 18)
      return;

    byte address = (packet.address & 0x7F);
    if(address < 5)
    {
      tallyStatus[address] = packet.control;
    }

    connected = true;
  }
}

void UMDTally::sendTally(byte address, bool tally1, bool tally2, bool tally3, bool tally4, const char* label, int labelLength)
{
  byte packet[] = {0x80,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};

  packet[0] |= address;

  if(tally1) packet[1] |= 1;
  if(tally2) packet[1] |= 2;
  if(tally3) packet[1] |= 4;
  if(tally4) packet[1] |= 8;

  strncpy((char*) (packet +2),label,labelLength);
  send(packet, 18);
}

void UMDTally::send(byte* data, int length)
{
  if(udp.beginPacket(ipAddress,destinationPort))
  {
      udp.write(data,length);
      udp.endPacket();
  }  
}

bool UMDTally::getTally(byte address, bool& tally1, bool& tally2, bool& tally3, bool& tally4)
{
  if(!connected)
    return false;

  byte control = tallyStatus[address];
  tally1 = (control & 1) > 0;
  tally2 = (control & 2) > 0;
  tally3 = (control & 4) > 0;
  tally4 = (control & 8) > 0;

  return true;
}

int UMDTally::getProgramTally()
{
  for(int address=0;address<5;address++)
  {
      if((tallyStatus[address] & 1) > 0)
        return address;
  }
  return 0;
}

int UMDTally::getPreviewTally()
{
  for(int address=0;address<5;address++)
  {
      if((tallyStatus[address] & 2) > 0)
        return address;
  }
  return 0;
}

bool UMDTally::isConnected()
{
  return connected;
}

