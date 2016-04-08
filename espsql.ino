#include <stdio.h>

int _ESPSQLLOGLEVEL_=0;


#define LOGERROR(x)    if(_ESPSQLLOGLEVEL_>0) { Serial.print("[EspSQL] "); Serial.println(x); }
#define LOGERROR1(x,y) if(_ESPSQLLOGLEVEL_>2) { Serial.print("[EspSQL] "); Serial.print(x); Serial.print(" "); Serial.println(y); }
#define LOGWARN(x)     if(_ESPSQLLOGLEVEL_>1) { Serial.print("[EspSQL] "); Serial.println(x); }
#define LOGWARN1(x,y)  if(_ESPSQLLOGLEVEL_>2) { Serial.print("[EspSQL] "); Serial.print(x); Serial.print(" "); Serial.println(y); }
#define LOGINFO(x)     if(_ESPSQLLOGLEVEL_>2) { Serial.print("[EspSQL] "); Serial.println(x); }
#define LOGINFO1(x,y)  if(_ESPSQLLOGLEVEL_>2) { Serial.print("[EspSQL] "); Serial.print(x); Serial.print(" "); Serial.println(y); }

#define LOGDEBUG(x)      if(_ESPSQLLOGLEVEL_>3) { Serial.println(x); }
#define LOGDEBUG0(x)     if(_ESPSQLLOGLEVEL_>3) { Serial.print(x); }
#define LOGDEBUG1(x,y)   if(_ESPSQLLOGLEVEL_>3) { Serial.print(x); Serial.print(" "); Serial.println(y); }
#define LOGDEBUG2(x,y,z) if(_ESPSQLLOGLEVEL_>3) { Serial.print(x); Serial.print(" "); Serial.print(y); Serial.print(" "); Serial.println(z); }

#include <ESP8266WiFi.h>
#include "mysql/MySQL_Connection.h"
#include "mysql/MySQL_Cursor.h"
#include <fs.h>

#define symFILE "/espsql1.ini"
#define symINI "[espsqlcfg]"
#define symSSID "ssid="
#define symPASS "pass="
#define symMYUSER "myuser="
#define symMYPASS "mypass="
#define symSERVER "myip="


int status = WL_IDLE_STATUS;

IPAddress server_addr(192,168,2,106);  // IP of the MySQL *server* here

// Initialize the client library
WiFiClient client;
MySQL_Connection conn((Client *)&client);

char wifiSSID[32];
char wifiPASS[32];
char myUSER[32];
char myPASS[32];

/**
 * Start Wifi connection
 * if passphrase is set the most secure supported mode will be automatically selected
 * @param ssid const char*          Pointer to the SSID string.
 * @param passphrase const char *   Optional. Passphrase. Valid characters in a passphrase must be between ASCII 32-126 (decimal).
 * @param bssid uint8_t[6]          Optional. BSSID / MAC of AP
 * @param channel                   Optional. Channel of AP
 * @param connect                   Optional. call connect
 * @return
 
wl_status_t ESP8266WiFiSTAClass::begin(const char* ssid, const char *passphrase, int32_t channel, const uint8_t* bssid, bool connect) {
*/

bool checkini(void){
  File f;
  String ss;
  f = SPIFFS.open(symFILE, "r");
  if (f) {
    if (f.find(symINI)) {
          LOGDEBUG("cfg -ok");
          do{
            ss=f.readStringUntil(10);
            LOGDEBUG1("get string:",ss);
            if (ss.startsWith(symSSID)) {
              ss=ss.substring(5);
              ss.toCharArray(wifiSSID,ss.length());
              LOGDEBUG1("wifiSSID=",wifiSSID);
            }
            if (ss.startsWith(symPASS)) {
              ss=ss.substring(5);
              ss.toCharArray(wifiPASS,ss.length());
              LOGDEBUG1("wifiPASS=",wifiPASS);
            }
            
            if (ss.startsWith(symMYUSER)) {
              ss=ss.substring(7);
              ss.toCharArray(myUSER,ss.length());
              LOGDEBUG1("myUSER=",myUSER);
            }
            
            if (ss.startsWith(symMYPASS)) {
              ss=ss.substring(7);
              ss.toCharArray(myPASS,ss.length());
              LOGDEBUG1("myPASS=",myPASS);
            }
            if (ss.startsWith(symSERVER)) {
              int a=255,b=255,c=255,d=255;
              char ipstr[32];
              ss=ss.substring(5);
              ss.toCharArray(ipstr,ss.length());
              server_addr.fromString(ipstr);
              LOGDEBUG1("SERVER=",server_addr);
            
            }
          }while (f.available()>0);
        return true;        
    }
  }
  LOGERROR("Init file open failed");
  SPIFFS.format();
  updateini();
  LOGDEBUG("Updated");
  return false;
}

void updateini(void)
{
  File f;
  f = SPIFFS.open(symFILE, "w");
  f.println(symINI);
  f.print(symSSID);
  f.println(wifiSSID);
  f.print(symPASS);
  f.println(wifiPASS);
  f.print(symMYUSER);
  f.println(myUSER);
  f.print(symMYPASS);
  f.println(myPASS);
  f.print(symSERVER);
  f.println(server_addr);
  f.close();
}

bool checkwifi()
{
  int cnt=10;  
  LOGDEBUG1("*** Connecting ",wifiSSID);
  WiFi.begin(wifiSSID, wifiPASS);
  
  while ((WiFi.status() != WL_CONNECTED)&&(cnt>0)) {
    delay(500);
    LOGDEBUG0(".");
    cnt--;
  }
  if(WiFi.status() == WL_CONNECTED) {
    LOGDEBUG("");
    LOGDEBUG1("*** WiFi connected. HostName:",WiFi.hostname());
    LOGDEBUG1("*** IP:",WiFi.localIP());
    return true;
  }
  else {
    LOGERROR("WIFI ERROR");
    return false;
  }
}

bool checksql()
{
  int cnt=10;  
  if (conn.connect(server_addr, 3306, myUSER, myPASS)) {
    return true;
  }
  LOGERROR1("$$$ Can't connect to server:",server_addr);
  LOGERROR1("$$$ user:",myUSER);
  return false;
}


void setup() {
  //pinMode(16,OUTPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println("ESPSQL v1.1");
  //delay(100);
  if(_ESPSQLLOGLEVEL_>3) {readall();}
  SPIFFS.begin();

  while(!checkini());
  if(checkwifi()) {
     if(_ESPSQLLOGLEVEL_>3) {WiFi.printDiag(Serial);}
     if(checksql()) {
          Serial.print("SQL>");
     }
  }
    
}

void soft_reset() {
  pinMode(0,OUTPUT);
  digitalWrite(0,1);
  pinMode(2,OUTPUT);
  digitalWrite(2,1);
  ESP.restart();
} 



  char c;
  long int mt=millis();
  bool g16f=false;
  String cmdSQL="";


void loop() {
  // put your main code here, to run repeatedly:
  if (millis()-mt>10000) {
      mt=millis();
      ESP.getCycleCount();
  }
  if(Serial.available()) {
    c=Serial.read();
    if (c>31) {
        cmdSQL=cmdSQL+c;
    }
    else {
        if((c==13)||(c==10)) {
          Serial.println(cmdSQL);
          if (cmdSQL.startsWith("$$")) {
              cmdSQL=cmdSQL.substring(2);
              if (cmdSQL.startsWith("id#")) {
                  cmdSQL=cmdSQL.substring(3);
                  cmdSQL.toCharArray(wifiSSID,cmdSQL.length()+1);
                  LOGDEBUG1("wifiSSID=",wifiSSID);
                  cmdSQL="";
              }
              if (cmdSQL.startsWith("ps#")) {
                  cmdSQL=cmdSQL.substring(3);
                  cmdSQL.toCharArray(wifiPASS,cmdSQL.length()+1);
                  LOGDEBUG1("wifiPASS=",wifiPASS);
                  cmdSQL="";
              }
              if (cmdSQL.startsWith("mu#")) {
                  cmdSQL=cmdSQL.substring(3);
                  cmdSQL.toCharArray(myUSER,cmdSQL.length()+1);
                  LOGDEBUG1("myUSER=",myUSER);
                  cmdSQL="";
              }
              if (cmdSQL.startsWith("mp#")) {
                  cmdSQL=cmdSQL.substring(3);
                  cmdSQL.toCharArray(myPASS,cmdSQL.length()+1);
                  LOGDEBUG1("myPASS=",myPASS);
                  cmdSQL="";
              }
              if (cmdSQL.startsWith("dl#")) {
                  cmdSQL=cmdSQL.substring(3);
                  _ESPSQLLOGLEVEL_=cmdSQL.toInt();
                  LOGDEBUG1("_ESPSQLLOGLEVEL_=",_ESPSQLLOGLEVEL_);
                  cmdSQL="";
              }
              if (cmdSQL.startsWith("ms#")) {
                  int a=255,b=255,c=255,d=255;
                  char ipstr[32];
                  cmdSQL=cmdSQL.substring(3);
                  cmdSQL.toCharArray(ipstr,cmdSQL.length()+1);
                  server_addr.fromString(ipstr);
                  LOGDEBUG1("SERVER=",server_addr);
                  cmdSQL="";
              }
              if (cmdSQL.startsWith("ss")) {
                  updateini();
                  soft_reset();
              }
              if (cmdSQL.startsWith("zz")) {
                if(checkwifi()) {
                  if(_ESPSQLLOGLEVEL_>3) {WiFi.printDiag(Serial);}
                  if(checksql()) {
                     updateini();
                     Serial.println("SQL ok");
                  }
                }
                cmdSQL="";
              }
          }
          else {
            select(cmdSQL);
          }
          cmdSQL="";
          Serial.print("SQL>");
        }
    }
  }
}

void select(String cmd){
   char tmpsql[1000];
   long head_count = 0;

   if (conn.connected()) {
      MySQL_Cursor cur = MySQL_Cursor(&conn);
      if (cmd.length()<1000) {
          cmd.toCharArray(tmpsql, cmd.length()+1);
          if (cur.execute(tmpsql)){
              column_names *columns = cur.get_columns();
              for (int f = 0; f < columns->num_fields; f++) {
                Serial.print(columns->fields[f]->name);
                if (f < columns->num_fields-1) {
                  Serial.print(',');
                }
              }
              Serial.println();
              row_values *row = NULL;
              do {
                row = cur.get_next_row();
                if (row != NULL) {
                  for (int f = 0; f < columns->num_fields; f++) {
                    Serial.print('"');
                    Serial.print(row->values[f]);
                    Serial.print('"');
                    if (f < columns->num_fields-1) {
                       Serial.print(',');
                    }
                  }
                  Serial.println();
                }
              } while (row != NULL);
          }
          else {
             Serial.println("*** NULL");
          }

      }
      cur.close();
  }
  else {
  LOGERROR1("$$$ Can't connect to server:",server_addr);
  LOGERROR1("$$$ user:",myUSER);
  }
}



void readall() {

    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();

    Serial.print("*** Wake up after ");
    Serial.println(ESP.getResetReason());
    Serial.printf("*** Flash real id:   %08X\n", ESP.getFlashChipId());
    Serial.printf("*** Flash real size: %u\n\n", realSize);
    Serial.printf("*** Flash ide  size: %u\n", ideSize);
    Serial.printf("*** Flash ide speed: %u\n", ESP.getFlashChipSpeed());
    Serial.printf("*** Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

    if(ideSize != realSize) {
        Serial.println("$$$ Flash Chip configuration wrong!\n");
    } else {
        Serial.println("*** Flash Chip configuration ok.\n");
    }

}

