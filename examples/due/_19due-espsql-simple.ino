#include "espsql.h"
#define EspSerial Serial3
#define ESP_CH_PD 51
#define ESP_RST   50
#define LED_PIN_DUE 13

#define Light  A7
#define ASmog  A6


ESPSql es;

void debugloop() {
  if ( Serial.available() ) { char c = Serial.read(); EspSerial.write(c); }
  if ( EspSerial.available() ) { char c = EspSerial.read(); Serial.write(c); }
}

void setup(void) {

    int i;
    Serial.begin(115200);
    EspSerial.begin(9600);
    
    pinMode(ESP_CH_PD,OUTPUT);
    pinMode(ESP_RST,OUTPUT);

    //digitalWrite(ESP_CH_PD,HIGH);delay(1);
    //digitalWrite(ESP_RST,LOW);delay(1);
    digitalWrite(ESP_RST,HIGH);  
    digitalWrite(ESP_CH_PD,HIGH);
    delay(1200);

    pinMode(LED_PIN_DUE,OUTPUT);
    digitalWrite(LED_PIN_DUE,LOW);

    Serial.println("begin");

    
    es.begin(&EspSerial);
    while(!es.config("MySSD","MyPass","192.168.2.106","mysqluser","mysqlpassword")) Serial.print("c?");
    while(!es.setsql(1,"SELECT idrawacts,pinname,pinvalue FROM arduino.rawacts WHERE isnull(updated) and device='DT' order by stamp limit %1")) Serial.print("1?");
    while(!es.setsql(2,"UPDATE arduino.rawacts SET updated=1 where idrawacts in ")) Serial.print("2?");
    while(!es.setsql(3,"INSERT INTO arduino.rawdata(device,value_name,value_value) VALUES ")) Serial.print("3?");
    while(!es.setsql(4,"INSERT INTO arduino.rawdata(device,value_name,value_value) VALUES ('DT',%1,%2)")) Serial.print("4?");
    i=0;
    while(!es.ready()) {
      delay(200);
      if (i) digitalWrite(LED_PIN_DUE,HIGH);
      else digitalWrite(LED_PIN_DUE,LOW);

      i=1-i;
    }
    while (!es.runsqladd(3,"('DT','TEST1',"+String(analogRead(Light))+"),('DT','TEST2',"+String(analogRead(ASmog))+")")) {
      Serial.print("5?");
    }
    while (!es.runsqlpar(1,"10")) {
      Serial.print("6?");
    }
    digitalWrite(LED_PIN_DUE,LOW);
    Serial.println();
}

unsigned long xtime=millis();


void loop(void) { 
        if (es.sqlready(3)) {
            if (es.runsqladd(3,"('DT','TEST1',"+String(analogRead(Light))+"),('DT','TEST2',"+String(analogRead(ASmog))+")")) {
                Serial.println(millis()-xtime);
                xtime=millis();
            }
        }
        if (es.sqlready(1)) {
          digitalWrite(LED_PIN_DUE,HIGH);
          
          int c=es.colnum(1);
          int r=es.rownum(1);
          String s="",x;
          for(int i=0;i<r;i++){
            for(int j=0;j<c;j++) {
               x=es.getvalue(1,j,i);

               Serial.print(x);
               Serial.print(" ");

               
               if((i>0)&&(j==0)) {
                 if ((i+1)<r) s=s+x+",";
                 else s=s+x;
               }
            }
            Serial.println();
          }
          Serial.println(s);
          
          for(int i=1;i<r;i++){
               if (es.getvalue(1,1,i).indexOf("LED")>=0) {
                    analogWrite(LED_PIN_DUE,es.getvalue(1,2,i).toInt());
               }
               if (es.getvalue(1,1,i).indexOf("TESTA")>=0) {
                    if (es.getvalue(1,2,i).toInt()==0) {
                       digitalWrite(LED_PIN_DUE,HIGH);
                       Serial.println("digitalWrite(RelayA,HIGH);");
                    }
                    else {
                       digitalWrite(LED_PIN_DUE,LOW);
                       Serial.println("digitalWrite(RelayA,LOW);");
                    }
               }
               if (es.getvalue(1,1,i).indexOf("TESTBB")>=0) {
                    if (es.getvalue(1,2,i).toInt()==0) {
                       Serial.println("digitalWrite(RelayB,HIGH);");
                    }
                    else {
                       Serial.println("digitalWrite(RelayB,LOW);");
                    }
               }
          }
          
          
          
          if(s.length()>0) {
            if (es.runsqladd(2,"("+s+")")) {
                while(!es.sqlready(2)) delay(300);
            }
          }
          while (!es.runsqlpar(1,"10")) delay(300);
          digitalWrite(LED_PIN_DUE,LOW);
        }
     

}


