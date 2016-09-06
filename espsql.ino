// SELECT idrawacts,pinname,pinvalue FROM arduino.rawacts WHERE isnull(updated) order by stamp limit 3
// $dl#4
// $cfg#1,9600
// $par#u314zd271n,1d5p15p0pd,gb2due,28jn4ERF3,192.168.2.106
// $sts#999
// $clr#
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <fs.h>
#include <EMySQL_Connection.h>
#include <EMySQL_Cursor.h>


#define LOGERROR(x)    if(_ESPMAINLOGLEVEL_>0) { Serial.print("$#ESP "); Serial.println(x); }
#define LOGERROR1(x,y) if(_ESPMAINLOGLEVEL_>0) { Serial.print("$#ESP "); Serial.print(x); Serial.print(" "); Serial.println(y); }
#define LOGWARN(x)     if(_ESPMAINLOGLEVEL_>1) { Serial.print("$#ESP "); Serial.println(x); }
#define LOGWARN1(x,y)  if(_ESPMAINLOGLEVEL_>1) { Serial.print("$#ESP "); Serial.print(x); Serial.print(" "); Serial.println(y); }
#define LOGINFO(x)     if(_ESPMAINLOGLEVEL_>2) { Serial.print("$#ESP "); Serial.println(x); }
#define LOGINFO1(x,y)  if(_ESPMAINLOGLEVEL_>2) { Serial.print("$#ESP "); Serial.print(x); Serial.print(" "); Serial.println(y); }

#define LOGDEBUG(x)      if(_ESPMAINLOGLEVEL_>3) { Serial.print("$#ESP "); Serial.println(x); }
#define LOGDEBUG0(x)      if(_ESPMAINLOGLEVEL_>3) { Serial.print(x); }
#define LOGDEBUG1(x,y)   if(_ESPMAINLOGLEVEL_>3) { Serial.print("$#ESP "); Serial.print(x); Serial.print(" "); Serial.println(y); }
#define LOGDEBUG2(x,y,z) if(_ESPMAINLOGLEVEL_>3) { Serial.print("$#ESP "); Serial.print(x); Serial.print(" "); Serial.print(y); Serial.print(" "); Serial.println(z); }


#define LOGERROR(x)    if(_ESPMAINLOGLEVEL_>0) { Serial.print("$#ESP "); Serial.println(x); }
#define LOGERROR1(x,y) if(_ESPMAINLOGLEVEL_>0) { Serial.print("$#ESP "); Serial.print(x); Serial.print(" "); Serial.println(y); }
#define LOGWARN(x)     if(_ESPMAINLOGLEVEL_>1) { Serial.print("$#ESP "); Serial.println(x); }
#define LOGWARN1(x,y)  if(_ESPMAINLOGLEVEL_>1) { Serial.print("$#ESP "); Serial.print(x); Serial.print(" "); Serial.println(y); }
#define LOGINFO(x)     if(_ESPMAINLOGLEVEL_>2) { Serial.print("$#ESP "); Serial.println(x); }
#define LOGINFO1(x,y)  if(_ESPMAINLOGLEVEL_>2) { Serial.print("$#ESP "); Serial.print(x); Serial.print(" "); Serial.println(y); }

#define LOGDEBUG(x)      if(_ESPMAINLOGLEVEL_>3) { Serial.print("$#ESP "); Serial.println(x); }
#define LOGDEBUG0(x)      if(_ESPMAINLOGLEVEL_>3) { Serial.print(x); }
#define LOGDEBUG1(x,y)   if(_ESPMAINLOGLEVEL_>3) { Serial.print("$#ESP "); Serial.print(x); Serial.print(" "); Serial.println(y); }
#define LOGDEBUG2(x,y,z) if(_ESPMAINLOGLEVEL_>3) { Serial.print("$#ESP "); Serial.print(x); Serial.print(" "); Serial.print(y); Serial.print(" "); Serial.println(z); }



int _ESPMAINLOGLEVEL_=0;



char sqlpar[5][32];
int serbaudrate=115200;

//long int mt=millis();

int status = WL_IDLE_STATUS;

IPAddress server_addr(192,168,2,106);  // IP of the MySQL *server* here


// Initialize the client library
WiFiClient client;
MySQL_Connection conn((Client *)&client);


void soft_reset() {
  while (conn.connected()) { conn.close();}
  client.stop();
  ESP.restart();
} 

bool loadini(void){
  File f;
  String ss,x;
  f = SPIFFS.open(symFILE, "r");
  if (f) {
    ss = f.readStringUntil('\n');
    LOGDEBUG1("LOADINI=",ss);
    for(int i=0;i<5;i++){
        x=ex(ss,symSepA,i+1);
        x.toCharArray(sqlpar[i],x.length()+1);
        LOGDEBUG1(i,sqlpar[i]);
    }
    switch(ex(ss,symSepA,6).toInt()) {
      case 115200: case 57600: case 38400: case 19200: case 9600: serbaudrate=ex(ss,symSepA,6).toInt(); break;
      default: serbaudrate=115200; break;
    }
    if (Serial.baudRate()!=serbaudrate) Serial.begin(serbaudrate);
    return true;        
  }
  else {
  /*  serbaudrate=115200;
    if (Serial.baudRate()!=serbaudrate) Serial.begin(serbaudrate);
    LOGDEBUG1("serbaudrate=",serbaudrate);*/
    return false;
  }
}


void select2file(){
   
   char tmpsql[1000];
   int rownum = 0;
   File f;

 for(int posSqlStr=0;posSqlStr<=MaxSqlStr;posSqlStr++) {
   LOGDEBUG1("posSqlStr",posSqlStr);
   if (SPIFFS.exists("sqlstring"+String(posSqlStr))) {
    if (conn.connected()) {
      LOGDEBUG("S2F00");
      f = SPIFFS.open("sqlstring"+String(posSqlStr), "r");
      String sqlstr=f.readStringUntil('\n');f.close();
      
      MySQL_Cursor cur = MySQL_Cursor(&conn);
      if ((sqlstr.length()<1000)&&(sqlstr.length()>2)) {
          sqlstr.toCharArray(tmpsql, sqlstr.length()+1);
          LOGDEBUG1("S2F1",posSqlStr);
          if (cur.execute(tmpsql)){
              LOGDEBUG("S2F2");
              f = SPIFFS.open("sqlbufer"+String(posSqlStr), "w");
              column_names *columns = cur.get_columns();
              f.println(columns->num_fields);
              for (int ff = 0; ff < columns->num_fields; ff++) {
                f.println(columns->fields[ff]->name);
                //SerialAutoMakerLoop();
              }
              row_values *row = NULL;
              do {
                row = cur.get_next_row();
                if (row != NULL) {
                  for (int ff = 0; ff < columns->num_fields; ff++) {
                    f.println(row->values[ff]);
                    //SerialAutoMakerLoop();
                  }
                }
                rownum++;
              } while (row != NULL);
              f.close();
              
              /*f = SPIFFS.open("sqlstat"+String(posSqlStr), "w");
              f.print(210); f.print(","); 
              f.print(posSqlStr); f.print(","); 
              f.print(columns->num_fields);f.print(","); 
              f.println(rownum);
              f.close();*/
              storestatsql(posSqlStr,210,columns->num_fields,rownum);
          }
          else {
              /*f = SPIFFS.open("sqlstat"+String(posSqlStr), "w");
              f.print(211); f.print(",");f.println(posSqlStr); 
              f.close();*/
              storestatsql(posSqlStr,211,0,0);
          }
          SPIFFS.remove("sqlstring"+String(posSqlStr));
          //posSqlStr++;
          LOGDEBUG("S2F3");
      }
      else {
         /*f = SPIFFS.open("sqlstat"+String(posSqlStr), "w");
         f.print(212); f.print(",");f.println(posSqlStr); 
         f.close();*/
         storestatsql(posSqlStr,212,0,0);
      }
      cur.close();
    }
    else {
      /*f = SPIFFS.open("sqlstat"+String(posSqlStr), "w");
      f.print(213); f.print(",");f.println(posSqlStr); 
      f.close();*/
      storestatsql(posSqlStr,213,0,0);
    }
  }
 }
}

void setup() {
  Serial.begin(serbaudrate);
  SPIFFS.begin();
  loadini();
  Serial.println();
  Serial.println("ESPSQL v1.1");
  WiFi.mode(WIFI_STA);
  /*WiFi.disconnect();*/
  delay(100);
/*  loadini();
  WiFi.begin(sqlpar[0], sqlpar[1]);
  if(checkwifi()) {
     if(_ESPMAINLOGLEVEL_>3) {WiFi.printDiag(Serial);}
     if(checksql()) {
        Serial.println("999");
     }
     else {
        Serial.println("998");
     }
  }
  else {
    Serial.println("997");
  }*/
  for (int i=0;i<=MaxSqlStr;i++) storestatsql(i,0,0,0);
}



void ConnectAuto () {
static int prauto,prwstt,conauto=0;
static unsigned long contmr=0;
    if (prauto!=conauto) {
      prauto=conauto;
      LOGDEBUG1("ConnAuto",prauto);
    }
    if (prwstt!=WiFi.status()) {
      prwstt=WiFi.status();
      LOGDEBUG1("WiFi.status",prwstt);
    }
    switch(conauto) {
      case 0:
          if (WiFi.status()==WL_CONNECTED) {
              if(conn.connected()) {
                  conn.close();
                  client.stop();
              }
              WiFi.disconnect();
          }
          conauto=1;
          break;
      case 1:
          if (WiFi.status()==WL_CONNECTED) {
              conauto=0;
          }
          else {
              conauto=2;
          }
          break;
      case 2: 
          if (loadini()) {
             WiFi.begin(sqlpar[0], sqlpar[1]);
             contmr=millis()+20000;
             conauto=5;             
          }
          else {
             SaveStat(995,WiFi.status());
             conauto=3;
          }
          break;
      case 3:
          if (loadini()) {
             conauto=0;
          }
          break;
      case 5:
          switch(WiFi.status()) {
            
            case WL_CONNECTED:
              SaveStat(997,WiFi.status());
              server_addr.fromString(sqlpar[4]);
              if (conn.connect(server_addr, 3306, sqlpar[2], sqlpar[3])) {
                  conauto=7;
                  SaveStat(999,WiFi.status());
                  LOGDEBUG1("con.connect",WiFi.status());
              }
              else {
                  conauto=0;
                  LOGDEBUG1("con.disconnect",WiFi.status());
              }
              break;
            case WL_IDLE_STATUS:     SaveStat(981,WiFi.status());  break;  
            case WL_NO_SSID_AVAIL:   SaveStat(982,WiFi.status());  break;  
            case WL_SCAN_COMPLETED:  SaveStat(983,WiFi.status());  break;  
            case WL_CONNECT_FAILED:  SaveStat(984,WiFi.status());  break;  
            case WL_CONNECTION_LOST: SaveStat(985,WiFi.status());  break;  
            case WL_DISCONNECTED:
              if (millis()>contmr) { 
                conauto=0;
                SaveStat(986,WiFi.status());
              }
              break;  
            default:  SaveStat(980,WiFi.status());  break;
          }
          /*if (SPIFFS.exists(symFILEFLAG)) {
              SPIFFS.remove(symFILEFLAG);*/
          if (ifreconnect()) {
              conauto=0;
              SaveStat(994,WiFi.status());
          }
          break;
      case 7:
          if (WiFi.status()!=WL_CONNECTED) {
             conauto=0;
             SaveStat(991,WiFi.status());
          }
          else {
            if(!conn.connected()) { 
               conauto=0;
              SaveStat(992,WiFi.status());
            }
            else select2file();
          }
          /*if (SPIFFS.exists(symFILEFLAG)) {
             SPIFFS.remove(symFILEFLAG);*/
          if (ifreconnect()) {
             conauto=0;
             SaveStat(994,WiFi.status());
          }
          break; 
    }
}

void loop() {
  // put your main code here, to run repeatedly:
/*  if (millis()-mt>10000) {
      mt=millis();
  }
  if (reconnect) {
      WiFi.begin(sqlpar[0], sqlpar[1]);
  if(WiFi.status() != WL_CONNECTED) {
    checkwifi();
  }*/
  ConnectAuto();
  SerialAutoMakerLoop();
}

