#include "Url.h"

static Url Url::Parse(const String& value, int offset)
{
	 Url newUrl;
	 Parse(value,newUrl,offset);
	 return newUrl;
}
	
static void Url::Parse(const String& value, Url& newUrl, int offset)
{ 
  newUrl.url = value.substring(offset,value.indexOf(" ",offset));  
  
  newUrl.queryStart = newUrl.url.indexOf("?");
  if(newUrl.queryStart == -1)
    newUrl.queryStart  = newUrl.url.length();
    
  newUrl.pathEnd = newUrl.url.lastIndexOf("/",newUrl.queryStart);
  if(newUrl.pathEnd == -1)
    newUrl.pathEnd = 0;

 
  newUrl.argumentCount = 0;
  int queryStart = newUrl.queryStart;
  while(queryStart != -1 && queryStart != newUrl.url.length() &&newUrl.argumentCount < 20)
  {
    int queryEnd = newUrl.url.indexOf("&",queryStart + 1);
    
    if(queryEnd != -1)
	{
		newUrl.arguments[newUrl.argumentCount][0] = queryStart + 1;
		newUrl.arguments[newUrl.argumentCount][1] = queryEnd; 
	}      
    else
	{
		newUrl.arguments[newUrl.argumentCount][0] = queryStart + 1;
		newUrl.arguments[newUrl.argumentCount][1] = newUrl.url.length(); 
	}       
         
    queryStart = queryEnd;
    newUrl.argumentCount++;
  }
} 

boolean Url::LookupArgument(const String& key,String& value)
{  
  for(int n=0;n<argumentCount;n++)
  {
	String argument = GetArgument(n);
    if(argument.startsWith(key))
    {
      value = argument.substring(argument.indexOf("=") + 1);
      return true;
    }      
  }
  return false;
}

String Url::GetQuery()
{
  String query = GetPage();
  return query; 
}
