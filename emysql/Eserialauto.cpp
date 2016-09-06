#include <string.h>
#include <ESP8266WiFi.h>
#include <fs.h>
#include <eserialauto.h>




#define LOGERROR(x)    if(_ESPSQLLOGLEVEL_>0) { Serial.print("$#SER "); Serial.println(x); }
#define LOGERROR1(x,y) if(_ESPSQLLOGLEVEL_>0) { Serial.print("$#SER "); Serial.print(x); Serial.print(" "); Serial.println(y); }
#define LOGWARN(x)     if(_ESPSQLLOGLEVEL_>1) { Serial.print("$#SER "); Serial.println(x); }
#define LOGWARN1(x,y)  if(_ESPSQLLOGLEVEL_>1) { Serial.print("$#SER "); Serial.print(x); Serial.print(" "); Serial.println(y); }
#define LOGINFO(x)     if(_ESPSQLLOGLEVEL_>2) { Serial.print("$#SER "); Serial.println(x); }
#define LOGINFO1(x,y)  if(_ESPSQLLOGLEVEL_>2) { Serial.print("$#SER "); Serial.print(x); Serial.print(" "); Serial.println(y); }

#define LOGDEBUG(x)      if(_ESPSQLLOGLEVEL_>3) { Serial.print("$#SER "); Serial.println(x); }
#define LOGDEBUG0(x)      if(_ESPSQLLOGLEVEL_>3) { Serial.print(x); }
#define LOGDEBUG1(x,y)   if(_ESPSQLLOGLEVEL_>3) { Serial.print("$#SER "); Serial.print(x); Serial.print(" "); Serial.println(y); }
#define LOGDEBUG2(x,y,z) if(_ESPSQLLOGLEVEL_>3) { Serial.print("$#SER "); Serial.print(x); Serial.print(" "); Serial.print(y); Serial.print(" "); Serial.println(z); }



int _ESPSQLLOGLEVEL_=0;

#define ESPSQLCOMMANDSNUM 8
const char*  ESPSQLCOMMANDS[] PROGMEM =
{
    "$par#",
    "$sql#",
    "$exe#",
    "$res#",
    "$deb#",
    "$clr#",
    "$sts#",
    "$cfg#"
};
  

typedef struct {
  int code,colnum,rownum; 
} tsqlstat;

tsqlstat sqlstat[MaxSqlStr+1];

int connstat=0;
int wifistat=0;
bool sqlreconnect=false;


int lp(String in,String dv) {
  int f=0,idx=1;
  while (in.indexOf(dv,f)>=0) {
      f=in.indexOf(dv,f)+dv.length();
      idx++;
  }
  return idx;
}



String ex(String in,String dv,int idx) {
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



String getfromfile(int sqlnum, int row, int col){
    File f;
    String ss="";
    f = SPIFFS.open("sqlbufer"+String(sqlnum), "r");
    if (f) {
        if (f.available()) {
          ss=f.readStringUntil(10);
          int c=ss.toInt();
          c=row*c+col+1;
          while((c>0)&&(f.available())) { ss=f.readStringUntil(10); c--; }
          LOGERROR1("str:",ss);
        }
    }
    f.close();
    return ss;
}

void atPar(String cmdSQL) {

            char sqlparametrs[5][32];
            int baud;

            if (lp(cmdSQL,symSepA)==5) {
              for(int i=0;i<5;i++){ String x=ex(cmdSQL,symSepA,i+1); x.toCharArray(sqlparametrs[i],x.length()+1); LOGDEBUG1("cmdSQL=",sqlparametrs[i]); }

              File f;
              f = SPIFFS.open(symFILE, "w");
              LOGDEBUG("SAVEINI");
              for(int i=0;i<5;i++){
                f.print(sqlparametrs[i]);
                //if (i<4) 
                f.print(symSepA);
                LOGDEBUG1(i,sqlparametrs[i]);
              }
              f.print(Serial.baudRate());
              f.println();
              f.println();
              f.close();
              //f = SPIFFS.open(symFILEFLAG, "w");f.println();f.println();f.close();
	      sqlreconnect=true;

  
              /*if (conn.connected()) { conn.close(); client.stop(); }
              WiFi.begin(sqlparametrs[0], sqlparametrs[1]);*/
              Serial.println(101);
            }
            else {
              Serial.println(102);
            }
}
void atSql(String cmdSQL) {
            if (lp(cmdSQL,symSepB)==2) {
              int i=ex(cmdSQL,symSepB,1).toInt();
              LOGDEBUG1("SQL #",i);
              LOGDEBUG1("SQL $",ex(cmdSQL,symSepB,2));
              if (i>0 && i<=MaxSqlStr) { 
                File f = SPIFFS.open("SQL"+String(i), "w"); f.println(ex(cmdSQL,symSepB,2)); f.close();  
                Serial.print(110); Serial.print(","); Serial.println(i);
              }
              else {
                 Serial.print(112); Serial.print(","); Serial.println(i);
              }
            }
            else Serial.println(111);
}
void atExe(String cmdSQL) {

            int q=0;
            
            if (lp(cmdSQL,symSepB)==2) {
              q=ex(cmdSQL,symSepB,1).toInt();
              LOGDEBUG1("SQL #",q);
              LOGDEBUG1("SQL $",ex(cmdSQL,symSepB,2));
              if (q>0 && q<=MaxSqlStr) { 
                File f = SPIFFS.open("SQL"+String(q), "r");
                cmdSQL=f.readStringUntil('\n')+ex(cmdSQL,symSepB,2);
                f.close();
              }
              else cmdSQL="";
            }
            else {
              if (lp(cmdSQL,symSepA)>0) {
                q=ex(cmdSQL,symSepA,1).toInt();
                LOGDEBUG1("SQL #",q);
                if (q>0 && q<=MaxSqlStr) { 
                    SPIFFS.remove("sqlbufer"+String(q));
                    //SPIFFS.remove("sqlstat"+String(q));
                    sqlstat[q].code=0;
                    sqlstat[q].colnum=0;
                    sqlstat[q].rownum=0; 
                    File f = SPIFFS.open("SQL"+String(q), "r");
                    String tmpsql=f.readStringUntil('\n');
                    f.close();
                    LOGDEBUG1("SQL:",tmpsql);
                    for(int i=2;i<=lp(cmdSQL,symSepA);i++) {
                      if(lp(tmpsql,symPar+String(i-1))>1) {
                        int j=1;
                        String ts=ex(tmpsql,symPar+String(i-1),j);
                        //LOGDEBUG1("ts0",ts);
                        ts=ts+ex(cmdSQL,symSepA,i);
                        //LOGDEBUG1("ts1",ts);
                        j++;
                        while(j<lp(tmpsql,symPar+String(i-1))){
                            ts=ts+ex(tmpsql,symPar+String(i-1),j);
                            //LOGDEBUG1("ts0",ts);
                            ts=ts+ex(cmdSQL,symSepA,i);
                            //LOGDEBUG1("ts1",ts);
                            j++;
                        }
                        ts=ts+ex(tmpsql,symPar+String(i-1),j);
                        tmpsql=ts;
                        //LOGDEBUG1("tmpSQL:",tmpsql);
                      }
                    }
                    cmdSQL=tmpsql;
                }
                else cmdSQL="";
              }
              else cmdSQL="";
            }
            LOGDEBUG1("cmdSQL:",cmdSQL);
            if ((q>0)&&(cmdSQL.length()>2)) {
              File z;z = SPIFFS.open("sqlstring"+String(q), "w");
              z.println(cmdSQL);
              z.close();
              SPIFFS.remove("sqlbufer"+String(q));
              //SPIFFS.remove("sqlstat"+String(q));
              sqlstat[q].code=0;
              sqlstat[q].colnum=0;
              sqlstat[q].rownum=0; 

              //select2file(ex(cmdSQL,symSepB,1).toInt(),cmdSQL); //-------
              Serial.print(120); Serial.print(","); Serial.println(q);
            }
            else {
              Serial.println(121);
            }
}

void atRes(String cmdSQL) {
            if (lp(cmdSQL,symSepA)==3) {
              int q=ex(cmdSQL,symSepA,1).toInt();
              if (SPIFFS.exists("sqlbufer"+String(q))) {
                Serial.print(130); Serial.print(","); Serial.println(q);
                Serial.println(getfromfile(q,ex(cmdSQL,symSepA,3).toInt(),ex(cmdSQL,symSepA,2).toInt()));
              }
              else Serial.println(131);
            }
            if (lp(cmdSQL,symSepA)==2) {
              if (SPIFFS.exists("sqlbufer"+String(0))) {
                Serial.println(130);
                Serial.println(getfromfile(0,ex(cmdSQL,symSepA,2).toInt(),ex(cmdSQL,symSepA,1).toInt()));
              }
              else Serial.println(131);
            }
}
void atDeb(String cmdSQL) {
              _ESPSQLLOGLEVEL_=cmdSQL.toInt();
              Serial.print(140);Serial.print(","); Serial.println(_ESPSQLLOGLEVEL_);
}
void atClr(String cmdSQL) {
              SPIFFS.remove(symFILE);
              //File f = SPIFFS.open(symFILEFLAG, "w");f.println();f.println();f.close();
	      sqlreconnect=true;	
              LOGDEBUG("RESET");
              Serial.println(150);
              //soft_reset();
}
void atStat(String cmdSQL) {
    String tmpsql;
    int q;
    File f;
              q=cmdSQL.toInt();
              if (q<=MaxSqlStr) {
                /*f = SPIFFS.open("sqlstat"+String(q), "r");
                tmpsql=f.readStringUntil('\n');
                f.close();*/
                Serial.print(160);Serial.print(","); 
                Serial.print(sqlstat[q].code);Serial.print(","); 
                Serial.print(q);Serial.print(","); 
                Serial.print(sqlstat[q].colnum);Serial.print(","); 
                Serial.println(sqlstat[q].rownum); 

		//Serial.println(tmpsql);
              }
              else {
                /*f = SPIFFS.open("connstat", "r");
                tmpsql=f.readStringUntil('\n');
                f.close();*/
                Serial.print(161);Serial.print(","); Serial.print(connstat);Serial.print(","); Serial.println(wifistat);
		//Serial.println(tmpsql);
              }
}

void atReadall(String cmdSQL) {

    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();
    switch(ex(cmdSQL,symSepA,1).toInt()){
      case 0:
        Serial.print("170,");
        Serial.print(ESP.getResetReason());
        Serial.printf(",%08X,%u,%u,%u,%s,",ESP.getFlashChipId(),realSize,ideSize,ESP.getFlashChipSpeed(),
          (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
        if(ideSize != realSize) {
            Serial.println("CFGBAD");
        } else {
            Serial.println("OK");
        }
        break;
      case 1:
        if (lp(cmdSQL,symSepA)==2) {
          Serial.begin(ex(cmdSQL,symSepA,2).toInt());
        }
        break;
    }

}



void SerialAutoMakerLoop() {
  if(Serial.available()) {
     String samcmdSQL="";
     int samcmd=0,res;
     LOGDEBUG("*");
     samcmdSQL = Serial.readStringUntil('\n');
     LOGDEBUG1("STR=",samcmdSQL);
     while (samcmd<ESPSQLCOMMANDSNUM) {
       if (samcmdSQL.startsWith(ESPSQLCOMMANDS[samcmd])) break;
       samcmd++;
     }
     if(samcmd<ESPSQLCOMMANDSNUM) {
        LOGDEBUG1("CMD=",samcmd);
        samcmdSQL=ex(samcmdSQL,ESPSQLCOMMANDS[samcmd],2);
        //.remove(0,sizeof(ESPSQLCOMMANDS[samcmd]));
        LOGDEBUG1("STR=",samcmdSQL);
        switch(samcmd) {
          case 0: atPar(samcmdSQL); break;
          case 1: atSql(samcmdSQL); break;
          case 2: atExe(samcmdSQL);  break;
          case 3: atRes(samcmdSQL); // $res#1,1,1          $res#0,1,1
          break;
          case 4: atDeb(samcmdSQL); 
          break;
          case 5: atClr(samcmdSQL); 
          break;
          case 6: atStat(samcmdSQL); 
          break;
          case 7: atReadall(samcmdSQL);
          break;
        }
     }
     else {
      if ((samcmdSQL.length()<1000)&&(samcmdSQL.length()>2)) {
              File z;z = SPIFFS.open("sqlstring"+String(0), "w");
              z.println(samcmdSQL);
              z.close();
              SPIFFS.remove("sqlbufer"+String(0));
              //SPIFFS.remove("sqlstat"+String(0));
              sqlstat[0].code=0;
              sqlstat[0].colnum=0;
              sqlstat[0].rownum=0; 

              //select2file(ex(cmdSQL,symSepB,1).toInt(),cmdSQL); //-------
              Serial.print(120); Serial.print(","); Serial.println(0);
      }
      else Serial.println(122);
     }
  }
}



void mysdelay(unsigned long ddd) {
   unsigned long mtt;
   mtt=millis();
   while((mtt-millis())<ddd) {
      SerialAutoMakerLoop();
      //LOGDEBUG0(".");
   }
}

void storestatsql(int s, int cd,int c,int r) {
     sqlstat[s].code=cd;
     sqlstat[s].colnum=c;
     sqlstat[s].rownum=r;
}


void SaveStat(int x,int z) {
     
     connstat=x;
     wifistat=z;
     /*File f;
     f = SPIFFS.open("connstat", "w");
     f.print(x);
     f.print(",");
     f.println(WiFi.status());
     f.close();*/
     
}

bool ifreconnect(void) {
	if (sqlreconnect) {
		sqlreconnect=false;	
		return true;
	}
	else {
		return false;		
	}

}