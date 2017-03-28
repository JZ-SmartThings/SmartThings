 /**
 *  Arduino Nano & Ethernet Shield Sample v1.0.20170327
 *  Source code can be found here: https://github.com/JZ-SmartThings/SmartThings/blob/master/Devices/Generic%20HTTP%20Device
 *  Copyright 2017 JZ
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 */
#include <UIPEthernet.h>
#include <EEPROM.h>

// DESIGNATE WHICH PINS ARE USED FOR TRIGGERS
// IF USING 3.3V RELAY, TRANSISTOR OR MOSFET THEN TOGGLE Use5Vrelay TO FALSE VIA UI
int relayPin1 = 2; // GPIO5 = D2
int relayPin2 = 3; // GPIO6 = D3
bool Use5Vrelay; // Value defaults by reading eeprom in the setup method

// DESIGNATE CONTACT SENSOR PINS.
#define SENSORPIN 5     // what pin is the Contact Sensor on?
#define SENSORPIN2 6     // what pin is the 2nd Contact Sensor on?

// OTHER VARIALBES
String currentIP;

void(* resetFunction) (void) = 0;

EthernetServer server = EthernetServer(80);

void setup()
{
  Serial.begin(115200);

  // DEFAULT CONFIG FOR CONTACT SENSOR
  //EEPROM.begin(1);
  int ContactSensor=EEPROM.read(1);
  if (ContactSensor != 0 && ContactSensor != 1) {
    EEPROM.write(1,0);
    //EEPROM.commit();
  }
  if (ContactSensor==1) {
    pinMode(SENSORPIN, INPUT_PULLUP);
  } else {
    pinMode(SENSORPIN, OUTPUT);
    digitalWrite(SENSORPIN2, HIGH);
  }

  // DEFAULT CONFIG FOR CONTACT SENSOR 2
  int ContactSensor2=EEPROM.read(2);
  if (ContactSensor2 != 0 && ContactSensor2 != 1) {
    EEPROM.write(2,0);
  }
  if (ContactSensor2==1) {
    pinMode(SENSORPIN2, INPUT_PULLUP);
  } else {
    pinMode(SENSORPIN2, OUTPUT);
    digitalWrite(SENSORPIN2, HIGH);
  }

  // DEFAULT CONFIG FOR USE5VRELAY
  //EEPROM.begin(5);
  int eepromUse5Vrelay=EEPROM.read(5);
  if (eepromUse5Vrelay != 0 && eepromUse5Vrelay != 1) {
    EEPROM.write(5,1);
    //EEPROM.commit();
  }
  if (eepromUse5Vrelay ? Use5Vrelay=1 : Use5Vrelay=0);

  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin1, Use5Vrelay==true ? HIGH : LOW);
  digitalWrite(relayPin2, Use5Vrelay==true ? HIGH : LOW);

  uint8_t mac[6] = {0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
  IPAddress myIP(192,168,0,225);
  Ethernet.begin(mac,myIP);
  /* // DHCP
  if (Ethernet.begin(mac) == 0) {
    while (1) {
      Serial.println(F("Failed to configure Ethernet using DHCP"));
      delay(10000);
    }
  }
  */
  server.begin();
  Serial.print(F("localIP: ")); Serial.println(Ethernet.localIP());
  Serial.print(F("subnetMask: ")); Serial.println(Ethernet.subnetMask());
  Serial.print(F("gatewayIP: ")); Serial.println(Ethernet.gatewayIP());
  Serial.print(F("dnsServerIP: ")); Serial.println(Ethernet.dnsServerIP());
  currentIP=Ethernet.localIP()[0]; currentIP+="."; currentIP+= Ethernet.localIP()[1]; currentIP+= ".";
  currentIP+=Ethernet.localIP()[2]; currentIP+= "."; currentIP+=Ethernet.localIP()[3] ;
}

void loop()
{
  // SERIAL MESSAGE
  if (millis()%900000==0) { // every 15 minutes
    Serial.print("UpTime: "); Serial.println(uptime());
  }

  // REBOOT
  //EEPROM.begin(1);
  int days=EEPROM.read(0);
  int RebootFrequencyDays=0;
  RebootFrequencyDays=days;
  if (RebootFrequencyDays > 0 && millis() >= (86400000*RebootFrequencyDays)) { //86400000 per day
    while(true);
  }

  EthernetClient client = server.available();  // try to get client

  String HTTP_req;          // stores the HTTP request
  String request;
  if (client) {  // got client?
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {   // client data available to read
        char c = client.read(); // read 1 byte (character) from client
        HTTP_req += c;  // save the HTTP request 1 char at a time
        // last line of client request is blank and ends with \n
        // respond to client only after last line received
        if (c == '\n') { //&& currentLineIsBlank --- first line only on low memory like UNO/Nano otherwise get it all for AUTH
          request = HTTP_req.substring(0,HTTP_req.indexOf('\r'));
          //auth = HTTP_req.substring(HTTP_req.indexOf('Authorization: Basic '),HTTP_req.indexOf('\r'));
          HTTP_req = "";    // finished with request, empty string
          request.replace("GET ", "");
          request.replace(" HTTP/1.1", "");

          // Match the request
          if (request.indexOf("/favicon.ico") > -1)  {
            return;
          }
          if (request.indexOf("/RebootNow") != -1)  {
            resetFunction();
          }
          if (request.indexOf("RebootFrequencyDays=") != -1)  {
            //EEPROM.begin(1);
            String RebootFrequencyDays=request;
            RebootFrequencyDays.replace("RebootFrequencyDays=",""); RebootFrequencyDays.replace("/",""); RebootFrequencyDays.replace("?","");
            //for (int i = 0 ; i < 512 ; i++) { EEPROM.write(i, 0); } // fully clear EEPROM before overwrite
            EEPROM.write(0,atoi(RebootFrequencyDays.c_str()));
            //EEPROM.commit();
          }
          if (request.indexOf("/ToggleSensor") != -1)  {
            //EEPROM.begin(1);
            if (EEPROM.read(1) == 0) {
              EEPROM.write(1,1);
              //EEPROM.commit();
              pinMode(SENSORPIN, INPUT_PULLUP);
            } else if (EEPROM.read(1) == 1) {
              EEPROM.write(1,0);
              //EEPROM.commit();
              pinMode(SENSORPIN, OUTPUT);
              digitalWrite(SENSORPIN, HIGH);
            }
          }
          if (request.indexOf("/Toggle2ndSensor") != -1)  {
            if (EEPROM.read(2) == 0) {
              EEPROM.write(2,1);
              pinMode(SENSORPIN2, INPUT_PULLUP);
            } else if (EEPROM.read(2) == 1) {
              EEPROM.write(2,0);
            }
          }
          if (request.indexOf("/ToggleUse5Vrelay") != -1)  {
            //EEPROM.begin(5);
            if (EEPROM.read(5) == 0) {
              Use5Vrelay=true;
              EEPROM.write(5,1);
              //EEPROM.commit();
              pinMode(SENSORPIN2, OUTPUT);
              digitalWrite(SENSORPIN2, HIGH);
            } else {
              Use5Vrelay=false;
              EEPROM.write(5,0);
              //EEPROM.commit();
            }
            resetFunction();
          }
          //Serial.print("Use5Vrelay == "); Serial.println(Use5Vrelay);
          if (request.indexOf("RELAY1=ON") != -1 || request.indexOf("MainTriggerOn=") != -1)  {
            digitalWrite(relayPin1, Use5Vrelay==true ? LOW : HIGH);
          }
          if (request.indexOf("RELAY1=OFF") != -1 || request.indexOf("MainTriggerOff=") != -1)  {
            digitalWrite(relayPin1, Use5Vrelay==true ? HIGH : LOW);
          }
          if (request.indexOf("RELAY1=MOMENTARY") != -1 || request.indexOf("MainTrigger=") != -1)  {
            digitalWrite(relayPin1, Use5Vrelay==true ? LOW : HIGH);
            delay(300);
            digitalWrite(relayPin1, Use5Vrelay==true ? HIGH : LOW);
          }

          if (request.indexOf("RELAY2=ON") != -1 || request.indexOf("CustomTriggerOn=") != -1)  {
            digitalWrite(relayPin2, Use5Vrelay==true ? LOW : HIGH);
          }
          if (request.indexOf("RELAY2=OFF") != -1 || request.indexOf("CustomTriggerOff=") != -1)  {
            digitalWrite(relayPin2, Use5Vrelay==true ? HIGH : LOW);
          }
          if (request.indexOf("RELAY2=MOMENTARY") != -1 || request.indexOf("CustomTrigger=") != -1)  {
            digitalWrite(relayPin2, Use5Vrelay==true ? LOW : HIGH);
            delay(300);
            digitalWrite(relayPin2, Use5Vrelay==true ? HIGH : LOW);
          }

          // Return the response
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println(F("\n")); //  do not forget this one
          client.println(F("<!DOCTYPE HTML>"));
          client.println(F("<html><head><title>Arduino & ENC28J60 Dual Switch</title></head><meta name=viewport content='width=500'>\n<style type='text/css'>\nbutton {line-height: 1.8em; margin: 5px; padding: 3px 7px;}"));
          client.println(F("\nbody {text-align:center;}\ndiv {border:solid 1px; margin: 3px; width:150px;}\n.center { margin: auto; width: 400px; border: 3px solid #73AD21; padding: 3px;}"));
          client.println(F("\nhr {width:400px;}\n</style></head>\n<h3 style=\"height: 15px; margin-top: 0px;\"><a href='/'>ARDUINO & ENC28J60 DUAL SWITCH</h3><h3 style=\"height: 15px;\">"));
          client.println(currentIP);
          client.println(F("</a>\n</h3>\n"));

          client.println(F("<i>Current Request:</i><br><b>"));
          client.println(request);
          client.println(F("</b><hr>"));

          client.println(F("<pre>"));
          // SHOW Use5Vrelay
          client.print(F("Use5Vrelay=")); client.print(Use5Vrelay ? F("true") : F("false") ); client.print(F("\n"));
          // SHOW CONTACT SENSOR
          if (EEPROM.read(1)==1) {
            client.print(F("<b><i>Contact Sensor Enabled:</i></b>\n"));
            client.print(F("Contact Sensor=")); client.print(digitalRead(SENSORPIN) ? F("Open") : F("Closed") ); client.print(F("\n"));
          } else {
            client.print(F("<b><i>Contact Sensor Disabled:</i></b>\n"));
            client.print(F("Contact Sensor=Closed\n"));
          }
          // SHOW CONTACT SENSOR
          if (EEPROM.read(2)==1) {
            client.print(F("<b><i>Contact Sensor 2 Enabled:</i></b>\n"));
            client.print(F("Contact Sensor 2=")); client.print(digitalRead(SENSORPIN2) ? F("Open") : F("Closed") ); client.print(F("\n"));
          } else {
            client.print(F("<b><i>Contact Sensor 2 Disabled:</i></b>\n"));
            client.print(F("Contact Sensor 2=Closed\n"));
          }
          client.print(F("UpTime=")); client.println(uptime());
          client.println(freeRam());
          client.println(F("</pre>")); client.println(F("<hr>\n"));

          client.println(F("<div class='center'>\n"));
          client.print(F("RELAY1 pin is now: "));
          if(Use5Vrelay==true) {
            if(digitalRead(relayPin1) == LOW) { client.print(F("On")); } else { client.print(F("Off")); }
          } else {
            if(digitalRead(relayPin1) == HIGH) { client.print(F("On")); } else { client.print(F("Off")); }
          }
          client.println(F("\n<br><a href=\"/RELAY1=ON\"><button onClick=\"parent.location='/RELAY1=ON'\">Turn On</button></a>\n"));
          client.println(F("<a href=\"/RELAY1=OFF\"><button onClick=\"parent.location='/RELAY1=OFF'\">Turn Off</button></a>\n"));
          client.println(F("<a href=\"/RELAY1=MOMENTARY\"><button onClick=\"parent.location='/RELAY1=MOMENTARY'\">MOMENTARY</button></a><br/></div><hr>\n"));

          client.println(F("<div class='center'>\n"));
          client.print(F("RELAY2 pin is now: "));
          if(Use5Vrelay==true) {
            if(digitalRead(relayPin2) == LOW) { client.print(F("On")); } else { client.print(F("Off")); }
          } else {
            if(digitalRead(relayPin2) == HIGH) { client.print(F("On")); } else { client.print(F("Off")); }
          }
          client.println(F("\n<br><a href=\"/RELAY2=ON\"><button onClick=\"parent.location='/RELAY2=ON'\">Turn On</button></a>\n"));
          client.println(F("<a href=\"/RELAY2=OFF\"><button onClick=\"parent.location='/RELAY2=OFF'\">Turn Off</button></a>\n"));
          client.println(F("<a href=\"/RELAY2=MOMENTARY\"><button onClick=\"parent.location='/RELAY2=MOMENTARY'\">MOMENTARY</button></a><br/></div><hr>\n"));


          client.println(F("<div class='center'>"));
          // SHOW TOGGLE Use5Vrelay
          client.println(F("<button onClick=\"javascript: if (confirm(\'Are you sure you want to toggle the Use5Vrelay flag?\\nTrue/1 sends a GND signal. False/0 sends a VCC with 3.3 volts.\\nThis will also reboot the device!!!\\nIf the device does not come back up, reset it manually.\')) parent.location='/ToggleUse5Vrelay';\">Toggle Use 5V Relay</button><br><hr>\n"));
          // SHOW TOGGLE CONTACT SENSORS
          client.println(F("<button onClick=\"javascript: if (confirm(\'Are you sure you want to toggle the Contact Sensor?\')) parent.location='/ToggleSensor';\">Toggle Contact Sensor</button>&nbsp;&nbsp;&nbsp;\n"));
          client.println(F("<button onClick=\"javascript: if (confirm(\'Are you sure you want to toggle the 2nd Contact Sensor?\')) parent.location='/Toggle2ndSensor';\">Toggle Contact Sensor 2</button><br><hr>\n"));


          client.println(F("<input id=\"RebootFrequencyDays\" type=\"text\" name=\"RebootFrequencyDays\" value=\""));
          //EEPROM.begin(1);
          int days=EEPROM.read(0);
          client.println(days);
          client.println(F("\" maxlength=\"3\" size=\"2\" min=\"0\" max=\"255\">&nbsp;&nbsp;&nbsp;<button style=\"line-height: 1em; margin: 3px; padding: 3px 3px;\" onClick=\"parent.location='/RebootFrequencyDays='+document.getElementById('RebootFrequencyDays').value;\">SAVE</button><br>Days between reboots.<br>0 to disable & 255 days is max."));
          client.println(F("<br><button onClick=\"javascript: if (confirm(\'Are you sure you want to reboot?\')) parent.location='/RebootNow';\">Reboot Now</button><br></div><hr>\n"));

          client.println(F("<div class='center'><a target='_blank' href='https://community.smartthings.com/t/raspberry-pi-to-php-to-gpio-to-relay-to-gate-garage-trigger/43335'>Project on SmartThings Community</a></br>\n"));
          client.println(F("<a target='_blank' href='https://github.com/JZ-SmartThings/SmartThings/tree/master/Devices/Generic%20HTTP%20Device'>Project on GitHub</a></br></div></html>\n"));

          break;
        }
        // every line of text received from the client ends with \r\n
        if (c == '\n') {
          // last character on line of received text
          // starting new line with next character read
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // a text character was received from client
          currentLineIsBlank = false;
        }
      } // end if (client.available())
    } // end while (client.connected())
    delay(1);      // give the web browser time to receive the data
    client.stop(); // close the connection
  } // end if (client)
}

String freeRam () {
  #if defined(ARDUINO_ARCH_AVR)
    extern int __heap_start, *__brkval;
    int v;
    return "Free Mem="+String((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval))+"B of 2048B";
  #elif defined(ESP8266)
    return "Free Mem="+String(ESP.getFreeHeap()/1024)+"KB of 80KB";
  #endif
}

String uptime() {
  float d,hr,m,s;
  String dstr,hrstr, mstr, sstr;
  unsigned long over;
  d=int(millis()/(3600000*24));
  dstr=String(d,0);
  dstr.replace(" ", "");
  over=millis()%(3600000*24);
  hr=int(over/3600000);
  hrstr=String(hr,0);
  if (hr<10) {hrstr=hrstr="0"+hrstr;}
  hrstr.replace(" ", "");
  over=over%3600000;
  m=int(over/60000);
  mstr=String(m,0);
  if (m<10) {mstr=mstr="0"+mstr;}
  mstr.replace(" ", "");
  over=over%60000;
  s=int(over/1000);
  sstr=String(s,0);
  if (s<10) {sstr="0"+sstr;}
  sstr.replace(" ", "");
  if (dstr=="0") {
    return hrstr + ":" + mstr + ":" + sstr;
  } else if (dstr=="1") {
    return dstr + " Day " + hrstr + ":" + mstr + ":" + sstr;
  } else {
    return dstr + " Days " + hrstr + ":" + mstr + ":" + sstr;
  }
}
