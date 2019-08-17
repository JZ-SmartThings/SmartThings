const char* version_number = "1.0.20190816";
 /**
 *  ESP32 Sample
 *  Source code can be found here: https://github.com/JZ-SmartThings/SmartThings/blob/master/Devices/ESP32%20Multi%20Device
 *  Copyright 2019 JZ
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 */

const char* ssid = "SSID";
const char* password = "password";

unsigned long RebootFrequencyDays = 0; // ZERO DISABLES THE AUTO-REBOOT

bool Use5Vrelay = true; // SEND GROUND ON PIN NOT VCC FOR 5V RELAYS - FALSE SENDS VCC
int switch1 = 32;
int switch2 = 33;

int STATUS_LED = 25;

// DESIGNATE CONTACT SENSOR PINS --- COMMENT OUT 2 LINES BELOW TO BYPASS CONTACT SENSORS
#define CONTACTPIN1 19     // what pin is the 1st Contact Sensor on?
#define CONTACTPIN2 23     // what pin is the 2nd Contact Sensor on?

const char* mqttServer = "192.168.0.251";
const char* mqttDeviceName = "ESP32HELTEC";
const char* mqttSwitch1Topic = "smartthings/ESP32HELTEC Stateful/switch";
const char* mqttSwitch2Topic = "smartthings/ESP32HELTEC Stateful 2/switch";
const char* mqttSwitch1StateTopic = "smartthings/ESP32HELTEC/state";
const char* mqttSwitch2StateTopic = "smartthings/ESP32HELTEC 2/state";
const char* mqttSwitch1MomentaryTopic = "smartthings/ESP32HELTEC Button/button";
const char* mqttSwitch2MomentaryTopic = "smartthings/ESP32HELTEC Button 2/button";
const char* mqttContact1Topic = "smartthings/ESP32HELTEC Contact/contact";
const char* mqttContact2Topic = "smartthings/ESP32HELTEC Contact 2/contact";
const char* mqttTemperatureTopic = "smartthings/ESP32HELTEC/temperature";
const char* mqttHumidityTopic = "smartthings/ESP32HELTEC/humidity";

#define useOLED
#ifdef useOLED
  #include <U8g2lib.h>      //--- Full OLED list: https://github.com/olikraus/U8g2_Arduino/blob/master/examples/full_buffer/GraphicsTest/GraphicsTest.ino
  U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);      // Heltec Wifi Kit 32 Clock=15 Data=4 Reset=16 --- Wemos LoLin Clock=4 Data=5 Reset=16
  //U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 4, /* data=*/ 5, /* reset=*/ 16);     // Heltec Wifi Kit 32 Clock=15 Data=4 Reset=16 --- Wemos LoLin Clock=4 Data=5 Reset=16

  int sceneDuration = 10000; unsigned long sceneNextMillis = 0; int sceneNext = 1;
#endif

// USE ESP32 INTERNAL TEMPERATURE SENSOR
//#define useESP32Temp
#ifdef useESP32Temp
  extern "C" // Extern C is used when we are using a funtion written in "C" language in a C++ code.
  {
    uint8_t temprature_sens_read(); // This function is written in C language
  }
  uint8_t temprature_sens_read();
  unsigned long lastESP32Tempread = 0;
#endif

// USE DHT TEMP/HUMIDITY SENSOR DESIGNATE WHICH PIN BELOW & PICK DHTTYPE BELOW AS WELL --- COMMENT OUT THE LINE BELOW TO BYPASS DHT LOGIC
//#define useDHT
#ifdef useDHT
  uint8_t DHTPIN = 13;    // what pin is the DHT on?
  // Uncomment whatever type of temperature sensor you're using!
  //#define DHTTYPE DHT11   // DHT 11
  #define DHTTYPE DHT22   // DHT 22  (AM2302)
  //#define DHTTYPE DHT21   // DHT 21 (AM2301)

  #include <DHT.h>
  DHT dht(DHTPIN, DHTTYPE);
  unsigned long lastDHTread = 0;
#endif

// USE BME280 TEMP/HUMIDITY/PRESSURE SENSOR. PICK YOUR OPTIONS BELOW OR COMMENT OUT THE LINE BELOW TO BYPASS BME280 LOGIC
#define useBME280
#ifdef useBME280
  // DEFAULT BELOW USES I2C, PINS HERE ARE FOR SPI ONLY
  #define BME_SCK 13
  #define BME_MISO 12
  #define BME_MOSI 11
  #define BME_CS 10
  
  #include <Wire.h>
  #include <SPI.h>
  #include <Adafruit_Sensor.h>
  #include <Adafruit_BME280.h>
  Adafruit_BME280 bme; // I2C
  //Adafruit_BME280 bme(BME_CS); // hardware SPI
  //Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

  #define SEALEVELPRESSURE_HPA (1013.25)
  unsigned long lastBME280read = 0;
#endif

float lastTemperaturePayload = -1; float lastHumidityPayload = -1; // only send MQTT on changes

// WIFI, OTA & HTTP
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
WebServer server(80);

const char* updateIndex = "<p style='font-size: 150%;'>___mqttDeviceName___</p><form method='POST' action='/updatepost' enctype='multipart/form-data'><input type='file' name='update' style='font-size: 150%;'><br><br><input type='submit' value='Update' style='font-size: 150%;'></form>";
const char* updateDone = "<html><head><meta http-equiv=\"REFRESH\" content=\"10;URL=/\"></head><marquee direction=\"right\"><h1>Update went OK!</h1><h1>Rebooting...</h1></marquee></html>";
const char* rebootIndex = "<button style='font-size: 150%;' onClick=\"javascript: if (confirm(\'Are you sure you want to reboot?\')) parent.location='/rebootnow';\">Reboot</button>";
const char* rebootNow = "<html><head><meta http-equiv=\"REFRESH\" content=\"10;URL=/\"></head><marquee direction=\"right\"><h1>Rebooting...</h1></marquee></html>";

#ifdef CONTACTPIN1
  const char* lastContact1Payload;
#endif
#ifdef CONTACTPIN2
  const char* lastContact2Payload;
#endif

WiFiClient espClient;
PubSubClient client(espClient);
int mqtt_reconnect_count = 0;


// ---------------------------------------------------------------------------------------------------------- SET UP WIFI AND OTHER WIFI DEPENDENT ITEMS
void setup_wifi(bool reset_wifi = false) {
  delay(50);
  // STATUS LED ON WHILE ACQUIRING WIFI
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);   // Turn the LED on (Note that LOW is the voltage level
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  //WiFi.mode(WIFI_AP_STA);
  if (reset_wifi==true) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    #ifdef useOLED
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_pxplusibmcgathin_8f);  // choose a suitable font
      u8g2.drawStr(0,12,"Trying SSID:");
      u8g2.drawStr(0,24,ssid);
      u8g2.drawStr(0,48,"for...");
      char buf[16];
      dtostrf(counter/2,0,0,buf);
      strcat(buf,(counter/2) == 1 ? " second" : " seconds");
      u8g2.drawStr(0,60,buf);
      u8g2.sendBuffer();          // transfer internal memory to the display
    #endif
    counter++;
    if ( counter >= 120 ) { ESP.restart(); }
  }
  #ifdef useOLED
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);  // choose a suitable font
    u8g2.drawStr(0,12,"Success with SSID:");
    u8g2.drawStr(0,24,ssid);
    u8g2.drawStr(0,48,"Time to connect:");
    char buf[20];
    dtostrf(counter/2,0,0,buf);
    strcat(buf,(counter/2) == 1 ? " second" : " seconds");
    u8g2.drawStr(0,60,buf);
    u8g2.sendBuffer();          // transfer internal memory to the display
    delay(5000);
    //u8g2.clearBuffer();
  #endif
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // STATUS LED OFF
  digitalWrite(STATUS_LED, LOW);  // Turn the LED off by making the voltage HIGH

  //OTA & JSON
  if (WiFi.status() == WL_CONNECTED) {
    String mqttDeviceNameSTRING = String(mqttDeviceName);
    mqttDeviceNameSTRING.replace(" ","_");
    MDNS.begin(mqttDeviceNameSTRING.c_str());
    server.on("/", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.sendHeader("charset", "ISO-8859-1");
      String jsonData = "{";
      jsonData.concat("\n\"DeviceName\":\""); jsonData.concat(mqttDeviceName); jsonData.concat("\",");
      jsonData.concat("\n\"DeviceIP\":\""); jsonData.concat(WiFi.localIP().toString()); jsonData.concat("\",");
      jsonData.concat("\n\"Uptime\":\""); jsonData.concat(uptime()); jsonData.concat("\",");
      jsonData.concat("\n\"InfoPage\":\"http://"); jsonData.concat(WiFi.localIP().toString()); jsonData.concat("/info\",");
      if (Use5Vrelay == true ) {
        jsonData.concat("\n\"Switch1\":\""); jsonData.concat(digitalRead(switch1) ? "off" : "on"); jsonData.concat("\",");
        jsonData.concat("\n\"Switch2\":\""); jsonData.concat(digitalRead(switch2) ? "off" : "on"); jsonData.concat("\"");
      } else {        
        jsonData.concat("\n\"Switch1\":\""); jsonData.concat(digitalRead(switch1) ? "on" : "off"); jsonData.concat("\",");
        jsonData.concat("\n\"Switch2\":\""); jsonData.concat(digitalRead(switch2) ? "on" : "off"); jsonData.concat("\"");
      }
      #ifdef CONTACTPIN1
        jsonData.concat(",");
        jsonData.concat("\n\"Contact1\":\""); jsonData.concat(digitalRead(CONTACTPIN1) ? "open" : "closed"); jsonData.concat("\"");
      #endif
      #ifdef CONTACTPIN2
        jsonData.concat(",");
        jsonData.concat("\n\"Contact2\":\""); jsonData.concat(digitalRead(CONTACTPIN2) ? "open" : "closed"); jsonData.concat("\"");
      #endif
      #ifdef useDHT
        jsonData.concat(",");
        jsonData.concat("\n\"Temperature\":\""); jsonData.concat(int((probeDHT(1) * 9.0 / 5.0) + 32.0)); jsonData.concat("\",");
        jsonData.concat("\n\"Humidity\":\""); jsonData.concat(int(probeDHT(0))); jsonData.concat("\"");
      #endif // DHT
      #ifdef useBME280
        jsonData.concat(",");
        jsonData.concat("\n\"Temperature\":\""); jsonData.concat(int((bme.readTemperature() * 9.0 / 5.0) + 32.0)); jsonData.concat("\",");
        jsonData.concat("\n\"Humidity\":\""); jsonData.concat(int(bme.readHumidity())); jsonData.concat("\"");
      #endif // BME280
      jsonData.concat("\n}");
      server.send(200, "text/json", (char*) jsonData.c_str());
    });
    server.on("/reboot", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", rebootIndex);
    });
    server.on("/rebootnow", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", rebootNow);
      delay(1000);
      ESP.restart();
    });
    server.on("/info", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      String infoData = "<html><table border=1 cellpadding=3><tbody>";
      infoData.concat("\r\n<tr><td>Update Page</td><td><a href=\"http://"); infoData.concat(WiFi.localIP().toString()); infoData.concat("/update\">");
      infoData.concat("http://"); infoData.concat(WiFi.localIP().toString()); infoData.concat("/update");infoData.concat("</a> OR <a href=\"http://");

      String mqttDeviceNameSTRING = String(mqttDeviceName);
      mqttDeviceNameSTRING.replace(" ","_");
      
      infoData.concat(mqttDeviceNameSTRING); infoData.concat("/update\">");
      infoData.concat("http://"); infoData.concat(mqttDeviceNameSTRING); infoData.concat("/update");infoData.concat("</a></td></tr>");
      infoData.concat("\r\n<tr><td>Switch 1</td><td>on pin "); infoData.concat(switch1); infoData.concat("</td></tr>");
      infoData.concat("\r\n<tr><td>Switch 2</td><td>on pin "); infoData.concat(switch2); infoData.concat("</td></tr>");
      #ifdef CONTACTPIN1
        infoData.concat("\r\n<tr><td>Contact Sensor 1</td><td>Enabled on pin "); infoData.concat(CONTACTPIN1); infoData.concat("</td></tr>");
      #endif
      #ifdef CONTACTPIN2
        infoData.concat("\r\n<tr><td>Contact Sensor 2</td><td>Enabled on pin "); infoData.concat(CONTACTPIN2); infoData.concat("</td></tr>");
      #endif
      #ifdef useOLED
        infoData.concat("\r\n<tr><td>OLED Display</td><td>Enabled</td></tr>");
      #endif
      #ifdef useDHT
        infoData.concat("\r\n<tr><td>DHT"); infoData.concat(DHTTYPE); infoData.concat(" Multi-Sensor</td><td>Enabled on pin "); infoData.concat(DHTPIN); infoData.concat("</td></tr>");
      #endif
      #ifdef useBME280
        infoData.concat("\r\n<tr><td>BME280 Multi-Sensor</td><td>Enabled</td></tr>");
      #endif
      #ifdef useESP32Temp
        infoData.concat("\r\n<tr><td>ESP32 Internal Temperature Sensor</td><td>Enabled</td></tr>");
      #endif    
      infoData.concat("\r\n<tr><td>mqttServer");infoData.concat("</td><td>");infoData.concat(mqttServer);infoData.concat("</td></tr>");
      infoData.concat("\r\n<tr><td>mqttDeviceName");infoData.concat("</td><td>");infoData.concat(mqttDeviceName);infoData.concat("</td></tr>");
      infoData.concat("\r\n<tr><td>mqttSwitch1Topic");infoData.concat("</td><td>");infoData.concat(mqttSwitch1Topic);infoData.concat("</td></tr>");
      infoData.concat("\r\n<tr><td>mqttSwitch2Topic");infoData.concat("</td><td>");infoData.concat(mqttSwitch2Topic);infoData.concat("</td></tr>");
      infoData.concat("\r\n<tr><td>mqttSwitch1StateTopic");infoData.concat("</td><td>");infoData.concat(mqttSwitch1StateTopic);infoData.concat("</td></tr>");
      infoData.concat("\r\n<tr><td>mqttSwitch2StateTopic");infoData.concat("</td><td>");infoData.concat(mqttSwitch2StateTopic);infoData.concat("</td></tr>");
      infoData.concat("\r\n<tr><td>mqttSwitch1MomentaryTopic");infoData.concat("</td><td>");infoData.concat(mqttSwitch1MomentaryTopic);infoData.concat("</td></tr>");
      infoData.concat("\r\n<tr><td>mqttSwitch2MomentaryTopic");infoData.concat("</td><td>");infoData.concat(mqttSwitch2MomentaryTopic);infoData.concat("</td></tr>");
      infoData.concat("\r\n<tr><td>mqttContact1Topic");infoData.concat("</td><td>");infoData.concat(mqttContact1Topic);infoData.concat("</td></tr>");
      infoData.concat("\r\n<tr><td>mqttContact2Topic");infoData.concat("</td><td>");infoData.concat(mqttContact2Topic);infoData.concat("</td></tr>");
      infoData.concat("\r\n<tr><td>mqttTemperatureTopic");infoData.concat("</td><td>");infoData.concat(mqttTemperatureTopic);infoData.concat("</td></tr>");
      infoData.concat("\r\n<tr><td>mqttHumidityTopic");infoData.concat("</td><td>");infoData.concat(mqttHumidityTopic);infoData.concat("</td></tr>");
      infoData.concat("\r\n</tbody></table><html>\r\n");
      server.send(200, "text/html", infoData.c_str() );
    });
    server.on("/update", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      String updateIndexSTRING = String(updateIndex);
      updateIndexSTRING.replace("___mqttDeviceName___",String(mqttDeviceName));
      server.send(200, "text/html", updateIndexSTRING);
    });
    server.on("/updatepost", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", (Update.hasError()) ? "FAIL" : updateDone);
      delay(1000);
      ESP.restart();
    }, []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin()) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
    });
    server.begin();
    MDNS.addService("http", "tcp", 80);

    Serial.printf("Ready! Open http://%s.local in your browser\n", mqttDeviceNameSTRING.c_str());
  }
} // ---------------------------------------------------------------------------------------------------------- setup_wifi


void setup() { // ---------------------------------------------------------------------------------------------------------- SETUP
  Serial.begin(115200);

  pinMode(switch1, OUTPUT);
  pinMode(switch2, OUTPUT);
  digitalWrite(switch1, Use5Vrelay == true ? HIGH : LOW);
  digitalWrite(switch2, Use5Vrelay == true ? HIGH : LOW);

  #ifdef CONTACTPIN1
    pinMode(CONTACTPIN1, INPUT_PULLUP);
  #endif
  #ifdef CONTACTPIN2
    pinMode(CONTACTPIN2, INPUT_PULLUP);
  #endif

  #ifdef useOLED
    /* U8g2 Project: SSD1306 Test Board */
    //pinMode(10, OUTPUT);
    //pinMode(9, OUTPUT);
    //digitalWrite(10, 0);
    //digitalWrite(9, 0);

    //pinMode(16, OUTPUT);
    //digitalWrite(16, LOW); // set GPIO16 low to reset OLED
    //delay(50);
    //digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 to high
    //u8x8.begin();
    //u8x8.setPowerSave(0);

    u8g2.begin();
    u8g2.clearBuffer();          // clear the internal memory
  #endif

  setup_wifi(false);
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);

  #ifdef useDHT
    pinMode(DHTPIN, INPUT);
    dht.begin();
    outputDHT(millis());
  #endif

  #ifdef useBME280
    unsigned status;
    status = bme.begin();  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!"); Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n"); Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n"); Serial.print("        ID of 0x61 represents a BME 680.\n");
        //while (1); --disable loop if BME not found
    }
    outputBME280(millis());
  #endif

  #ifdef useESP32Temp
    outputESP32Temp(millis());
  #endif

  #ifdef useOLED
    sceneNextMillis = millis() + sceneDuration;
  #endif

} // ---------------------------------------------------------------------------------------------------------- SETUP



unsigned long loopMillis = 0;

void loop() { //---------------------------------------------------------------------------------------------------------- LOOP

  loopMillis = millis();

  // SERIAL KEEP ALIVE MESSAGE EVERY 15
  if (loopMillis % 900000 == 0) { // every 15 minutes
    Serial.print("--------------------------------------------------UpTime: "); Serial.println(uptime());
  }

  // REBOOT FREQUENCY
  if (RebootFrequencyDays > 0 && loopMillis >= (86400000 * RebootFrequencyDays)) { //86400000 per day
    ESP.restart();
  }

  // WIFI RECONNECT
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi(true);
  }

  if (!client.connected()) { // MQTT RECONNECT
    reconnect();
  }
  client.loop();

  server.handleClient(); // HANDLE HTTP CLIENT

  #ifdef useOLED
    if (loopMillis > sceneNextMillis) {
      OLEDscene(loopMillis, sceneNext);
      sceneNextMillis = loopMillis + sceneDuration;
      if (sceneNext == 1) {
        sceneNext=2;
        sceneNextMillis -= 6000;
      } else if (sceneNext == 2) {
        sceneNext=1;
      }
    } else { // SHOW CURRENT
      OLEDscene(loopMillis, sceneNext);
    }
  #endif

  #ifdef useDHT
    if (loopMillis-lastDHTread > 30000 ) { // DHT every 30 seconds
      lastDHTread += 30000;
      outputDHT(loopMillis);
    }
  #endif // DHT

  #ifdef useBME280
    if (loopMillis-lastBME280read > 30000 ) { // BME280 every 30 seconds
      lastBME280read += 30000;
      outputBME280(loopMillis);
    }
  #endif // BME280

  #ifdef useESP32Temp
    if (loopMillis-lastESP32Tempread > 30000 ) { // ESP32 Temp every 30 seconds
      lastESP32Tempread += 30000;
      outputESP32Temp(loopMillis);
    }
    outputESP32Temp(millis());
  #endif
  

  #ifdef CONTACTPIN1
    // CONTACT SENSOR 1
    bool currentContact1Read = digitalRead(CONTACTPIN1);
    if (lastContact1Payload!=(currentContact1Read ? "open" : "closed")) {
      client.publish(mqttContact1Topic,currentContact1Read ? "open" : "closed");
      lastContact1Payload = currentContact1Read ? "open" : "closed";

      //SmartThings contact sensors are READ ONLY - switch logic is the workaround.
      String mqttContact1TopicSTRING = String(mqttContact1Topic);
      mqttContact1TopicSTRING.replace("/contact","/switch");
      client.publish(mqttContact1TopicSTRING.c_str(),currentContact1Read ? "on" : "off");

      Serial.print ("mqttSensor1 published: "); Serial.println (lastContact1Payload);
    }
  #endif
  #ifdef CONTACTPIN2
    // CONTACT SENSOR 2
    bool currentContact2Read = digitalRead(CONTACTPIN2);
    if (lastContact2Payload!=(currentContact2Read ? "open" : "closed")) {
      client.publish(mqttContact2Topic,currentContact2Read ? "open" : "closed");
      lastContact2Payload = currentContact2Read ? "open" : "closed";

      //SmartThings contact sensors are READ ONLY - switch logic is the workaround.
      String mqttContact2TopicSTRING = String(mqttContact2Topic);
      mqttContact2TopicSTRING.replace("/contact","/switch");
      client.publish(mqttContact2TopicSTRING.c_str(),currentContact2Read ? "on" : "off");

      Serial.print ("mqttSensor2 published: "); Serial.println (lastContact2Payload);
    }
  #endif
} // ---------------------------------------------------------------------------------------------------------- LOOP



void callback(char* topic, byte* payload, unsigned int length) { // MQTT CALLBACK
  Serial.print("Message arrived ["); Serial.print(topic); Serial.print("] ");

  String fullPayload="";
  for (int i=0;i<length;i++) {
    fullPayload += (char)payload[i];
  }
  Serial.println(fullPayload);

  if ((String(topic)==mqttSwitch1Topic && fullPayload=="on") || (String(topic)==mqttSwitch1Topic && fullPayload=="off") \
        || (String(topic)==mqttSwitch1MomentaryTopic && fullPayload=="pushed") ) {
      Serial.println("Starting Switch 1 MQTT Callback...");
      if (String(topic)==mqttSwitch1Topic && fullPayload=="on") {
        digitalWrite(switch1, Use5Vrelay == true ? LOW : HIGH);
        //client.publish(mqttSwitch1Topic,"", true);
      }
      if (String(topic)==mqttSwitch1Topic && fullPayload=="off") {
        digitalWrite(switch1, Use5Vrelay == true ? HIGH : LOW);
        //client.publish(mqttSwitch1Topic,"", true);
      }
      if (String(topic)==mqttSwitch1MomentaryTopic && fullPayload=="pushed") {
        digitalWrite(switch1, Use5Vrelay == true ? LOW : HIGH);
        delay(300);
        digitalWrite(switch1, Use5Vrelay == true ? HIGH : LOW);
        client.publish(mqttSwitch1MomentaryTopic,"momentary_done", true);
        client.publish(mqttSwitch1MomentaryTopic,"", true);
      }
    }

  if ((String(topic)==mqttSwitch2Topic && fullPayload=="on") || (String(topic)==mqttSwitch2Topic && fullPayload=="off") \
        || (String(topic)==mqttSwitch2MomentaryTopic && fullPayload=="pushed") ) {
      Serial.println("Starting Switch 2 MQTT Callback...");
      if (String(topic)==mqttSwitch2Topic && fullPayload=="on") {
        digitalWrite(switch2, Use5Vrelay == true ? LOW : HIGH);
        //client.publish(mqttSwitch2Topic,"", true);
      }
      if (String(topic)==mqttSwitch2Topic && fullPayload=="off") {
        digitalWrite(switch2, Use5Vrelay == true ? HIGH : LOW);
        //client.publish(mqttSwitch2Topic,"", true);
      }
      if (String(topic)==mqttSwitch2MomentaryTopic && fullPayload=="pushed") {
        digitalWrite(switch2, Use5Vrelay == true ? LOW : HIGH);
        delay(300);
        digitalWrite(switch2, Use5Vrelay == true ? HIGH : LOW);
        client.publish(mqttSwitch2MomentaryTopic,"momentary_done", true);
        client.publish(mqttSwitch2MomentaryTopic,"", true);
      }
  }
} // MQTT CALLBACK

void reconnect() { // MQTT RECONNECT
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-";
    clientId += String(mqttDeviceName);
    clientId += "-";
    clientId += String(random(0xffff), HEX);
    Serial.print(clientId);
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqttSwitch1StateTopic,0,false,"LWT disconnected")) {
      Serial.println("...connected");
      // Once connected, publish an announcement...
      client.publish(mqttSwitch1StateTopic, "connected");
      client.publish(mqttSwitch2StateTopic, "connected");
      // ... and resubscribe
      client.subscribe(mqttSwitch1Topic);
      client.subscribe(mqttSwitch2Topic);
      client.subscribe(mqttSwitch1MomentaryTopic);
      client.subscribe(mqttSwitch2MomentaryTopic);
      mqtt_reconnect_count = 0;
    } else {
      //if ( mqtt_reconnect_count >= 12 ) { ESP.restart(); } // REBOOT IN 1 MINUTE
      if ( mqtt_reconnect_count >= 12 ) { setup_wifi(true); } // RESET WIFI IN 1 MINUTE
      mqtt_reconnect_count++;
      Serial.print(" - failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
} // MQTT RECONNECT

String uptime() {
  float d, hr, m, s;
  String dstr, hrstr, mstr, sstr;
  unsigned long currentMillis, over;
  currentMillis = millis();
  d = int( currentMillis / (3600000 * 24));
  dstr = String(d, 0);
  dstr.replace(" ", "");
  over = currentMillis % (3600000 * 24);
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
  if (d == 0) {
    return hrstr + ":" + mstr + ":" + sstr;
  }
  else if (d == 1) {
    return dstr + " Day " + hrstr + ":" + mstr + ":" + sstr;
  }
  else if (d > 1 && d <= 99 ) {
    return dstr + " Days " + hrstr + ":" + mstr;
  }
  else if (d >= 100 ) {
    return dstr + " D " + hrstr + ":" + mstr;
  }
}

void outputDHT(unsigned long varMillis) { // OUTPUT DHT
  #ifdef useDHT
    //unsigned long currentMillis = millis();
    float h = probeDHT(0);
    float tc = probeDHT(1);
    float tf = int((tc * 9.0 / 5.0) + 32.0);
    //float tf = round(tc*10)/10; // comment out this line or one above for Celcius conversion and see below comments for more changes
    char buf[8];
    //Serial.println (round(lastTemperaturePayload)); Serial.println (tf); Serial.println (round(tf));
    if ( ( varMillis>15000 && tc != -1000 ) && ( ( int(lastTemperaturePayload)!=int(tf) ) || ( varMillis % 300000 < 300 || lastTemperaturePayload==-1 ) ) ) { // uncomment for Fahrenheit
    //if ( ( varMillis>15000 && tc != -1000 ) && ( ( lastTemperaturePayload)!=tc ) || ( varMillis % 300000 < 300 || lastTemperaturePayload==-1 ) ) ) { // uncomment for Celcius
      //client.publish(mqttTemperatureTopic,dtostrf(tf, 0, 1, buf), true);  // uncomment for Celcius
      client.publish(mqttTemperatureTopic,dtostrf(tf, 0, 0, buf), true);  // uncomment for Fahrenheit
      lastTemperaturePayload=int(tf); // remove int for Celcius and add it for F
      //Serial.println ("mqttTemperature published");
    } else if (tc == -1000) {
      Serial.println ("DHT Temperature Reading Failed.");
    }
    //Serial.println (round(lastHumidityPayload)); Serial.println (h); Serial.println (round(h));
    if ( ( varMillis>15000 && h!=-1000 ) && ( ( int(lastHumidityPayload)!=int(h) && abs(lastHumidityPayload-h)>2 ) || ( varMillis % 300000 < 300 || lastHumidityPayload==-1 ) ) ) {
      //Serial.println ("mqttHumidity published");
      client.publish(mqttHumidityTopic,dtostrf(int(h), 0, 0, buf), true);
      lastHumidityPayload=int(h);
    } else if (h == -1000) {
      Serial.println ("DHT Humidity Reading Failed.");
    }
  #endif
} // OUTPUT DHT

float probeDHT(int whichSensor) {
  // Reading temperature or humidity takes about 250 milliseconds. Sensor readings may also be up to 2 seconds old
  #ifdef useDHT
    float h = -1000;
    float tc = -1000;
    int counter = 1;
    if (whichSensor == 0) {
      h = dht.readHumidity();
      while (counter <= 3 && (isnan(h) || h == -1000)) {
        if (isnan(h) || h == -1000) { h = dht.readHumidity(); } // re-read
        counter += 1; delay(50);
      }
    }
    else if (whichSensor == 1) {
      tc = dht.readTemperature();
      while (counter <= 3 && (isnan(tc) || tc == -1000)) {
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

void outputBME280(unsigned long varMillis) { // OUTPUT BME280
  #ifdef useBME280
    //unsigned long currentMillis = millis();
    float h = bme.readHumidity();
    float tc = bme.readTemperature();
    float tf = int((tc * 9.0 / 5.0) + 32.0);
    //float tf = round(tc*10)/10;; // comment out the line below and uncomment this line for Celcius conversion and see below for more changes
    char buf[8];
    //Serial.println (round(lastTemperaturePayload)); Serial.println (tf); Serial.println (round(tf));
    //if (tc < 10) { Serial.print("----------BAD TEMP----------   "); Serial.println(tc); }
    if ( ( varMillis>15000 && tc > -80 && tc!=0.00) && ( ( int(lastTemperaturePayload)!=int(tf) ) || ( varMillis % 300000 < 300 || lastTemperaturePayload==-1 ) ) ) { // uncomment for Fahrenheit
    //if ( ( varMillis>15000 && tc > -80 && tc!=0.00) && ( ( lastTemperaturePayload)!=tc ) || ( varMillis % 300000 < 300 || lastTemperaturePayload==-1 ) ) ) { // uncomment for Celcius
      //client.publish(mqttTemperatureTopic,dtostrf(tf, 0, 1, buf), true);  // uncomment for Celcius
      client.publish(mqttTemperatureTopic,dtostrf(tf, 0, 0, buf), true);  // uncomment for Fahrenheit
      lastTemperaturePayload=int(tf); // remove int for Celcius and add it for F
      //Serial.println ("mqttTemperature published");
    }
    //Serial.println (round(lastHumidityPayload)); Serial.println (h); Serial.println (round(h));
    if ( ( varMillis>15000 && h!=0 && h!=100 && h!=2147483647 ) && ( ( int(lastHumidityPayload)!=int(h) && abs(lastHumidityPayload-h)>2 ) || ( varMillis % 300000 < 300 || lastHumidityPayload==-1 ) ) ) {
      client.publish(mqttHumidityTopic,dtostrf(int(h), 0, 0, buf), true);
      lastHumidityPayload=int(h);
      //Serial.println ("mqttHumidity published");
    }
  #endif
} // OUTPUT BME280

void outputESP32Temp(unsigned long varMillis) { // OUTPUT ESP32TEMP
  #ifdef useESP32Temp
    float tf = temprature_sens_read();
    float tc = (tf - 32)/1.8;
    //float tf = round(tc*10)/10;; // comment out the line below and uncomment this line for Celcius conversion and see below for more changes
    char buf[8];
    //Serial.println (round(lastTemperaturePayload)); Serial.println (tf); Serial.println (round(tf));
    if ( ( varMillis>15000 && tc > -80 ) && ( ( int(lastTemperaturePayload)!=int(tf) ) || ( varMillis % 300000 < 300 || lastTemperaturePayload==-1 ) ) ) { // uncomment for Fahrenheit
    //if ( ( varMillis>15000 && tc > -80 ) && ( ( lastTemperaturePayload)!=tc ) || ( varMillis % 300000 < 300 || lastTemperaturePayload==-1 ) ) ) { // uncomment for Celcius
      //client.publish(mqttTemperatureTopic,dtostrf(tf, 0, 1, buf), true);  // uncomment for Celcius
      client.publish(mqttTemperatureTopic,dtostrf(tf, 0, 0, buf), true);  // uncomment for Fahrenheit
      lastTemperaturePayload=int(tf); // remove int for Celcius and add it for F
      //Serial.println ("mqttTemperature published");
    }
  #endif
} // OUTPUT ESP32TEMP

void OLEDscene(unsigned long varMillis, int whichScene) {
  #ifdef useOLED
    u8g2.clearBuffer();
    if (whichScene==1) {
      // UPTIME
      char upt[32];
      uptime().toCharArray(upt, 32);
      u8g2.setFont(u8g2_font_pxplusibmcgathin_8f);  // choose a suitable font
      u8g2.drawStr(0,12,"Uptime:");
      u8g2.drawStr(0,24,upt);
      // IP
      u8g2.setFont(u8g2_font_6x10_tf);  // choose a suitable font
      u8g2.drawStr(0,60,"IP: ");
      u8g2.drawStr(25,60,(char*) WiFi.localIP().toString().c_str());
      // TEMP & HUMIDITY
      #if defined(useDHT) || defined(useBME280) || defined(useESP32Temp)
        if (lastTemperaturePayload != -1) {
          char tTemp[5];
          String(int(lastTemperaturePayload)).toCharArray(tTemp, 5); // remove int for Celcius and add int for F
          char temp[7];
          strcpy(temp,tTemp);
          strcat(temp,"\xB0");
          strcat(temp,"F");
          u8g2.setFont(u8g2_font_pxplusibmcgathin_8f);  // choose a suitable font
          u8g2.drawStr(0,36,"Temp: ");
          u8g2.drawStr(50,36,temp);
        }
      #endif
      #if defined(useDHT) || defined(useBME280)
        if (lastHumidityPayload != -1) {
          char tHmdt[4];
          String(int(lastHumidityPayload)).toCharArray(tHmdt, 4);
          char hmdt[5];
          strcpy(hmdt,tHmdt);
          strcat(hmdt,"%");
          u8g2.setFont(u8g2_font_pxplusibmcgathin_8f);  // choose a suitable font
          u8g2.drawStr(0,48,"Humidity: ");
          u8g2.drawStr(80,48,hmdt);
        }
      #endif
    } else if (whichScene==2) {
      u8g2.setFont(u8g2_font_6x10_tf);  // choose a suitable font
      u8g2.drawStr(0,12,digitalRead(switch1) ? "Switch 1: off" : "Switch 1: on");
      u8g2.drawStr(0,24,digitalRead(switch2) ? "Switch 2: off" : "Switch 2: on");
      #ifdef CONTACTPIN1
        u8g2.drawStr(0,36,digitalRead(CONTACTPIN1) ? "Contact 1: open" : "Contact 1: closed");
      #endif
      #ifdef CONTACTPIN2
        u8g2.drawStr(0,48,digitalRead(CONTACTPIN2) ? "Contact 2: open" : "Contact 2: closed");
      #endif
    }
    u8g2.sendBuffer();          // transfer internal memory to the display
  #endif
}