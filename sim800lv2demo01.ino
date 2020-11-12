#include <SoftwareSerial.h>

//Create software serial object to communicate with SIM800L
SoftwareSerial gsm(2, 3); //SIM800L Tx & Rx is connected to Arduino #2 & #3

String buff;
String message;

void setup()
{
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and SIM800L
  gsm.begin(9600);

  Serial.println("Initializing..."); 
  delay(5000);  
  
  gsm.println("AT");
  sendGSMtoSerial();
  
  gsm.println("AT+CMGF=1");
  sendGSMtoSerial();
  
  gsm.println("AT+CNMI=1,2,0,0,0");
  sendGSMtoSerial();
}

void loop()
{
  if(Serial.available()) 
  {
    sendSerialToGSM();
  }
  if(gsm.available())
  {
    readFromGSM();
  }
}

void sendGSMtoSerial() {
  if (wait(10000)) Serial.println(gsm.readString());
}

void sendSerialToGSM() {
  gsm.println(Serial.readString());
  sendGSMtoSerial();
}

bool wait(unsigned long timeout)
{
  unsigned long start = millis();
  unsigned long delta = 0;
  
  while(!gsm.available()) {
    delta = millis() - start;
    if (delta >= timeout) {
      Serial.println("Timed out!");
      break;
    }
  }

  return gsm.available();
}

void readFromGSM()
{
  buff = gsm.readString();
  Serial.println(buff);

  if (buff.indexOf("+CMT:") != -1)
  {
    if (buff.indexOf("Status") != -1)
    {
      gsm.print("AT+CMGS=\"");
      gsm.print(buff.substring(9, 22));
      gsm.println("\"");
      sendGSMtoSerial();
      gsm.print("I'm online!");
      gsm.write(26);
      sendGSMtoSerial();
      sendGSMtoSerial();
    }
    else
    {
      message = buff.substring(51);
      message.trim();
      message.replace(" ", "+");
      
      gsm.println("AT+CREG?");
      sendGSMtoSerial();
      gsm.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
      sendGSMtoSerial();
      gsm.println("AT+SAPBR=3,1,\"APN\",\"internet\"");
      sendGSMtoSerial();
      gsm.println("AT+SAPBR=1,1");
      sendGSMtoSerial();
      gsm.println("AT+HTTPINIT");
      sendGSMtoSerial();
      gsm.println("AT+HTTPPARA=\"CID\",1");
      sendGSMtoSerial();
      gsm.print("AT+HTTPPARA=\"URL\",\"some.site.com/test-api/\?message=");
      gsm.print(message);
      gsm.println("\"");
      sendGSMtoSerial();
      gsm.println("AT+HTTPSSL=0");
      sendGSMtoSerial();
      
      gsm.println("AT+HTTPACTION=0");
      if (wait(10000)) {
        buff = gsm.readString();
        Serial.println(buff);
        if (buff.indexOf("ERROR") == -1) {
          sendGSMtoSerial();
          gsm.println("AT+HTTPREAD");
          sendGSMtoSerial();
        }
      }      
      
      gsm.println("AT+HTTPTERM");
      sendGSMtoSerial();
      gsm.println("AT+SAPBR=0,1");
      sendGSMtoSerial();
    }
  }
}
