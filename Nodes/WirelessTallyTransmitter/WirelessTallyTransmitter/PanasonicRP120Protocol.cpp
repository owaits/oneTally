#include "PanasonicRP120Protocol.h"

PanasonicRP120Protocol::PanasonicRP120Protocol()
  {
  }
  
  void PanasonicRP120Protocol::HTTP_ResponseHeader(TClient& client,int code)
  {
      client.print(F("HTTP/1.0 "));
      client.print(code);
      client.println(F(" OK"));
      client.println(F("Status: "));
      client.println(code);
      client.println(F("Connection: close"));
  }
  
  void PanasonicRP120Protocol::HTTP_TextHeader(TClient& client)
  {
      client.println(F("HTTP/1.0 200 OK"));
      client.println(F("Connection: close"));
      client.println(F("Content-Type: text/plain"));
      client.println();
  }
  
  void PanasonicRP120Protocol::PTZ_Open(TClient& client,const String& command)
    {
      client.println("p1");   
    }
    
    void PanasonicRP120Protocol::PTZ_Pan(const String& command)
    {
      //Serial.println("--Pan--");
    }
    
    void PanasonicRP120Protocol::PTZ_Tilt(const String& command)
    {
        //Serial.println("--Tilt--");
    }
    
    void PanasonicRP120Protocol::PTZ_Zoom(const String& command)
    {
        float zoomSpeed = (float) (atoi(command.c_str() + 1) - 50) /50.0;
        zoomSpeed *=abs(zoomSpeed);
    }
    
    void PanasonicRP120Protocol::PTZ_Focus(const String& command)
    {
      float focusSpeed = (float) (atoi(command.c_str() + 1) - 50) / 50.0;
      focusSpeed *=abs(focusSpeed);

      focus += focusSpeed/10;
      focus = min(max(focus,0),1);
    }
    
    void PanasonicRP120Protocol::PTZ_Iris(const String& command)
    {
      float scaledIris = (float) (0xFFF - strtol(command.c_str() + 3, NULL, 16)) / 0xAAA;
      Serial.println(scaledIris);
    }

    void PanasonicRP120Protocol::PTZ_GetIris(TClient& client)
    {
      client.println("giFFF0"); 
    }
    
    void PanasonicRP120Protocol::PTZ_Tally(const String& command)
    {
      programTallyOn = atoi(command.c_str() + 2) == 1;
    }
  
    bool PanasonicRP120Protocol::ProcessManageSession(Url& request,TClient& client)
    {
        HTTP_TextHeader(client);
        client.println("Event session:1");
        return true;
    }  
    
    
  
    bool PanasonicRP120Protocol::aw_ptz(Url& request,TClient& client)
    { 
      HTTP_TextHeader(client);

      String command;
      if(request.LookupArgument("cmd",command))
      {
        if(command.startsWith("%23"))
          command = command.substring(3);
        else
          command = command.substring(1);
        
        if(command.startsWith("O"))
          PTZ_Open(client,command);
        if(command.startsWith("P"))
           PTZ_Pan(command);
        if(command.startsWith("T"))
           PTZ_Tilt(command); 
        if(command.startsWith("Z"))
           PTZ_Zoom(command); 
        if(command.startsWith("F"))
           PTZ_Focus(command); 
        if(command.startsWith("GI"))
           PTZ_GetIris(client);
        if(command.startsWith("AXI"))
           PTZ_Iris(command);
        if(command.startsWith("DA"))
          PTZ_Tally(command);
        if(command.startsWith("LPC1"))
          client.println("IPC1");

        command.toLowerCase();
        client.println(command);
        return true;
      }
      
      return false;
    }

    bool PanasonicRP120Protocol::aw_cam(Url& request,TClient& client)
    { 
      HTTP_TextHeader(client);

      String command;
      if(request.LookupArgument("cmd",command))
      {
        command.toLowerCase();
        client.println(command);
        return true;
      }
      
      return false;
    }
    
        
    
    bool PanasonicRP120Protocol::TryProcessGET(Url& request,TClient& client)
    {      
      if(request.GetPage() == "man_session")
        return ProcessManageSession(request,client);
      if(request.GetPage() == "aw_ptz")
        return aw_ptz(request,client);
              if(request.GetPage() == "aw_cam")
        return aw_cam(request,client);
        
        HTTP_ResponseHeader(client,204);
        return true;
    }

    bool PanasonicRP120Protocol::GetTally()
    {      
        return programTallyOn;
    }
