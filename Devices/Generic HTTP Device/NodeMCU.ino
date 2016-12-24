/**
 *  Arduino / ESP8266-12E / NodeMCU Sample v1.0.20161223
 *  Source code can be found here: https://github.com/JZ-SmartThings/SmartThings/blob/master/Devices/Generic%20HTTP%20Device
 *  Copyright 2016 JZ
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 */

#include <ESP8266WiFi.h>
#include <DHT.h>

const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

const bool use5Vrelay = false;

int relayPin1 = D1; // GPIO5 = D1
int relayPin2 = D2; // GPIO4 = D2

#define DHTPIN D3     // what pin we're connected to  // GPIO0 = D2

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

WiFiServer server(80);

void(* resetFunction) (void) = 0;

void setup() {
  Serial.begin(115200);
  delay(10);

  dht.begin();

  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin1, use5Vrelay==true ? HIGH : LOW);
  digitalWrite(relayPin2, use5Vrelay==true ? HIGH : LOW);

  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  //RESET EVERY 8 HOURS
  if (millis () >= 28800000) {
    resetFunction();
  }

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  //Serial.println("---FULL REQUEST---");
  //Serial.println(client.readString());
  //Serial.println("---END OF FULL REQUEST---");

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  String fullrequest = client.readString();
  Serial.println(request);
  client.flush();

/*
  // BASIC AUTHENTICATION
  // The below Base64 string is gate:gate1 for the username:password
  if (fullrequest.indexOf("Authorization: Basic Z2F0ZTpnYXRlMQ==") == -1)  {
    client.println("HTTP/1.1 401 Access Denied");
    client.println("WWW-Authenticate: Basic realm=\"ESP8266\"");
      //client.println("Content-Type: text/html");
      //client.println(""); //  do not forget this one
      //client.println("Failed : Authentication Required!");
      //Serial.println(fullrequest);
    return;
  }
*/

  // Match the request
  if (request.indexOf("/RebootNow") != -1)  {
    resetFunction();
  }

  Serial.print("use5Vrelay == "); Serial.println(use5Vrelay);
  if (request.indexOf("RELAY1=ON") != -1 || request.indexOf("MainTriggerOn=") != -1)  {
    digitalWrite(relayPin1, use5Vrelay==true ? LOW : HIGH);
  }
  if (request.indexOf("RELAY1=OFF") != -1 || request.indexOf("MainTriggerOff=") != -1)  {
    digitalWrite(relayPin1, use5Vrelay==true ? HIGH : LOW);
  }
  if (request.indexOf("RELAY1=MOMENTARY") != -1 || request.indexOf("MainTrigger=") != -1)  {
    digitalWrite(relayPin1, use5Vrelay==true ? LOW : HIGH);
    delay(300);
    digitalWrite(relayPin1, use5Vrelay==true ? HIGH : LOW);
  }

  if (request.indexOf("RELAY2=ON") != -1 || request.indexOf("CustomTriggerOn=") != -1)  {
    digitalWrite(relayPin2, use5Vrelay==true ? LOW : HIGH);
  }
  if (request.indexOf("RELAY2=OFF") != -1 || request.indexOf("CustomTriggerOff=") != -1)  {
    digitalWrite(relayPin2, use5Vrelay==true ? HIGH : LOW);
  }
  if (request.indexOf("RELAY2=MOMENTARY") != -1 || request.indexOf("CustomTrigger=") != -1)  {
    digitalWrite(relayPin2, use5Vrelay==true ? LOW : HIGH);
    delay(300);
    digitalWrite(relayPin2, use5Vrelay==true ? HIGH : LOW);
  }

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html><head><title>ESP8266 Dual 5V Relay</title></head><meta name=viewport content='width=500'><style type='text/css'>button {line-height: 2.2em; margin: 10px;} body {text-align:center;}");
  client.println("div {border:solid 1px; margin: 3px; width:150px;} .center { margin: auto; width: 350px; border: 3px solid #73AD21; padding: 10px;");
  client.println("</style></head><h1><a href='/'>ESP8266 DUAL RELAY</a></h1>");

  String requestIn = request;
  requestIn.replace("GET ", ""); requestIn.replace(" HTTP/1.1", "");
  Serial.println("---WEB PAGE OUTPUT---");
  Serial.println(request);
  Serial.println("---END OF WEB PAGE OUTPUT---");
  client.println("<i>Current Request:</i><br><b>");
  client.println(requestIn);
  client.println("</b><hr>");

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float tc = dht.readTemperature();
  float tf = (tc * 9.0 / 5.0) + 32.0;

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  client.println("<pre>");
  if (isnan(tc) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  } else {
    client.print("Temperature="); client.print(tc,1); client.print((char)176); client.print("C "); client.print(round(tf)); client.print((char)176); client.println("F");
    client.print("Humidity="); client.print(round(h)); client.println("%");
  }
  client.print("UpTime="); client.println(uptime());
  client.println("</pre>"); client.println("<hr>");

  client.print("<div class='center'>RELAY1 pin is now: ");
  if(use5Vrelay==true) {
    if(digitalRead(relayPin1) == LOW) { client.println("On"); } else { client.println("Off"); }
  } else {
    if(digitalRead(relayPin1) == HIGH) { client.println("On"); } else { client.println("Off"); }
  }
  client.println("<br><a href=\"/RELAY1=ON\"><button onClick=\"parent.location='/RELAY1=ON'\">Turn On</button></a>");
  client.println("<a href=\"/RELAY1=OFF\"><button onClick=\"parent.location='/RELAY1=OFF'\">Turn Off</button></a><br/>");
  client.println("<a href=\"/RELAY1=MOMENTARY\"><button onClick=\"parent.location='/RELAY1=MOMENTARY'\">MOMENTARY</button></a><br/></div>");

  client.println("<hr>");
  client.print("<div class='center'>RELAY2 pin is now: ");
  if(use5Vrelay==true) {
    if(digitalRead(relayPin2) == LOW) { client.println("On"); } else { client.println("Off"); }
  } else {
    if(digitalRead(relayPin2) == HIGH) { client.println("On"); } else { client.println("Off"); }
  }
  client.println("<br><a href=\"/RELAY2=ON\"><button onClick=\"parent.location='/RELAY2=ON'\">Turn On</button></a>");
  client.println("<a href=\"/RELAY2=OFF\"><button onClick=\"parent.location='/RELAY2=OFF'\">Turn Off</button></a><br/>");
  client.println("<a href=\"/RELAY2=MOMENTARY\"><button onClick=\"parent.location='/RELAY2=MOMENTARY'\">MOMENTARY</button></a><br/></div>");

  client.println("<hr><div class='center'><a target='_blank' href='https://community.smartthings.com/t/raspberry-pi-to-php-to-gpio-to-relay-to-gate-garage-trigger/43335'>Project on SmartThings Community</a></br>");
  client.println("<a target='_blank' href='https://github.com/JZ-SmartThings/SmartThings/tree/master/Devices/Generic%20HTTP%20Device'>Project on GitHub</a></br></div></html>");

  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
}

String uptime() {
  float d,hr,m,s,ms;
  String dstr,hrstr, mstr, sstr;
  unsigned long over;
  d=int(millis()/3600000*24);
  dstr=String(hr,0);
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
  ms=over%1000;
  if (dstr!="0") {
    return dstr + " Days " + hrstr + ":" + mstr + ":" + sstr;
  } else {
    return hrstr + ":" + mstr + ":" + sstr;
  }
}
