#include "espsql.h"
#include <SoftwareSerial.h>

#include <DHT.h>
#define LIGHT  A0     
#define DHTPINA 10     
#define DHTPINB 11
#define DHTTYPE DHT22   

#define LED_PIN_DUE 13
#define LAMP220 12





SoftwareSerial esp8266(8, 9);
DHT dht1(DHTPINA, DHTTYPE);
DHT dht2(DHTPINB, DHTTYPE);

#define EspSerial esp8266

ESPSql es;


void setup(void) {

    int i;
    EspSerial.begin(9600);
    Serial.begin(115200);

    dht1.begin();
    dht2.begin();

    pinMode(LED_PIN_DUE,OUTPUT);
    digitalWrite(LED_PIN_DUE,LOW);
    pinMode(LAMP220,OUTPUT);
    digitalWrite(LAMP220,HIGH);
    Serial.println("Start");
  
    es.begin(&EspSerial);
    while(!es.config("MySSD","MyPass","192.168.2.106","mysqluser","mysqlpassword"));
    while(!es.setsql(1,"SELECT idrawacts,pinname,pinvalue FROM arduino.rawacts WHERE isnull(updated) and device='ROOF' order by stamp limit %1")) ;
    while(!es.setsql(2,"UPDATE arduino.rawacts SET updated=1 where idrawacts in ")) ;
    while(!es.setsql(3,"INSERT INTO arduino.rawdata(device,value_name,value_value) VALUES ")) ;
    while(!es.setsql(4,"INSERT INTO arduino.rawdata(device,value_name,value_value) VALUES ('ROOF',%1,%2)")) ;
    i=0;
    while(!es.ready()) {
      delay(200);
      if (i) digitalWrite(LED_PIN_DUE,HIGH);
      else digitalWrite(LED_PIN_DUE,LOW);
      i=1-i;
    }
    if (es.runsqladd(3,"('ROOF','WAKEUP',"+String(1)+")")) {
    }
    if (es.runsqlpar(1,"1")) {
    }
    digitalWrite(LED_PIN_DUE,LOW);
}

unsigned long xtime=millis();

unsigned long sumstemp=0;
unsigned long sumhum=0;
unsigned long sumlight=0;
unsigned int cnt=0;
unsigned long scaninterval=30000;


void loop(){
  mainloop();
}

void rundebug() {
  if ( Serial.available() ) { char c = Serial.read(); EspSerial.write(c); }
  if ( EspSerial.available() ) { char c = EspSerial.read(); Serial.write(c); }
}


void mainloop(void) { 

     sumlight=sumlight+analogRead(LIGHT);
     sumhum=sumhum+dht1.readHumidity();
     sumstemp=sumstemp+dht1.readTemperature();
     cnt++;
     if(xtime<millis()) {
        if (es.sqlready(3)) {
            Serial.println("PUT1");
            //if (es.runsqladd(3,"('ROOF','H1',"+String(dht1.readHumidity())+"),('ROOF','T1',"+String(dht1.readTemperature())+"),('ROOF','H2',"+String(dht2.readHumidity())+"),('ROOF','T2',"+String(dht2.readTemperature())+")")) {
            float light= sumlight/cnt;sumlight=0;
            float temp= sumstemp/cnt;sumstemp=0;
            float hum= sumhum/cnt;sumhum=0;
            cnt=0;
            if (es.runsqladd(3,"('ROOF','H1',"+String(hum)+"),('ROOF','T1',"+String(temp)+"),('ROOF','LIGHT',"+String(light)+")")) {
                xtime=millis()+scaninterval;
                Serial.println("PUT2");
            }
        }
     }
        if (es.sqlready(1)) {
          Serial.println("GET");
          
          int c=es.colnum(1);
          int r=es.rownum(1);
          String s="",x;
          for(int i=0;i<r;i++){
            for(int j=0;j<c;j++) {
               x=es.getvalue(1,j,i);
               if((i>0)&&(j==0)) {
                 if ((i+1)<r) s=s+x+",";
                 else s=s+x;
               }
            }
          }
          
          for(int i=1;i<r;i++){
               if (es.getvalue(1,1,i).indexOf("LED")>=0) {
                    analogWrite(LED_PIN_DUE,es.getvalue(1,2,i).toInt());
               }
               if (es.getvalue(1,1,i).indexOf("DLAMP")>=0) {
                    if (es.getvalue(1,2,i).toInt()==0) {
                       digitalWrite(LAMP220,HIGH);
                    }
                    else {
                       digitalWrite(LAMP220,LOW);
                    }
               }
               if (es.getvalue(1,1,i).indexOf("ScanInterval")>=0) {
                    scaninterval=es.getvalue(1,2,i).toInt();
               }
          }
          
          
          
          if(s.length()>0) {
            if (es.runsqladd(2,"("+s+")")) {
                while(!es.sqlready(2)) delay(300);
            }
          }
          while (!es.runsqlpar(1,"10")) delay(300);
          Serial.println("GET END");
        }
     
}



