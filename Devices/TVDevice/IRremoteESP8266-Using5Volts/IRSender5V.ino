/**
 *  TV Device Sample v1.0.20170518
 *  Source code can be found here: https://github.com/JZ-SmartThings/SmartThings/blob/master/Devices/TVDevice
 *  Copyright 2016 JZ
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 */

// CHOOSE WHICH LIBRARY TO USE FOR IR
#include <IRremoteESP8266at5V.h>
//#include <IRremoteESP8266.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
 
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

// SELECT IR PIN
IRsend irsend(D2);

MDNSResponder mdns;
ESP8266WebServer server(80);

// OTA AUTODETECT
WiFiServer TelnetServer(8266);

// OTHER VARIABLES
String currentIP;

void setup(void){
  //TURN OFF BUILTIN LEDS
  pinMode(LED_BUILTIN, OUTPUT);     //GPIO16 also D0 also LED_BUILTIN
  pinMode(D4, OUTPUT);              //GPIO2 also D4
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(D4, HIGH);

  irsend.begin();
  //OTA
  TelnetServer.begin();
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  currentIP = WiFi.localIP().toString();

  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  // OTA
  // Port defaults to 8266
  //ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
  //ArduinoOTA.setHostname("myesp8266");
  // No authentication by default
  //ArduinoOTA.setPassword((const char *)"xxxxx");

  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA End");
    Serial.println("Rebooting...");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  server.on("/", handleRoot);
  server.on("/ir", handleIr); 
 
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void){
  ArduinoOTA.handle();
  server.handleClient();
} 


void handleRoot() {
 String htmlContent = "<html><head><title>ESP8266 IR Remote</title></head><body><a href='/'><h1>ESP8266 IR Remote</h1><h2>";
   htmlContent += currentIP;
   htmlContent += "</h2></a><br>\n";
   for (uint8_t i=0; i<server.args(); i++){
      htmlContent += server.argName(i); htmlContent += "="; htmlContent += server.arg(i); htmlContent += "\n<br>";
   }
   htmlContent += "UpTime="; htmlContent += uptime(); htmlContent += "\n<br>";
 
 
 htmlContent += "</br><p><a href=\"ir?tv=on\">TV ON</a></p><p><a href=\"ir?tv=off\">TV OFF</a></p>";
 htmlContent += "<p><a href=\"ir?tv=chanup\">TV CHANNEL UP</a></p><p><a href=\"ir?tv=chandown\">TV CHANNEL DOWN</a></p><p><a href=\"ir?tv=prev\">PREVIOUS</a></p>";
 htmlContent += "<p><a href=\"ir?tv=volup\">TV VOLUME UP</a></p><p><a href=\"ir?tv=voldown\">TV VOLUME DOWN</a></p><p><a href=\"ir?tv=mute\">TV MUTE</a></p><p><a href=\"ir?tv=input\">TV INPUT</a></p>";
 htmlContent += "<p><a href=\"ir?hdmi=1\">HDMI 1</a></p><p><a href=\"ir?hdmi=2\">HDMI 2</a></p>";
 htmlContent += "<p><a href=\"ir?hdmi=3\">HDMI 3</a></p><p><a href=\"ir?hdmi=4\">HDMI 4</a></p></body></html>";
 server.send(200, "text/html", htmlContent);
}

void handleIr(){
  for (uint8_t i=0; i<server.args(); i++){
    if(server.argName(i) == "single") 
    {
        unsigned int  rawData[67] = {9050,4500, 600,600, 550,600, 550,1700, 550,600, 550,600, 550,600, 550,600, 550,600, 550,1700, 600,1700, 550,600, 550,1700, 550,1700, 550,1700, 600,1700, 550,1700, 550,600, 550,600, 550,600, 550,1700, 600,600, 550,600, 550,600, 550,600, 550,1700, 600,1700, 600,1700, 600,600, 550,1700, 550,1700, 600,1700, 600,1700, 600};  // NEC 20DF10EF
        irsend.sendRaw(rawData, 67, 38);
    }
    else if(server.argName(i) == "tv") 
    {
      if (server.arg(i) == "on") {
        unsigned int  rawData[20] = {6000,1200, 1200,1200, 600,600, 1200,600, 600,600, 600,600, 600,600, 1200,1200, 1200,600, 600,30000};  // UNKNOWN A80E7E5E
        irsend.sendRaw(rawData, 20, 38);
        delay(1000);
        //irsend.sendNEC(0x20DF23DC, 38);
        unsigned int  tvon[67] = {9050,4450, 650,550, 650,550, 650,1650, 650,550, 650,550, 650,550, 650,550, 650,550, 650,1650, 650,1650, 650,550, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,550, 650,550, 650,1650, 650,550, 650,550, 650,550, 650,1650, 650,1650, 650,1650, 650,1650, 650,550, 650,1650, 650,1650, 650,1650, 650,550, 650,550, 650};  // NEC 20DF23DC
        irsend.sendRaw(tvon, 67, 38);
      }
      if (server.arg(i) == "off") {
        unsigned int  rawData[20] = {6000,1200, 1200,1200, 600,600, 1200,600, 600,600, 600,600, 600,1200, 1200,1200, 1200,1200, 600,30000};  // UNKNOWN 692D04A0
        irsend.sendRaw(rawData, 20, 38);
        delay(1000);
        //irsend.sendNEC(0x20DFA35C, 38);
        unsigned int  tvoff[67] = {9050,4450, 650,550, 650,550, 650,1650, 650,550, 650,550, 650,550, 650,550, 650,550, 650,1650, 650,1650, 650,550, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,550, 650,1650, 650,550, 650,550, 650,550, 650,1650, 650,1650, 650,550, 650,1650, 650,550, 650,1650, 650,1650, 650,1650, 650,550, 650,550, 650};  // NEC 20DFA35C
        irsend.sendRaw(tvoff, 67, 38);
      }
      if (server.arg(i) == "chanup") {
        unsigned int  chanup[20] = {6000,1200, 1200,1200, 600,600, 600,600, 600,600, 1200,1200, 600,1200, 1200,600, 1200,600, 600,30000};  // UNKNOWN 165412B7
        irsend.sendRaw(chanup, 20, 38);
      }
      if (server.arg(i) == "chandown") {
        unsigned int  chandown[20] = {6000,1200, 1200,1200, 600,600, 600,600, 600,600, 1200,1200, 1200,600, 1200,600, 1200,1200, 600,30000};  // UNKNOWN 5815B090
        irsend.sendRaw(chandown, 20, 38);
      }
      if (server.arg(i) == "prev") {
        unsigned int  prev[20] = {6000,1200, 1200,1200, 600,600, 600,600, 600,600, 1200,1200, 1200,1200, 1200,1200, 600,600, 600,30000};  // UNKNOWN 5BFBFDE9
        irsend.sendRaw(prev, 20, 38);
      }
      if (server.arg(i) == "volup") {
        //irsend.sendNEC(0x20DF40BF, 38);
        unsigned int  volup[67] = {9100,4450, 650,500, 650,500, 650,1650, 650,500, 650,500, 650,500, 650,500, 650,500, 650,1650, 650,1650, 650,500, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,500, 650,1650, 650,500, 650,500, 650,500, 650,500, 650,500, 650,500, 650,1650, 650,500, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650};  // NEC 20DF40BF
        irsend.sendRaw(volup, 67, 38);
      }
      if (server.arg(i) == "voldown") {
        //irsend.sendNEC(0x20DFC03F, 38);
        unsigned int  voldown[67] = {9100,4450, 650,500, 650,500, 650,1650, 650,500, 650,500, 650,500, 650,500, 650,500, 650,1650, 650,1650, 650,500, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,500, 650,500, 650,500, 650,500, 650,500, 650,500, 650,500, 650,500, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650};  // NEC 20DFC03F
        irsend.sendRaw(voldown, 67, 38);

      }
      if (server.arg(i) == "mute") {
        //irsend.sendNEC(0x20DF906F, 38);
        unsigned int  mute[67] = {9050,4500, 650,550, 550,600, 550,1700, 650,550, 600,550, 650,550, 600,550, 600,550, 650,1650, 650,1650, 650,550, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,550, 650,550, 650,1650, 650,550, 650,550, 650,550, 650,550, 650,550, 650,1650, 650,1650, 650,550, 650,1650, 650,1650, 650,1650, 650,1650, 650};  // NEC 20DF906F
        irsend.sendRaw(mute, 67, 38);
      }
      if (server.arg(i) == "input") {
        //irsend.sendNEC(0x20DFD02F, 38);
        unsigned int  input[67] = {9100,4450, 650,500, 650,550, 650,1650, 650,500, 650,500, 650,500, 650,550, 650,550, 650,1650, 650,1650, 650,500, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,1650, 650,500, 650,1650, 650,500, 650,500, 650,550, 650,550, 650,550, 650,550, 650,1650, 650,500, 650,1650, 650,1650, 650,1650, 650,1650, 650};  // NEC 20DFD02F
        irsend.sendRaw(input, 67, 38);
        delay(500);
        irsend.sendRaw(input, 67, 38);
        delay(500);
        //irsend.sendNEC(0x20DF22DD, 38);
        unsigned int  okay[67] = {9050,4450, 700,500, 650,500, 650,1600, 650,550, 650,500, 600,550, 600,550, 600,550, 600,1650, 700,1600, 700,500, 650,1600, 650,1600, 700,1600, 650,1600, 650,1600, 700,500, 650,500, 650,1600, 650,500, 650,500, 650,500, 650,1600, 650,500, 650,1600, 700,1600, 700,500, 650,1600, 650,1600, 650,1650, 650,550, 650,1600, 650};  // NEC 20DF22DD
        irsend.sendRaw(okay, 67, 38);
      }
    }
    else if(server.argName(i) == "hdmi") 
    {
      if (server.arg(i) == "1") {
        //irsend.sendNEC(0x1FE40BF, 38);
        unsigned int  hdmi1[67] = {9024,4512, 564,564, 564,564, 564,564, 564,564, 564,564, 564,564, 564,564, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,564, 564,564, 564,1692, 564,564, 564,564, 564,564, 564,564, 564,564, 564,564, 564,1692, 564,564, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564};  // NEC 1FE40BF
        irsend.sendRaw(hdmi1, 67, 38);
      }
      if (server.arg(i) == "2") {
        //irsend.sendNEC(0x1FE20DF, 38);
        unsigned int  hdmi2[67] = {9024,4512, 564,564, 564,564, 564,564, 564,564, 564,564, 564,564, 564,564, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,564, 564,564, 564,564, 564,1692, 564,564, 564,564, 564,564, 564,564, 564,564, 564,1692, 564,1692, 564,564, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564};  // NEC 1FE20DF
        irsend.sendRaw(hdmi2, 67, 38);
      }
      if (server.arg(i) == "3") {
        //irsend.sendNEC(0x1FEA05F, 38);
        unsigned int  hdmi3[67] = {9024,4512, 564,564, 564,564, 564,564, 564,564, 564,564, 564,564, 564,564, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,564, 564,1692, 564,564, 564,1692, 564,564, 564,564, 564,564, 564,564, 564,564, 564,564, 564,1692, 564,564, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564};  // NEC 1FEA05F
        irsend.sendRaw(hdmi3, 67, 38);
      }
      if (server.arg(i) == "4") {
        //irsend.sendNEC(0x1FE609F, 38);
        unsigned int  hdmi4[67] = {9024,4512, 564,564, 564,564, 564,564, 564,564, 564,564, 564,564, 564,564, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564,564, 564,564, 564,1692, 564,1692, 564,564, 564,564, 564,564, 564,564, 564,564, 564,1692, 564,564, 564,564, 564,1692, 564,1692, 564,1692, 564,1692, 564,1692, 564};  // NEC 1FE609F
        irsend.sendRaw(hdmi4, 67, 38);
      }
    }
  }
  handleRoot();
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
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
