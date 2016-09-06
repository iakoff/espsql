#ifndef ESPSQL_h
#define ESPSQL_h

#include <Stream.h>
#include <Arduino.h>
/*#include <IPAddress.h>*/


class ESPSql
{
  public:
     void begin(Stream *ess);
     bool config(String ssid,String pass, String ip, String usr, String spass);
     bool ready();
     bool sql(String s);
     bool setsql(int x,String s);
     bool runsqladd(int x,String par);
     bool runsqlpar(int x,String par);
     bool sqlready();
     bool sqlready(int x);
     int colnum(int x);
     int rownum(int x);
     int colnum();
     int rownum();
     String getvalue(int x,int col, int row);
     void clrresp();
  
  private:
     void espreset();
     int lp(String in,String dv);
     String ex(String in,String dv,int idx);
     String funccmd(String s,int xmin, int xmax, int timeout);
  protected: 
     Stream *espsqlSerial;
};

#endif 

