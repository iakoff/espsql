
#include <Arduino.h>

#include "espsql.h"

#define ESP_CH_PD 51
#define ESP_RST   50




int ESPSql::lp(String in,String dv) {
  int f=0,idx=1;
  while (in.indexOf(dv,f)>=0) {
      f=in.indexOf(dv,f)+dv.length();
      idx++;
  }
  return idx;
}


String ESPSql::ex(String in,String dv,int idx) {
  int l,f=0;
  String tt;
  do {
      l=in.indexOf(dv,f);
      if (l<0) l=in.length();
      l+=dv.length();
      //LOGDEBUG1("L=",l);
      idx--;
      if(idx==0) return in.substring(f,l-dv.length());
      f=l;
  } while(f<in.length()&&idx>0);
  return "";
}

String ESPSql::funccmd(String s,int xmin, int xmax, int timeout) {
    String t="";
    do { 
      while(espsqlSerial->available()) espsqlSerial->readStringUntil('\n');
      espsqlSerial->print(s+"\n");espsqlSerial->flush();
      while(!espsqlSerial->available());
      t=espsqlSerial->readStringUntil('\n');timeout--;
      //Serial.print(timeout);Serial.print("!!");Serial.print(s);Serial.print("!!");Serial.print(xmin);Serial.print("!!");Serial.print(xmax);Serial.print("!!");Serial.println(t);
    }while (((ex(t,",",1).toInt()<xmin)||(ex(t,",",1).toInt()>xmax))&&(timeout>0));
    if (!timeout) t="";
    return t;
}

void ESPSql::begin(Stream *ess){
      espsqlSerial = ess;
      //espsqlSerial->begin(115200);
      espsqlSerial->setTimeout(1000);delay(500);
      while(espsqlSerial->available()) espsqlSerial->readStringUntil('\n');
}

//$par#u314zd271n,1d5p15p0pd,gb2due,28jn4ERF3,192.168.2.106
bool ESPSql::config(String ssid,String pass, String ip, String usr, String spass){
    String s=funccmd("$par#"+ssid+","+pass+","+usr+","+spass+","+ip,101,102,5);
    if (s.toInt()==101) {
       //while(ready());
       return true;
    }
    else {
      //Serial.println(s);
      return false;
    }
}

bool ESPSql::ready(){
    String s=funccmd("$sts#999",160,161,5);
    if (lp(s,",")==3) {
       if((ex(s,",",1).toInt()==161)&&(ex(s,",",2).toInt()==999)) {
          return true;
       }
       else {
        //Serial.println(s);
        return false;
      }
    }
    else {
      //Serial.println(s);
      return false;
    }
}

bool ESPSql::sql(String s){
}

bool ESPSql::setsql(int x,String s){
    s=funccmd("$sql#"+String(x)+"#$"+s,110,112,5);
    if (lp(s,",")==2) {
       if((ex(s,",",1).toInt()==110)&&(ex(s,",",2).toInt()==x)) {
          return true;
       }
       else {
        //Serial.println(s);
        return false;
       }
    }
    else {
      //Serial.println(s);
      return false;
    }
}
bool ESPSql::runsqladd(int x,String par){
    par=funccmd("$exe#"+String(x)+"#$"+par,120,121,5);
    if (lp(par,",")==2) {
       if((ex(par,",",1).toInt()==120)&&(ex(par,",",2).toInt()==x)) {
          return true;
       }
       else {
        //Serial.println(s);
        return false;
       }
    }
    else {
      //Serial.println(s);
      return false;
    }
}
bool ESPSql::runsqlpar(int x,String par){
    par=funccmd("$exe#"+String(x)+","+par,120,121,5);
    if (lp(par,",")==2) {
       if((ex(par,",",1).toInt()==120)&&(ex(par,",",2).toInt()==x)) {
          return true;
       }
       else {
        //Serial.println(s);
        return false;
       }
    }
    else {
      //Serial.println(s);
      return false;
    }
}
bool ESPSql::sqlready(){
    String s=funccmd("$sts#0",160,161,5);
    if (lp(s,",")>2) {
       if((ex(s,",",1).toInt()==160)&&(ex(s,",",3).toInt()==0)) {
          if(ex(s,",",2).toInt()==211) {
            return true;
          }
          if(ex(s,",",2).toInt()==210) {
            return true;
          }
          return false;
       }
       else {
        //Serial.println(s);
        return false;
       }
    }
    else {
      //Serial.println(s);
      return false;
    }
}
bool ESPSql::sqlready(int x){
    String s=funccmd("$sts#"+String(x),160,161,5);
    if (lp(s,",")>2) {
       if((ex(s,",",1).toInt()==160)&&(ex(s,",",3).toInt()==x)) {
          if(ex(s,",",2).toInt()==211) {
            return true;
          }
          if(ex(s,",",2).toInt()==210) {
            return true;
          }
          return false;
       }
       else {
        //Serial.println(s);
        return false;
       }
    }
    else {
      //Serial.println(s);
      return false;
    }
}
int ESPSql::colnum(int x){
    String s=funccmd("$sts#"+String(x),160,161,5);
    if (lp(s,",")>2) {
       if((ex(s,",",1).toInt()==160)&&(ex(s,",",2).toInt()==210)&&(ex(s,",",3).toInt()==x)) {
          return ex(s,",",4).toInt();
       }
       else {
        //Serial.println(s);
        return -1;
       }
    }
    else {
      //Serial.println(s);
      return -1;
    }
}
int ESPSql::rownum(int x){
    String s=funccmd("$sts#"+String(x),160,161,5);
    if (lp(s,",")>2) {
       if((ex(s,",",1).toInt()==160)&&(ex(s,",",2).toInt()==210)&&(ex(s,",",3).toInt()==x)) {
          return ex(s,",",5).toInt();
       }
       else {
        //Serial.println(s);
        return -1;
       }
    }
    else {
      //Serial.println(s);
      return -1;
    }
}
int ESPSql::colnum(){}
int ESPSql::rownum(){}
String ESPSql::getvalue(int x,int col, int row){
    String s=funccmd("$res#"+String(x)+","+String(col)+","+String(row),130,131,5);
    if (lp(s,",")==2) {
       if((ex(s,",",1).toInt()==130)&&(ex(s,",",2).toInt()==x)) {

           while(!espsqlSerial->available());
           s=espsqlSerial->readStringUntil('\n');
           return s;
       }
       else {
        //Serial.println(s);
        s="";
        return s;
       }
    }
    else {
      //Serial.println(s);
      s="";
      return s;
    }
}
void ESPSql::clrresp(){}
void ESPSql::espreset(){}


