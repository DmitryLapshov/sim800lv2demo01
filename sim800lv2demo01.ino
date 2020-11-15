#include <SoftwareSerial.h>
// #define _SS_MAX_RX_BUFF 128 // RX buffer size in the \hardware\arduino\avr\libraries\SoftwareSerial\src\SoftwareSerial.h

SoftwareSerial gsm(2, 3); //SIM800L Tx & Rx is connected to Arduino #2 & #3

String buff;
String message;
String mynumber = "%2b"; // html encoded '+'
String token = "1234567890";

void setup()
{
  Serial.begin(9600);
  
  gsm.begin(9600);

  Serial.println("Initializing..."); 
  delay(5000);  
  
  gsm.println("AT");
  sendGSMtoSerial();
  
  gsm.println("AT+CMGF=1");
  sendGSMtoSerial();
  
  gsm.println("AT+CNMI=1,2,0,0,0");
  sendGSMtoSerial();

  gsm.println("AT+CNUM");
  sendGSMtoSerial();
  mynumber.concat(buff.substring(22, 34));
  Serial.println(mynumber);
}

void loop()
{
  sendSerialToGSM();
  readFromGSM();
}

void sendGSMtoSerial() {
  if (wait(10000)) {
    buff = gsm.readString();
    Serial.println(buff);
  }
  else {
    buff = "";
  }
}

void sendSerialToGSM() {
  if(!Serial.available()) return;
  
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
  if (!gsm.available()) return;

  sendGSMtoSerial();

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
      
      if (buff.indexOf("+CREG: 0,1") != -1) { // if connected to the network
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
        gsm.print("AT+HTTPPARA=\"URL\",\"web.hosting-test.net/api/?token=");
        gsm.print(token);
        gsm.print("&source=");
        gsm.print(mynumber);
        gsm.print("&type=");
        gsm.print("0");
        gsm.print("&value=");
        gsm.print(message);
        gsm.println("\"");
        // "web.hosting-test.net/api/?token=1234567890&source=%2b38095XXXXXXX&type=0&value=Test+is+a+test+01"
        sendGSMtoSerial();
        gsm.println("AT+HTTPSSL=0");
        sendGSMtoSerial();
        
        gsm.println("AT+HTTPACTION=0");
        sendGSMtoSerial();
        
        if (buff.indexOf("ERROR") == -1) {
          sendGSMtoSerial();
          gsm.println("AT+HTTPREAD");
          sendGSMtoSerial();
        }
        
        gsm.println("AT+HTTPTERM");
        sendGSMtoSerial();
        gsm.println("AT+SAPBR=0,1");
        sendGSMtoSerial();
      }
    }
  }
}