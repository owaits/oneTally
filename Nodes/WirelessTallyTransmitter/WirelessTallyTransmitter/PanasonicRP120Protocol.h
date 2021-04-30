 #include <UIPEthernet.h>
 #include <Url.h>
 
 #define TClient EthernetClient
 #define TServer EthernetServer

class PanasonicRP120Protocol
{
  private:
    float focus = 0;
    float zoom = 0;
    bool programTallyOn = false;
    /*float focusSpeed = 0;
    float zoomSpeed = 0;
    int focusDivider = 0;
    int zoomDivider = 0;*/
    int loopDivider = 0;

    
  
  public:
  PanasonicRP120Protocol();
  
  void HTTP_ResponseHeader(TClient& client,int code);
  
  void HTTP_TextHeader(TClient& client);

  void PTZ_Open(TClient& client,const String& command);
    
    void PTZ_Pan(const String& command);
    
    void PTZ_Tilt(const String& command);
    
    void PTZ_Zoom(const String& command);
    
    void PTZ_Focus(const String& command);
    
    void PTZ_Iris(const String& command);

    void PTZ_GetIris(TClient& client);
    
    void PTZ_Tally(const String& command);
  
    bool ProcessManageSession(Url& request,TClient& client);

    bool aw_ptz(Url& request,TClient& client);
    
    bool aw_cam(Url& request,TClient& client);

    bool TryProcessGET(Url& request,TClient& client);

    bool GetTally();
};
