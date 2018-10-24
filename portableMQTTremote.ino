/* 
*  Portable MQTT Remote Controller 
* 
* Instructions:
* Please update the your wifi and mqtt settings and check pin numbers for encoder.
* Please note that code uses I2C display
* 
* Copyright (c) 2018 Erdi Kuşdemir erdikusdemir@gmail.com
*
* MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this hardware,
* software, and associated documentation files (the "Product"), to deal in the Product without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Product, and to permit persons to whom the
* Product is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Product.
*
* THE PRODUCT IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
* NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE PRODUCT OR THE USE OR OTHER DEALINGS IN THE PRODUCT. 
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "ArduinoJson.h"
#include "Button2.h";
#include "ESPRotary.h";

// Update these with values suitable for your network.
const char* ssid = "yourwifiid";
const char* password = "yourwifipass";
const char* mqtt_server = "yourmqttserver";
const char* mqtt_username = "yourmqttuser";
const char* mqtt_password = "mqttpass";
const char* outTopic = "kumanda1/in";
const char* inTopic = "kumanda1/out";
#define ROTARY_PIN1 D6 //--------Update encoder pins which is suitable for you.
#define ROTARY_PIN2 D7
#define BUTTON_PIN D3
#define movesperclick 2 //--encoder click to movement constant. 
//--------------------------------------------------------------------------------
//--------------------You don't need to alternate the code below here ------------ 
ESPRotary enc = ESPRotary(ROTARY_PIN1, ROTARY_PIN2, movesperclick);
Button2 encbtn = Button2(BUTTON_PIN);
const size_t bufferSize = JSON_OBJECT_SIZE(4) + 70;
DynamicJsonBuffer JSONBuffer(bufferSize);
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);
WiFiClient espClient;
PubSubClient client(espClient);

char data[80];
int mid=3;
const char* menu0[4] = {"id1", "id2", "id3", "id4"};
boolean state[] = {0, 0, 0, 0};
String statetemp;
int slider[] = {0, 0, 0, 0};
boolean svalid[] = {0,0,0,0};
boolean mainmenu = 1;
boolean initseq = 0;
int seqcounter = 0;
int cursorpos = 0;
long lastupdatetimer=0;
boolean lastreflesh = 0;

void setup() {
enc.setChangedHandler(rotate);
encbtn.setClickHandler(btnhandler);
// encbtn.setLongClickHandler(btnhandler);
encbtn.setDoubleClickHandler(btnhandler);
//encbtn.setTripleClickHandler(btnhandler);
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
display.clearDisplay();
display.setTextSize(1);
display.setTextColor(WHITE);
Serial.begin(115200);
setup_wifi();
client.setServer(mqtt_server, 1883);
client.setCallback(callback);
}//end of setup

void loop() {
if (!client.connected())reconnect();
client.loop();
enc.loop();
encbtn.loop();
}//end of loop

void rotate(ESPRotary& enc) {
if(mainmenu){
cursorpos=cursorpos+enc.getPosition();
if(cursorpos<0)cursorpos=0;
if(cursorpos>=mid)cursorpos=mid;
}
else {
slider[cursorpos]=slider[cursorpos]+10*(enc.getPosition());
if(slider[cursorpos]<0)slider[cursorpos]=0;
if(slider[cursorpos]>100)slider[cursorpos]=100;
if(lastupdatetimer<millis()-200){outputhandler();lastupdatetimer=millis();}
else {if(slider[cursorpos]==100)outputhandler();if(slider[cursorpos]==0)outputhandler();}
}//end else
enc.resetPosition();
screenhandler();
}//end rotate

void btnhandler(Button2& btn) {
switch (btn.getClickType()) {
case SINGLE_CLICK:
state[cursorpos]=!state[cursorpos];
if(mainmenu==0){state[cursorpos]=0;mainmenu=1;}
if(mainmenu&&state[cursorpos])slider[cursorpos]=100;
if(mainmenu&&state[cursorpos]==0)slider[cursorpos]=0;
outputhandler();
break;
case DOUBLE_CLICK:
if(svalid){
if(mainmenu&&state[cursorpos]==0){state[cursorpos]=1;slider[cursorpos]=100;}
mainmenu=!mainmenu;
}
break;
//case TRIPLE_CLICK:
//if(svalid){
//mainmenu=!mainmenu;
//if(mainmenu==0&&state[cursorpos]==0)state[cursorpos]=1;slider[cursorpos]=100;
//}
//break;
//case LONG_CLICK:
//Serial.print("long");
//break;
}
screenhandler();
}//end of btnhandler

void screenhandler(){
display.fillRect(0,0,8,64,BLACK);
display.fillRect(0,cursorpos*8,128,8,BLACK);
display.setCursor(0,cursorpos*8);//cursor
display.println(">");
display.setCursor(8,cursorpos*8);
display.println(menu0[cursorpos]);
display.setCursor(90,cursorpos*8);
if(state[cursorpos])statetemp="ON";
else statetemp="OFF";
display.println(statetemp);
if (svalid[cursorpos]==1){
display.setCursor(110,cursorpos*8);
display.println(slider[cursorpos]);
}//end svalid
display.display();
}//end of screen handler

void callback(char* topic, byte* payload, unsigned int length) {
//Serial.print("Message arrived [");
//Serial.print(topic);
//Serial.print("] ");
//for (int i = 0; i < length; i++) {
//Serial.print((char)payload[i]);
//}
//Serial.println();
JsonObject& root = JSONBuffer.parseObject((const char*)payload); //Parse message
//if (!root.success()) {   //Check for errors in parsing
//Serial.println("Parsing failed");
//delay(5000);
//return;
//}
cursorpos = root["id"];
state[cursorpos] = root["state"];
slider[cursorpos] = root["slider"];
if(initseq){
menu0[cursorpos] =root["name"];
svalid[cursorpos] = root["svalid"];
mid= root["mid"];
mid--;
if(seqcounter>=mid){initseq=0;seqcounter=0;lastreflesh=1;}
seqcounter++;
}//end initseq
screenhandler();//---------------
if(lastreflesh){cursorpos = 0;screenhandler();lastreflesh=0;}
}//end of callback

void outputhandler() {
String payload = "{\"id\": "+ String(cursorpos) +", \"state\": " + String(state[cursorpos]) +", \"slider\": " + String(slider[cursorpos])+ "}";
payload.toCharArray(data, (payload.length() + 1));
client.publish(outTopic , data);
}//end of outputhandler

void initdata(){//init data from HA -call is inside void reconnect()
String payload = "{\"id\": 99}";
payload.toCharArray(data, (payload.length() + 1));
client.publish(outTopic , data);
initseq= 1;
}//end of init

void setup_wifi() {
delay(10);
// We start by connecting to a WiFi network
Serial.println();
Serial.print("Connecting to ");
Serial.println(ssid);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) { 
delay(500);
Serial.print(".");
}
Serial.println("");
Serial.println("WiFi connected");
Serial.println("IP address: ");
Serial.println(WiFi.localIP());
}//end of setupwifi
void reconnect() {
while (!client.connected()) {
Serial.print("Attempting MQTT connection...");
if (client.connect("deneme", mqtt_username, mqtt_password)) {
Serial.println("connected");
client.subscribe(inTopic);
initdata();
} else {
Serial.print("failed, rc=");
Serial.print(client.state());
Serial.println(" try again in 5 seconds");
delay(5000);
    }
  }
}//end of reconnect

