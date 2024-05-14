#pragma once

#include <Arduino.h>

class Url
{
  private:
    String url;
   int arguments[20][2];
   int argumentCount;
   int pathEnd = 0;
   int queryStart = 0;
  
  public:
  static Url Parse(const String& value, int offset);
  
  static void Parse(const String& value, Url& url, int offset);
  
  String* GetUrl() { return &url; }
  
  String GetPath() { return url.substring(0,queryStart); }
  
  String GetPage() { return url.substring(pathEnd + 1,queryStart); }

  String GetQuery();
  
  String GetArgument(int index) { return url.substring(arguments[index][0],arguments[index][1]) ; }
  
  boolean LookupArgument(const String& key,String& value);
  
  int ArgumentCount() { return argumentCount; }
};
