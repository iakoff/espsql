#include <stdio.h>

#define EspSerial Serial3
#define ESP_CH_PD 51
#define ESP_RST   50
#define LED_PIN_DUE 13


char ssid[] = "SSSSSS";            // your network SSID (name)
char pass[] = "PPPPPPP";        // your network password

int ledStatus = LOW;
int ndigi[20],nanal[20];
int odigi[20],oanal[20];


unsigned long mytime;


void scanall(void)
{
  int i;
  Serial.println("scanall");
  for(i=0;i<14;i++){
     ndigi[i]=digitalRead(i);
     if(ndigi[i]!=odigi[i]) {
        //Serial.print("D");Serial.print(i);Serial.print("=");Serial.println(ndigi[i]);
        odigi[i]=ndigi[i];
        EspSerial.print("INSERT INTO arduino.rawdata(value_name,value_value) VALUES ('D");EspSerial.print(i);EspSerial.print("','");EspSerial.print(ndigi[i]);EspSerial.println("')");
        while(!EspSerial.find("***")) { delay(100); }
        while (EspSerial.available()) { Serial.write(EspSerial.read()); }
     }
  }
  for(int i=0;i<7;i++){
     nanal[i]=analogRead(i);
     if(nanal[i]!=oanal[i]) {
        //Serial.print("A");Serial.print(i);Serial.print("=");Serial.println(nanal[i]);
        oanal[i]=nanal[i];
        EspSerial.print("INSERT INTO arduino.rawdata(value_name,value_value) VALUES ('A");EspSerial.print(i);EspSerial.print("','");EspSerial.print(nanal[i]);EspSerial.println("')");
        while(!EspSerial.find("***")) { delay(100); }
        while (EspSerial.available()) { Serial.write(EspSerial.read()); }
     }
  }
}

/*SELECT pinname,pinvalue FROM arduino.rawacts WHERE isnull(updated) order by stamp limit 1;*/


void actsas(void)
{
        EspSerial.println("SELECT idrawacts,pinname,pinvalue FROM arduino.rawacts WHERE isnull(updated) order by stamp desc limit 1");
        Serial.println("actsas");
        while(!EspSerial.find("pinname,pinvalue")) { delay(100); }
        while(!EspSerial.find("pinname,pinvalue")) { delay(100); }
        int id = EspSerial.parseInt();
        if(id>0){
          int pin = EspSerial.parseInt();
          int value = EspSerial.parseInt();
          Serial.print("D");Serial.print(pin);Serial.print("=");Serial.println(value);
          digitalWrite(pin,value);
          while(!EspSerial.find("SQL>")) { delay(100); }

          EspSerial.print("UPDATE arduino.rawacts SET updated=1 where idrawacts=");
          EspSerial.println(id, DEC);
          while(!EspSerial.find("SQL>")) { delay(100); }
        }
}

void setup(void)
{
    
    pinMode(LED_PIN_DUE,OUTPUT);
    pinMode(ESP_CH_PD,OUTPUT);
    pinMode(ESP_RST,OUTPUT);
    Serial.begin(115200);

    EspSerial.begin(115200);
    
    Serial.print("setup1 begin\r\n");
    digitalWrite(ESP_RST,HIGH);delay(100);
    digitalWrite(ESP_CH_PD,HIGH);
    digitalWrite(LED_PIN_DUE,HIGH);delay(100);digitalWrite(LED_PIN_DUE,LOW);delay(100);
    digitalWrite(LED_PIN_DUE,HIGH);delay(100);digitalWrite(LED_PIN_DUE,LOW);delay(100);
    digitalWrite(LED_PIN_DUE,HIGH);delay(100);digitalWrite(LED_PIN_DUE,LOW);delay(100);

    //wifi.begin(115200);
    mytime = millis();
    while(!EspSerial.find("SQL>")) { delay(100); Serial.print(".");}
    Serial.println(".");
}

void loop(void)
{
   char c;

 
    while (EspSerial.available()) { Serial.write(EspSerial.read()); }
    while (Serial.available()) {  
        c=Serial.read(); 
        if (c=='`')scanall();
        else {
          if (c=='~')actsas();
          else EspSerial.write(c);  
        }
    }
    if (millis() - mytime > 5000) {
          mytime = millis();
          actsas();scanall();
    }
    

}


