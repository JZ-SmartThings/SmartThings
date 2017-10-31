 /**
 *  ESP8266-12E / NodeMCU / WeMos D1 Mini WiFi & ENC28J60 Sample v1.0.20171030
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
#include <EEPROM.h>
#include <stdlib.h> // Needed for data type conversions like atoi
#ifdef ESP8266 // Needed for system_get_chip_id()
  extern "C" {
    #include "user_interface.h"
  }
#endif

// SET YOUR NETWORK MODE TRUE TO USE WIFI OR FALSE FOR ENC28J60 --- DO NOT COMMENT OUT & USE TRUE/FALSE INSTEAD
#define useWIFI true
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

// WHICH PINS ARE USED FOR TRIGGERS
// IF USING 3.3V RELAY, TRANSISTOR OR MOSFET THEN TOGGLE Use5Vrelay FLAG TO FALSE USING THE HTTP UI
int relayPin1 = D1; // GPIO5 = D1 // Eco Plugs by KAB or WiOn uses D8 or GPIO15 // Sonoff use D6 or GPIO12
int relayPin2 = D2; // GPIO4 = D2 // Eco Plugs D3 or GPIO0 for WiFi LED
bool Use5Vrelay; // Value defaults by reading eeprom in the setup method & configured via HTTP

// PHYSICAL POWER BUTTON & LEDs --- COMMENT OUT INDIVIDUAL ITEMS TO BYPASS THEIR LOGIC
//#define physicalButton D3   // Eco Plugs or WiOn D7 or GPIO13 // Sonoff D3 or GPIO0 // OITTM D3
#define physicalLED D4    // Eco Plugs or WiOn D3 or GPIO2 // Sonoff D7 or GPIO13 // OITTM D7 // NodeMCU Red LED is D0, GPIO16 or LED_BUILTIN and Blue LED D4 or GPIO2
//#define physicalLED2 D5   // OITTM D5

// USE BASIC HTTP AUTH --- COMMENT OUT TO BYPASS
// #define useAuth

// DESIGNATE CONTACT SENSOR PINS --- ENABLED VIA HTTP UI & STORED ON EEPROM NO NEED TO COMMENT OUT TO DISABLE
#define SENSORPIN D4     // what pin is the Contact Sensor on?
#define SENSORPIN2 D0     // what pin is the 2nd Contact Sensor on?

// USE DHT TEMP/HUMIDITY SENSOR DESIGNATE WHICH PIN BELOW & PICK DHTTYPE BELOW ALSO --- COMMENT OUT LINE BELOW TO BYPASS DHT LOGIC
//#define useDHT
#ifdef useDHT
  #define DHTPIN D3     // what pin is the DHT on?
  #include <DHT.h>      // Use library version 1.2.3 as 1.3.0 gives error
  // Uncomment whatever type of temperature sensor you're using!
  #define DHTTYPE DHT11   // DHT 11
  //#define DHTTYPE DHT22   // DHT 22  (AM2302)
  //#define DHTTYPE DHT21   // DHT 21 (AM2301)
  DHT dht(DHTPIN, DHTTYPE);
#endif

// IF USING MQTT MODIFY TOPICS BELOW --- COMMENT OUT THE LINE BELOW TO BYPASS ALL MQTT LOGIC
//#define useMQTT
#ifdef useMQTT
  const char* mqttServer = "192.168.0.200";
  char buf[6];
  const char* mqttClientName = itoa(system_get_chip_id(),buf,16); // MUST BE UNIQUE OR MQTT LOOP OCCURS
  const char* mqttSwitch1Topic = "smartthings/Gate Switch/switch";
  const char* mqttSwitch2Topic = "smartthings/Gate Switch 2/switch";
  const char* mqttContact1Topic = "smartthings/Gate Switch/contact";
  const char* mqttContact2Topic = "smartthings/Gate Switch 2/contact";
  const char* mqttTemperatureTopic = "smartthings/Gate Switch/temperature";
  const char* mqttHumidityTopic = "smartthings/Gate Switch/humidity";
  // MQTT VARIABLES
  long lastReconnectAttempt = 0; // don't send MQTT too often
  const char* lastContact1Payload; const char* lastContact2Payload; // only send MQTT on changes
  float lastTemperaturePayload = 0; float lastHumidityPayload = 0; // only send MQTT on changes
#endif

// OTHER VARIALBES
String currentIP, clientIP, request, fullrequest;
char *clientIPchar;
long lastButtonPush = 0; // physical button millis() for debouncing

// LOAD UP NETWORK LIB & PORT
#include <PubSubClient.h>
#if useWIFI==true
  #include <ESP8266WiFi.h>
  #include <WiFiClient.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
  #include <ESP8266HTTPUpdateServer.h>
  #include <ArduinoOTA.h>
  WiFiServer server(80);
  ESP8266WebServer httpServer(81);
  ESP8266HTTPUpdateServer httpUpdater;
  //MQTT
  #ifdef useMQTT
    WiFiClient espClient;
    PubSubClient mqtt(espClient);
  #endif
#else
  #include <UIPEthernet.h>
  EthernetServer server = EthernetServer(80);
#endif

void setup()
{
  Serial.begin(115200);

  #ifdef useDHT
    dht.begin();
  #endif

  // PHYSICAL POWER BUTTON & LED
  #ifdef physicalButton
    pinMode(physicalButton, INPUT_PULLUP);
  #endif
  #ifdef physicalLED
    pinMode(physicalLED, OUTPUT);
  #endif
  #ifdef physicalLED2
    pinMode(physicalLED2, OUTPUT);
  #endif

  // DEFAULT CONFIG FOR CONTACT SENSOR
  EEPROM.begin(1);
  int ContactSensor = EEPROM.read(1);
  if (ContactSensor != 0 && ContactSensor != 1) {
    EEPROM.write(1, 0);
    EEPROM.commit();
  }
  if (ContactSensor == 1) {
    pinMode(SENSORPIN, INPUT_PULLUP);
  }
  else {
    pinMode(SENSORPIN, OUTPUT);
    digitalWrite(SENSORPIN2, HIGH);
  }

  // DEFAULT CONFIG FOR CONTACT SENSOR 2
  EEPROM.begin(2);
  int ContactSensor2 = EEPROM.read(2);
  if (ContactSensor2 != 0 && ContactSensor2 != 1) {
    EEPROM.write(2, 0);
    EEPROM.commit();
  }
  if (ContactSensor2 == 1) {
    pinMode(SENSORPIN2, INPUT_PULLUP);
  }
  else {
    pinMode(SENSORPIN2, OUTPUT);
    digitalWrite(SENSORPIN2, HIGH);
  }

  // DEFAULT CONFIG FOR USE5VRELAY
  EEPROM.begin(5);
  int eepromUse5Vrelay = EEPROM.read(5);
  if (eepromUse5Vrelay != 0 && eepromUse5Vrelay != 1) {
    EEPROM.write(5, 1);
    EEPROM.commit();
  }
  if (eepromUse5Vrelay ? Use5Vrelay = 1 : Use5Vrelay = 0);

  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin1, Use5Vrelay == true ? HIGH : LOW);
  digitalWrite(relayPin2, Use5Vrelay == true ? HIGH : LOW);

  #if useWIFI==true
    // Connect to WiFi network
    Serial.println(); Serial.print("Connecting to "); Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("\nlocalIP: "); Serial.println(WiFi.localIP());
    Serial.print("subnetMask: "); Serial.println(WiFi.subnetMask());
    Serial.print("gatewayIP: "); Serial.println(WiFi.gatewayIP());
    Serial.print("dnsIP: "); Serial.println(WiFi.dnsIP());
    // Start the server
    server.begin();
    Serial.println("Server started");
    // Print the IP address
    Serial.print("Use this URL to connect: ");
    Serial.print("http://"); Serial.print(WiFi.localIP()); Serial.println("/");
    currentIP = WiFi.localIP().toString();
  
    // OTA WEB PAGE LOGIC IN SETUP
    // MDNS.begin(host);
    httpUpdater.setup(&httpServer);
    httpServer.begin();
    MDNS.addService("http", "tcp", 81);
    Serial.print("HTTPUpdateServer ready! Open following location: http://"); Serial.print(currentIP); Serial.println(":81/update in your browser\n");
  
    // OTA DIRECTLY FROM IDE
    ArduinoOTA.setHostname("OTA-ESP8266-PORT81");
    // No authentication by default
    // ArduinoOTA.setPassword("admin");
    // ArduinoOTA.setPassword((const char *)"admin");
    ArduinoOTA.onStart([]() {});
    ArduinoOTA.onEnd([]() {});
    ArduinoOTA.onError([](ota_error_t error) { ESP.restart(); });
    ArduinoOTA.begin();
  
    //MQTT
    #ifdef useMQTT
      mqtt.setServer(mqttServer, 1883);
      mqtt.setCallback(callback);
      lastReconnectAttempt = 0;
    #endif
  
  #else
    // ENC28J60 ETHERNET
    uint8_t mac[6] = { 0x0A,0x0B,0x0C,0x0D,0x0E,0x0F };
    // FIXED IP ADDRESS
    //IPAddress myIP(192,168,0,226);
    //Ethernet.begin(mac,myIP);
    if (Ethernet.begin(mac) == 0) {
      while (1) {
        Serial.println("Failed to configure Ethernet using DHCP");
        delay(10000);
      }
    }
    server.begin();
    Serial.print("\nlocalIP: "); Serial.println(Ethernet.localIP());
    Serial.print("subnetMask: "); Serial.println(Ethernet.subnetMask());
    Serial.print("gatewayIP: "); Serial.println(Ethernet.gatewayIP());
    Serial.print("dnsServerIP: "); Serial.println(Ethernet.dnsServerIP());
    currentIP = Ethernet.localIP().toString();
  #endif
  Serial.println("Setup finished...");
}

void loop() {
  #ifdef physicalButton
    // PHYSICAL PUSH BUTTON LIMITER WITH MINIMAL DEBOUNCE
    if (millis()-lastButtonPush > 1000 && digitalRead(physicalButton) == LOW){
      delay(50);
      if (digitalRead(physicalButton) == LOW){
        delay(50);
        if (digitalRead(physicalButton) == LOW){
          digitalWrite(relayPin1, digitalRead(relayPin1) ? LOW : HIGH);
          lastButtonPush = millis();
          #ifdef useMQTT
            if (Use5Vrelay == true) {
              mqtt.publish(mqttSwitch1Topic,digitalRead(relayPin1) ? "off" : "on");
            } else {
              mqtt.publish(mqttSwitch1Topic,digitalRead(relayPin1) ? "on" : "off");
            }
          #endif
        }
      }
    }
  #endif
  #ifdef physicalLED
    // PHYSICAL LED MATCHES RELAY 1 STATE
    if (Use5Vrelay == true) {
      digitalWrite(physicalLED, digitalRead(relayPin1) ? HIGH : LOW);
    } else {
      digitalWrite(physicalLED, digitalRead(relayPin1) ? LOW : HIGH);
    }
  #endif
  #ifdef physicalLED2
    // PHYSICAL LED 2 MATCHES RELAY 2 STATE
    if (Use5Vrelay == true) {
      digitalWrite(physicalLED2, digitalRead(relayPin2) ? HIGH : LOW);
    } else {
      digitalWrite(physicalLED2, digitalRead(relayPin2) ? LOW : HIGH);
    }
  #endif

  // OTA
  #if useWIFI==true
    httpServer.handleClient();
    ArduinoOTA.handle();
  #endif

  // SERIAL KEEP ALIVE MESSAGE
  if (millis() % 900000 == 0) { // every 15 minutes
    Serial.print("UpTime: "); Serial.println(uptime());
  }

  // REBOOT FREQUENCY
  EEPROM.begin(1);
  int days = EEPROM.read(0);
  int RebootFrequencyDays = 0;
  RebootFrequencyDays = days;
  if (RebootFrequencyDays > 0 && millis() >= (86400000 * RebootFrequencyDays)) { //86400000 per day
    while (true);
  }

#if useWIFI==true
  #ifdef useMQTT
    mqttInLoop(millis()); // CALL MQTT LOGIC
  #endif

  // Check if HTTP client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.print("New client: ");
  Serial.println(client.remoteIP().toString());
  clientIP = " requested by: " + client.remoteIP().toString();

  while (!client.available()) {
    delay(1);
  }
  //Serial.println("---FULL REQUEST---"); Serial.println(client.readString()); Serial.println("---END OF FULL REQUEST---");

  request = client.readStringUntil('\r');  // Read the first line of the request
  fullrequest = client.readString();
  Serial.println(request);
  request.replace("GET ", ""); request.replace(" HTTP/1.1", ""); request.replace(" HTTP/1.0", "");
  client.flush();
#else // ENC28J60

  EthernetClient client = server.available();  // try to get client
  //Serial.print("client="); Serial.println(client);
  //Serial.print("client.connected="); Serial.println(client.connected());
  //Serial.print("client.read FIRST CHAR=");Serial.println(client.read());
  if (client) {  // got client?
    boolean currentLineIsBlank = true;
    String HTTP_req;          // stores the HTTP request
    while (client.connected()) {
      if (client.available()) {   // client data available to read
        char c = client.read(); // read 1 byte (character) from client
        HTTP_req += c;  // save the HTTP request 1 char at a time
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
        if (c == '\n' && currentLineIsBlank) { //&& currentLineIsBlank --- first line only on low memory like UNO/Nano otherwise get it all for AUTH
          fullrequest = HTTP_req;
          request = HTTP_req.substring(0, HTTP_req.indexOf('\r'));
          //auth = HTTP_req.substring(HTTP_req.indexOf('Authorization: Basic '),HTTP_req.indexOf('\r'));
          HTTP_req = "";    // finished with request, empty string
          request.replace("GET ", ""); request.replace(" HTTP/1.1", ""); request.replace(" HTTP/1.0", "");
#endif
          // HANDLE HTTP REQUEST
          Serial.println(request);
          handleRequest();
          delay(10); // pause to make sure pin status is updated for response below


        if (request.indexOf("/json") != -1) {
          client.println(jsonResponse());
        } else {
          client.println(clientResponse(0));
          client.println(clientResponse(1));
          client.println(clientResponse(2));
          client.println(clientResponse(3));
          client.println(clientResponse(4));
        }

// END THE HTTP REQUEST
#if useWIFI==true
          delay(1);
          Serial.println("Client disonnected");
          Serial.println("");
#else
          break;
        }
        // every line of text received from the client ends with \r\n
        if (c == '\n') {
          // last character on line of received text
          // starting new line with next character read
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // a text character was received from client
          currentLineIsBlank = false;
        }
      } // end if (client.available())
    } // end while (client.connected())
    delay(1);      // give the web browser time to receive the data
    client.stop(); // close the connection
  } // end if (client)
#endif
} //loop

void handleRequest() {
  //Serial.println("Starting handleRequest...");
  // Match the request
  if (request.indexOf("/favicon.ico") > -1) {
    return;
  }
  if (request.indexOf("/RebootNow") != -1) {
    //while(true);
    ESP.restart();
  }
  if (request.indexOf("RebootFrequencyDays=") != -1) {
    EEPROM.begin(1);
    String RebootFrequencyDays = request;
    RebootFrequencyDays.replace("RebootFrequencyDays=", ""); RebootFrequencyDays.replace("/", ""); RebootFrequencyDays.replace("?", "");
    //for (int i = 0 ; i < 512 ; i++) { EEPROM.write(i, 0); } // fully clear EEPROM before overwrite
    EEPROM.write(0, atoi(RebootFrequencyDays.c_str()));
    EEPROM.commit();
  }
  if (request.indexOf("/ToggleSensor") != -1) {
    EEPROM.begin(1);
    if (EEPROM.read(1) == 0) {
      EEPROM.write(1, 1);
      EEPROM.commit();
      pinMode(SENSORPIN, INPUT_PULLUP);
    }
    else if (EEPROM.read(1) == 1) {
      EEPROM.write(1, 0);
      EEPROM.commit();
      pinMode(SENSORPIN, OUTPUT);
      digitalWrite(SENSORPIN, HIGH);
    }
  }
  if (request.indexOf("/Toggle2ndSensor") != -1) {
    EEPROM.begin(2);
    if (EEPROM.read(2) == 0) {
      EEPROM.write(2, 1);
      EEPROM.commit();
      pinMode(SENSORPIN2, INPUT_PULLUP);
    }
    else if (EEPROM.read(2) == 1) {
      EEPROM.write(2, 0);
      EEPROM.commit();
      pinMode(SENSORPIN2, OUTPUT);
      digitalWrite(SENSORPIN2, HIGH);
    }
  }
  if (request.indexOf("/ToggleUse5Vrelay") != -1) {
    EEPROM.begin(5);
    if (EEPROM.read(5) == 0) {
      Use5Vrelay = true;
      EEPROM.write(5, 1);
      EEPROM.commit();
    }
    else {
      Use5Vrelay = false;
      EEPROM.write(5, 0);
      EEPROM.commit();
    }
    ESP.restart();
  }
  //Serial.print("Use5Vrelay == "); Serial.println(Use5Vrelay);
  if (request.indexOf("RELAY1=ON") != -1 || request.indexOf("MainTriggerOn=") != -1) {
    digitalWrite(relayPin1, Use5Vrelay == true ? LOW : HIGH);
    #ifdef useMQTT
      clientIP = "HTTP ON" + clientIP;
      //clientIPchar = strcpy((char*)malloc(clientIP.length()+1), clientIP.c_str());
      char* clientIPchar = const_cast<char*>(clientIP.c_str());
      mqtt.publish(mqttSwitch1Topic,clientIPchar);
      delay(100);
      mqtt.publish(mqttSwitch1Topic,"on");
    #endif
  }
  if (request.indexOf("RELAY1=OFF") != -1 || request.indexOf("MainTriggerOff=") != -1) {
    digitalWrite(relayPin1, Use5Vrelay == true ? HIGH : LOW);
    #ifdef useMQTT
      clientIP = "HTTP OFF" + clientIP;
      //clientIPchar = strcpy((char*)malloc(clientIP.length()+1), clientIP.c_str());
      char* clientIPchar = const_cast<char*>(clientIP.c_str());
      mqtt.publish(mqttSwitch1Topic,clientIPchar);
      delay(100);
      mqtt.publish(mqttSwitch1Topic,"off");
    #endif
  }
  if (request.indexOf("RELAY1=MOMENTARY") != -1 || request.indexOf("MainTrigger=") != -1) {
    digitalWrite(relayPin1, Use5Vrelay == true ? LOW : HIGH);
    delay(400);
    digitalWrite(relayPin1, Use5Vrelay == true ? HIGH : LOW);
    #ifdef useMQTT
      clientIP = "HTTP MOMENTARY" + clientIP;
      //clientIPchar = strcpy((char*)malloc(clientIP.length()+1), clientIP.c_str());
      char* clientIPchar = const_cast<char*>(clientIP.c_str());
      mqtt.publish(mqttSwitch1Topic,clientIPchar);
      delay(100);
      mqtt.publish(mqttSwitch1Topic,"on");
      delay(100);
      mqtt.publish(mqttSwitch1Topic,"off");
    #endif
  }

  if (request.indexOf("RELAY2=ON") != -1 || request.indexOf("CustomTriggerOn=") != -1) {
    digitalWrite(relayPin2, Use5Vrelay == true ? LOW : HIGH);
    #ifdef useMQTT
      clientIP = "HTTP ON" + clientIP;
      clientIPchar = strcpy((char*)malloc(clientIP.length()+1), clientIP.c_str());
      mqtt.publish(mqttSwitch2Topic,clientIPchar);
      delay(100);
      mqtt.publish(mqttSwitch2Topic,"on");
    #endif
  }
  if (request.indexOf("RELAY2=OFF") != -1 || request.indexOf("CustomTriggerOff=") != -1) {
    digitalWrite(relayPin2, Use5Vrelay == true ? HIGH : LOW);
    #ifdef useMQTT
      clientIP = "HTTP OFF" + clientIP;
      clientIPchar = strcpy((char*)malloc(clientIP.length()+1), clientIP.c_str());
      mqtt.publish(mqttSwitch2Topic,clientIPchar);
      delay(100);
      mqtt.publish(mqttSwitch2Topic,"off");
    #endif
  }
  if (request.indexOf("RELAY2=MOMENTARY") != -1 || request.indexOf("CustomTrigger=") != -1) {
    digitalWrite(relayPin2, Use5Vrelay == true ? LOW : HIGH);
    delay(400);
    digitalWrite(relayPin2, Use5Vrelay == true ? HIGH : LOW);
    #ifdef useMQTT
      clientIP = "HTTP MOMENTARY" + clientIP;
      clientIPchar = strcpy((char*)malloc(clientIP.length()+1), clientIP.c_str());
      mqtt.publish(mqttSwitch2Topic,clientIPchar);
      delay(100);
      mqtt.publish(mqttSwitch2Topic,"on");
      delay(100);
      mqtt.publish(mqttSwitch2Topic,"off");
    #endif
  }
}

// HTTP response
String clientResponse(int section) {
  String clientResponse = "";
  if (section == 0) {
    // BASIC AUTHENTICATION
    #ifdef useAuth
      // The below Base64 string is gate:gate1 for the username:password
      if (fullrequest.indexOf("Authorization: Basic Z2F0ZTpnYXRlMQ==") == -1) {
        clientResponse.concat("HTTP/1.1 401 Access Denied\n");
        clientResponse.concat("WWW-Authenticate: Basic realm=\"ESP8266\"\n");
        clientResponse.concat("Content-Type: text/html\n");
        clientResponse.concat("\n"); //  do not forget this one
        clientResponse.concat("Failed : Authentication Required!\n");
        return clientResponse;
      }
    #else
      clientResponse.concat("HTTP/1.1 200 OK\n");
      clientResponse.concat("Content-Type: text/html; charset=ISO-8859-1\n");
      clientResponse.concat("\n"); //  do not forget this one
    #endif
  }
  else if (section == 1) {
    // Return the response
    clientResponse.concat("<!DOCTYPE HTML>\n");
    clientResponse.concat("<html><head><title>ESP8266 & ");
    #if useWIFI==true
      clientResponse.concat("WIFI");
    #else
      clientResponse.concat("ENC28J60");
    #endif
    clientResponse.concat(" DUAL SWITCH</title></head><meta name=viewport content='width=500'>\n<style type='text/css'>\nbutton {line-height: 1.8em; margin: 5px; padding: 3px 7px;}");
    clientResponse.concat("\nbody {text-align:center;}\ndiv {border:solid 1px; margin: 3px; width:150px;}\n.center { margin: auto; width: 400px; border: 3px solid #73AD21; padding: 3px;}");
    clientResponse.concat("\nhr {width:400px;}\n</style><script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js\"></script>\n");
    clientResponse.concat("<script>\n");
    clientResponse.concat("$(document).ready(function(){\n");
    clientResponse.concat("  $('.hide_settings').click(function(){\n");
    clientResponse.concat("    $('.settings').hide(1000);\n");
    clientResponse.concat("    $('.show_settings').show(1000);\n");
    clientResponse.concat("  });\n");
    clientResponse.concat("  $('.show_settings').click(function(){\n");
    clientResponse.concat("    $('.settings').show(1000);\n");
    clientResponse.concat("    $('.show_settings').hide(1000);\n");
    clientResponse.concat("  });\n");
    clientResponse.concat("});\n");
    clientResponse.concat("function show_settings() {\n");
    clientResponse.concat("  if (/MSIE (\\d+\\.\\d+);/.test(navigator.userAgent) || navigator.userAgent.indexOf(\"Trident/\") > -1 ){ \n");
    clientResponse.concat("    document.getElementById('show_settings').style.display = 'none';\n");
    clientResponse.concat("    document.getElementById('settings').style.display = 'block';\n");
    clientResponse.concat("  }\n");
    clientResponse.concat("}\n");
    clientResponse.concat("function hide_settings() {\n");
    clientResponse.concat("  if (/MSIE (\\d+\\.\\d+);/.test(navigator.userAgent) || navigator.userAgent.indexOf(\"Trident/\") > -1 ){ \n");
    clientResponse.concat("    document.getElementById('show_settings').style.display = 'block';\n");
    clientResponse.concat("    document.getElementById('settings').style.display = 'none';\n");
    clientResponse.concat("  }\n");
    clientResponse.concat("}\n");
    clientResponse.concat("</script>\n");
    clientResponse.concat("</head>");
    clientResponse.concat("\n<h2 style=\"height: 15px; margin-top: 0px;\"><a href='/'>ESP8266 & ");
    #if useWIFI==true
      clientResponse.concat("WIFI");
    #else
      clientResponse.concat("ENC28J60");
    #endif
    clientResponse.concat(" DUAL SWITCH</h2><h3 style=\"height: 15px;\">");
    clientResponse.concat(currentIP);
    clientResponse.concat("</a>\n</h3>\n");

    clientResponse.concat("<i>Current Request:</i><br><b>\n");
    clientResponse.concat(request);
    clientResponse.concat("\n</b><hr>");
  }
  else if (section == 2) {
    clientResponse.concat("<pre>\n");
    // SHOW Use5Vrelay
    clientResponse.concat("Use5Vrelay="); clientResponse.concat(Use5Vrelay ? "true" : "false"); clientResponse.concat("\n");
    // SHOW UPTIME & FREERAM
    clientResponse.concat("UpTime="); clientResponse.concat(uptime()); clientResponse.concat("\n");
    clientResponse.concat("Free Mem="); clientResponse.concat(freeRam()); clientResponse.concat("\n");
    // SHOW CONTACT SENSOR
    if (EEPROM.read(1) == 1) {
      clientResponse.concat("<b><i>Contact Sensor Enabled:</i></b>\n");
      clientResponse.concat("Contact Sensor="); clientResponse.concat(digitalRead(SENSORPIN) ? "Open" : "Closed"); clientResponse.concat("\n");
      //mqtt.publish(mqttContact1Topic,digitalRead(SENSORPIN) ? "open" : "closed");
    }
    else {
      clientResponse.concat("<b><i>Contact Sensor Disabled:</i></b>\n");
      clientResponse.concat("Contact Sensor=Closed\n");
    }
    // SHOW CONTACT SENSOR 2
    if (EEPROM.read(2) == 1) {
      clientResponse.concat("<b><i>Contact Sensor 2 Enabled:</i></b>\n");
      clientResponse.concat("Contact Sensor 2="); clientResponse.concat(digitalRead(SENSORPIN2) ? "Open" : "Closed"); clientResponse.concat("\n");
      //mqtt.publish(mqttContact2Topic,digitalRead(SENSORPIN2) ? "open" : "closed");
    }
    else {
      clientResponse.concat("<b><i>Contact Sensor 2 Disabled:</i></b>\n");
      clientResponse.concat("Contact Sensor 2=Closed\n");
    }
    // SHOW & HANDLE DHT
    #ifdef useDHT
      clientResponse.concat("<b><i>DHT");
      clientResponse.concat(DHTTYPE);
      clientResponse.concat(" Sensor Information:</i></b>\n");
      float h = processDHT(0);
      float tc = processDHT(1); float tf = (tc * 9.0 / 5.0) + 32.0;
      if (tc == -1000) {
        clientResponse.concat("<b><i>DHT Temperature Reading Failed</i></b>\n");
      } else {
        clientResponse.concat("Temperature="); clientResponse.concat(String(tc, 1)); clientResponse.concat((char)176); clientResponse.concat("C "); clientResponse.concat(round(tf)); clientResponse.concat((char)176); clientResponse.concat("F\n");
      }
      if (h == -1000) {
        clientResponse.concat("<b><i>DHT Humidity Reading Failed</i></b>\n");
      } else {
        clientResponse.concat("Humidity="); clientResponse.concat(round(h)); clientResponse.concat("%\n");
      }
    #else
      clientResponse.concat("<b><i>DHT Sensor Disabled</i></b>\n");
    #endif
    clientResponse.concat("</pre>\n"); clientResponse.concat("<hr>\n");
  }
  else if (section == 3) {
    clientResponse.concat("<div class='center'>\n");
    clientResponse.concat("RELAY1 pin is now: ");
    if (Use5Vrelay == true) {
      if (digitalRead(relayPin1) == LOW) { clientResponse.concat("On"); }
      else { clientResponse.concat("Off"); }
    }
    else {
      if (digitalRead(relayPin1) == HIGH) { clientResponse.concat("On"); }
      else { clientResponse.concat("Off"); }
    }
    clientResponse.concat("\n<br><a href=\"/RELAY1=ON\"><button onClick=\"parent.location='/RELAY1=ON'\">Turn On</button></a>\n");
    clientResponse.concat("<a href=\"/RELAY1=OFF\"><button onClick=\"parent.location='/RELAY1=OFF'\">Turn Off</button></a>\n");
    clientResponse.concat("<a href=\"/RELAY1=MOMENTARY\"><button onClick=\"parent.location='/RELAY1=MOMENTARY'\">MOMENTARY</button></a><br/></div><hr>\n");

    clientResponse.concat("<div class='center'>\n");
    clientResponse.concat("RELAY2 pin is now: ");
    if (Use5Vrelay == true) {
      if (digitalRead(relayPin2) == LOW) { clientResponse.concat("On"); }
      else { clientResponse.concat("Off"); }
    }
    else {
      if (digitalRead(relayPin2) == HIGH) { clientResponse.concat("On"); }
      else { clientResponse.concat("Off"); }
    }
    clientResponse.concat("\n<br><a href=\"/RELAY2=ON\"><button onClick=\"parent.location='/RELAY2=ON'\">Turn On</button></a>\n");
    clientResponse.concat("<a href=\"/RELAY2=OFF\"><button onClick=\"parent.location='/RELAY2=OFF'\">Turn Off</button></a>\n");
    clientResponse.concat("<a href=\"/RELAY2=MOMENTARY\"><button onClick=\"parent.location='/RELAY2=MOMENTARY'\">MOMENTARY</button></a></div><hr>\n");
  }
  else if (section == 4) {
    //clientResponse.concat("<div class='center'>");
    clientResponse.concat("<div class='center show_settings' id='show_settings'><a href='#' onClick='javascript:show_settings();' class='show_settings'><button class='show_settings'>Show Settings</button></a></div>\n");
    clientResponse.concat("<div class='settings center' id='settings' style='display:none;'><a href='#' onClick='javascript:hide_settings();' class='hide_settings'><button class='hide_settings'>Hide Settings</button></a><br><hr>\n");
    // SHOW TOGGLE Use5Vrelay
    clientResponse.concat("<button onClick=\"javascript: if (confirm(\'Are you sure you want to toggle the Use5Vrelay flag?\\nTrue/1 sends a GND signal. False/0 sends a VCC with 3.3 volts.\\nThis will also reboot the device!!!\\nIf the device does not come back up, reset it manually.\')) parent.location='/ToggleUse5Vrelay';\">Toggle Use 5V Relay</button><br><hr>\n");
    // SHOW TOGGLE CONTACT SENSORS
    clientResponse.concat("<button style='width: 43%' onClick=\"javascript: if (confirm(\'Are you sure you want to toggle the Contact Sensor?\')) parent.location='/ToggleSensor';\">Toggle Contact Sensor</button>&nbsp;&nbsp;&nbsp;\n");
    clientResponse.concat("<button style='width: 43%' onClick=\"javascript: if (confirm(\'Are you sure you want to toggle the 2nd Contact Sensor?\')) parent.location='/Toggle2ndSensor';\">Toggle Contact Sensor 2</button><br><hr>\n");

    // SHOW OTA INFO IF WIFI IS ENABLED
    #if useWIFI==true
      clientResponse.concat("<a href=\"http://"); clientResponse.concat(currentIP);
      clientResponse.concat(":81/update\"><button onClick=\"parent.location='http://"); clientResponse.concat(currentIP);
      clientResponse.concat(":81/update'\">OTA Update</button></a><br><span style=\"font-size:0.8em;\">This is the <a target='_blank' href='http://esp8266.github.io/Arduino/versions/2.0.0/doc/ota_updates/ota_updates.html#web-browser'>Web Browser OTA method</a>,<br>");
      clientResponse.concat("Open Start&rarr;Run&rarr;type in %TEMP% then enter.<br>BIN file will be under one of the build* folders.<br>2nd method of <a target='_blank' href='http://esp8266.github.io/Arduino/versions/2.0.0/doc/ota_updates/ota_updates.html#arduino-ide'>OTA directly via Arduino IDE</a>.</span><hr>\n");
    #endif
    // REBOOT FREQUENCY
    clientResponse.concat("<input id=\"RebootFrequencyDays\" type=\"text\" name=\"RebootFrequencyDays\" value=\"");
    EEPROM.begin(1);
    int days = EEPROM.read(0);
    clientResponse.concat(days);
    clientResponse.concat("\" maxlength=\"3\" size=\"2\" min=\"0\" max=\"255\">&nbsp;&nbsp;&nbsp;<button style=\"line-height: 1em; margin: 3px; padding: 3px 3px;\" onClick=\"parent.location='/RebootFrequencyDays='+document.getElementById('RebootFrequencyDays').value;\">SAVE</button><br>Days between reboots.<br>0 to disable & 255 days is max.");
    clientResponse.concat("<br><button onClick=\"javascript: if (confirm(\'Are you sure you want to reboot?\')) parent.location='/RebootNow';\">Reboot Now</button><br></div>\n");
    // BOTTOM LINKS  
    clientResponse.concat("<hr><div class='center'><a target='_blank' href='https://community.smartthings.com/t/raspberry-pi-to-php-to-gpio-to-relay-to-gate-garage-trigger/43335'>Project on SmartThings Community</a></br>\n");
    clientResponse.concat("<a target='_blank' href='https://github.com/JZ-SmartThings/SmartThings/tree/master/Devices/Generic%20HTTP%20Device'>Project on GitHub</a></br></div></html>\n");
  }
  return clientResponse;
}

String jsonResponse() {
  String jsonResponse = "";
  //jsonResponse.concat("HTTP/1.1 200 OK\n");
  jsonResponse.concat("HTTP/1.x 200 OK\n");
  jsonResponse.concat("Content-Type: application/json; charset=ISO-8859-1\n");
  jsonResponse.concat("\n"); //  do not forget this one
  jsonResponse.concat("{\n");
  jsonResponse.concat("    \"UpTime\": \""); jsonResponse.concat(uptime()); jsonResponse.concat("\"");
  jsonResponse.concat(",\n");
  jsonResponse.concat("    \"Free Mem\": \""); jsonResponse.concat(freeRam()); jsonResponse.concat("\"");
  jsonResponse.concat(",\n");
  if (Use5Vrelay == true) {
    jsonResponse.concat("    \"MainPinStatus\": \"");
    if (digitalRead(relayPin1) == LOW) { jsonResponse.concat("1\""); }
    else { jsonResponse.concat("0\""); }
    jsonResponse.concat(",\n");
  }
  else {
    jsonResponse.concat("    \"MainPinStatus\": \"");
    if (digitalRead(relayPin1) == HIGH) { jsonResponse.concat("1\""); }
    else { jsonResponse.concat("0\""); }
    jsonResponse.concat(",\n");
  }  
  if (Use5Vrelay == true) {
    jsonResponse.concat("    \"CustomPinStatus\": \"");
    if (digitalRead(relayPin2) == LOW) { jsonResponse.concat("1\""); }
    else { jsonResponse.concat("0\""); }
    jsonResponse.concat(",\n");
  }
  else {
    jsonResponse.concat("    \"CustomPinStatus\": \"");
    if (digitalRead(relayPin2) == HIGH) { jsonResponse.concat("1\""); }
    else { jsonResponse.concat("0\""); }
    jsonResponse.concat(",\n");
  }
  if (EEPROM.read(1) == 1) {
    jsonResponse.concat("    \"SensorPinStatus\": \""); jsonResponse.concat(digitalRead(SENSORPIN) ? "1" : "0"); jsonResponse.concat("\"");
    jsonResponse.concat(",\n");
  }
  else {
    jsonResponse.concat("    \"SensorPinStatus\": \"0\"");
    jsonResponse.concat(",\n");
  }
  if (EEPROM.read(1) == 1) {
    jsonResponse.concat("    \"Sensor2PinStatus\": \""); jsonResponse.concat(digitalRead(SENSORPIN) ? "1" : "0"); jsonResponse.concat("\"");
  }
  else {
    jsonResponse.concat("    \"Sensor2PinStatus\": \"0\"");
  }
  #ifdef useDHT
    float h = processDHT(0);
    float tc = processDHT(1); float tf = (tc * 9.0 / 5.0) + 32.0;
    if (tc != -1000) {
      jsonResponse.concat(",\n");
      jsonResponse.concat("    \"Temperature\": \""); jsonResponse.concat(String(tc, 1)); jsonResponse.concat((char)176); jsonResponse.concat("C ");
      jsonResponse.concat(round(tf)); jsonResponse.concat((char)176); jsonResponse.concat("F\"");
    }
    if (h != -1000) {
      jsonResponse.concat(",\n");
      jsonResponse.concat("    \"Humidity\": \""); jsonResponse.concat(round(h)); jsonResponse.concat("%\"");
    }
  #endif
  jsonResponse.concat("\n}");
  //Serial.println(jsonResponse);
  return jsonResponse;
}

// MQTT method called from loop
boolean reconnect() {
  #ifdef useMQTT
    Serial.print("Attempting MQTT connection... ");
    if (mqtt.connect(mqttClientName, mqttSwitch1Topic, 0, false, "LWT disconnected")) {
      Serial.print(mqttClientName);
      Serial.println(" connected");
      // Once connected, publish an announcement...
      mqtt.publish(mqttSwitch1Topic,"connected");
      mqtt.publish(mqttSwitch1Topic,"available");
      mqtt.publish(mqttSwitch2Topic,"connected");
      mqtt.publish(mqttSwitch2Topic,"available");
      mqtt.subscribe(mqttSwitch1Topic);
      mqtt.subscribe(mqttSwitch2Topic);
    } else {
      Serial.print(" connection failed, rc=");
      Serial.println(mqtt.state());
    }
    return mqtt.connected();
  #endif
}

// MQTT received messages
void callback(char* topic, byte* payload, unsigned int length) {
  #ifdef useMQTT
    Serial.print("Message arrived ["); Serial.print(topic); Serial.print("] ");
    String fullPayload;
    for (int i=0;i<length;i++) {
      Serial.print((char)payload[i]);
      fullPayload += (char)payload[i];
    }
    Serial.println();
    //MQTT handling
    if (String(topic)==mqttSwitch1Topic && fullPayload=="on") {
      digitalWrite(relayPin1, Use5Vrelay == true ? LOW : HIGH);
    }
    if (String(topic)==mqttSwitch1Topic && fullPayload=="off") {
      digitalWrite(relayPin1, Use5Vrelay == true ? HIGH : LOW);
    }
    if (String(topic)==mqttSwitch1Topic && fullPayload=="momentary") {
      digitalWrite(relayPin1, Use5Vrelay == true ? LOW : HIGH);
      delay(300);
      mqtt.publish(mqttSwitch1Topic,"on");
      delay(50);
      mqtt.publish(mqttSwitch1Topic,"off");
      digitalWrite(relayPin1, Use5Vrelay == true ? HIGH : LOW);
    }
    if (String(topic)==mqttSwitch2Topic && fullPayload=="on") {
      digitalWrite(relayPin2, Use5Vrelay == true ? LOW : HIGH);
    }
    if (String(topic)==mqttSwitch2Topic && fullPayload=="off") {
      digitalWrite(relayPin2, Use5Vrelay == true ? HIGH : LOW);
    }
    if (String(topic)==mqttSwitch2Topic && fullPayload=="momentary") {
      digitalWrite(relayPin2, Use5Vrelay == true ? LOW : HIGH);
      delay(300);
      mqtt.publish(mqttSwitch2Topic,"on");
      delay(50);
      mqtt.publish(mqttSwitch2Topic,"off");
      digitalWrite(relayPin2, Use5Vrelay == true ? HIGH : LOW);
    }
  #endif
}

void mqttInLoop(unsigned long ) {
  #ifdef useMQTT // MQTT LOGIC IN LOOP
    unsigned long currentMillis = millis();
    if (!mqtt.connected()) {
      if (currentMillis - lastReconnectAttempt > 15000) { // RECONNECT ATTEMPT EVERY 15 SECONDS
        lastReconnectAttempt = currentMillis;
        // Attempt to reconnect
        if (reconnect()) {
          lastReconnectAttempt = 0;
        }
      }
    } else {
      // Client connected
      mqtt.loop();
    }
    // MQTT publish contact, temperature & humidity
    const char* currentPayload;
    if (currentMillis % 1000 == 0) { // check contact sensors every 1 seconds
      // CONTACT SENSOR 1
      if (EEPROM.read(1) == 1 && lastContact1Payload!=(digitalRead(SENSORPIN) ? "open" : "closed")) {
        mqtt.publish(mqttContact1Topic,digitalRead(SENSORPIN) ? "open" : "closed");
        lastContact1Payload = digitalRead(SENSORPIN) ? "open" : "closed";
      }
      // CONTACT SENSOR 2
      if (EEPROM.read(2) == 1 && lastContact2Payload!=(digitalRead(SENSORPIN2) ? "open" : "closed")) {
        mqtt.publish(mqttContact2Topic,digitalRead(SENSORPIN2) ? "open" : "closed");
        lastContact2Payload = digitalRead(SENSORPIN2) ? "open" : "closed";
      }
    }
    if (currentMillis % 20000 == 0) { // every 20 seconds DHT
      #ifdef useDHT
        float h = processDHT(0);
        float tc = processDHT(1); float tf = (tc * 9.0 / 5.0) + 32.0;
        char buf[8];
        //Serial.println (round(lastTemperaturePayload)); Serial.println (tf); Serial.println (round(tf));
        if ((tc != -1000 && round(lastTemperaturePayload)!=round(tf)) || (tc != -1000 && currentMillis % 60000 == 0)) {
          Serial.println ("mqttTemperature published");
          mqtt.publish(mqttTemperatureTopic,dtostrf(round(tf), 0, 0, buf));
          lastTemperaturePayload=round(tf);
        }
        //Serial.println (round(lastHumidityPayload)); Serial.println (h); Serial.println (round(h));
        if ((h != -1000 && round(lastHumidityPayload)!=round(h)) || (h != -1000 && currentMillis % 60000 == 0)) {
          Serial.println ("mqttHumidity published");
          mqtt.publish(mqttHumidityTopic,dtostrf(round(h), 0, 0, buf));
          lastHumidityPayload=round(h);
        }
      #endif // DHT
    }
    if (currentMillis % 900000 == 0) { // check connection every 15 minutes
      // MQTT KEEPALIVE AND LWT LOGIC
      mqtt.publish(mqttSwitch1Topic,"available");
      mqtt.publish(mqttSwitch2Topic,"available");
    }
  #endif // MQTT
}

float processDHT(bool whichSensor) {
  // Reading temperature or humidity takes about 250 milliseconds. Sensor readings may also be up to 2 seconds old
  #ifdef useDHT
    float h = -1000;
    float tc = -1000;
    int counter = 1;
    if (whichSensor == 0) {
      h = dht.readHumidity();
      while (counter <= 5 && (isnan(h) || h == -1000)) {
        if (isnan(h) || h == -1000) { h = dht.readHumidity(); } // re-read
        counter += 1; delay(50);
      }
    }
    else if (whichSensor == 1) {
      tc = dht.readTemperature();
      while (counter <= 5 && (isnan(tc) || tc == -1000)) {
        if (isnan(tc) || tc == -1000) { tc = dht.readTemperature(); } // re-read
        counter += 1; delay(50);
      }
    }
    if (whichSensor == 0) {
      if (isnan(h) || h == -1000) { return -1000; }
        else { return h; }
    } else if (whichSensor == 1) {
      if (isnan(tc) || tc == -1000) { return -1000; }
        else { return tc; }
    }
  #endif
}

String freeRam() {
  #if defined(ARDUINO_ARCH_AVR)
    extern int __heap_start, *__brkval;
    int v;
    return String((int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval)) + "B of 2048B";
  #elif defined(ESP8266)
    return String(ESP.getFreeHeap() / 1024) + "KB of 80KB";
  #endif
}

String uptime() {
  float d, hr, m, s;
  String dstr, hrstr, mstr, sstr;
  unsigned long over;
  d = int(millis() / (3600000 * 24));
  dstr = String(d, 0);
  dstr.replace(" ", "");
  over = millis() % (3600000 * 24);
  hr = int(over / 3600000);
  hrstr = String(hr, 0);
  if (hr<10) { hrstr = hrstr = "0" + hrstr; }
  hrstr.replace(" ", "");
  over = over % 3600000;
  m = int(over / 60000);
  mstr = String(m, 0);
  if (m<10) { mstr = mstr = "0" + mstr; }
  mstr.replace(" ", "");
  over = over % 60000;
  s = int(over / 1000);
  sstr = String(s, 0);
  if (s<10) { sstr = "0" + sstr; }
  sstr.replace(" ", "");
  if (dstr == "0") {
    return hrstr + ":" + mstr + ":" + sstr;
  }
  else if (dstr == "1") {
    return dstr + " Day " + hrstr + ":" + mstr + ":" + sstr;
  }
  else {
    return dstr + " Days " + hrstr + ":" + mstr + ":" + sstr;
  }
}
