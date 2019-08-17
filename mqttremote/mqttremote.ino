#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "ArduinoJson.h"

// Update these with values suitable for your network.
const char* deviceName = "remote1";// if more than 1 remote is exist, change the devicename.
const char* ssid = "yourssid";
const char* password = "andpassword";
const char* mqtt_server = "yourmqttserverip";
const char* mqtt_clientid = deviceName;
const char* mqtt_username = "username";
const char* mqtt_password = "password";
const char* outTopic = "kumanda1/in";
const char* inTopic = "kumanda1/out";
IPAddress ip(192, 168, 1, 11); //IP Address for remote
IPAddress gateway(192, 168, 1, 1);   //IP Address of your WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns(192, 168, 1, 1);  //DNS

#define PIN_A D6    //ky-040 clk pin, interrupt, 
#define PIN_B D7    //ky-040 dt  pin,            
int killsw = D0;
int encbtn = D5;

bool btnstate = LOW;
bool laststate = LOW;
bool btnpressed= LOW;
unsigned long now = 0;
unsigned long lastbouncetime =0;
int debouncetime = 20;
int multiclicktime =200;
int longclicktime = 1000;
int clickcount = 0;
int clickcluck = 0;

byte currValueAB;
byte prevValueAB;
int counter=0;

const size_t bufferSize = JSON_OBJECT_SIZE(7) + 70;
DynamicJsonBuffer JSONBuffer(bufferSize);

#define OLED_RESET -1
#define OLED_VCC D3
Adafruit_SSD1306 display(OLED_RESET);
WiFiClient espClient;
PubSubClient client(espClient);

const int maxitems= 20; //max number of item can be allocated by memory :( only 5 page of item
char data[80];
char data2[80];
int mid=3;
const char* menu0[maxitems] = {"id1", "id2", "id3", "id4"};
boolean state[maxitems] = {0, 0, 0, 0};
String statetemp;
int value0[maxitems] = {0, 0, 0, 0};
int value1[maxitems] = {0, 0, 0, 0};
int type[maxitems] = {0,0,0,0};
boolean mainmenu = 1;
boolean initseq = -1;
int seqcounter = 0;
int dispcursor=0;
int syscursor=0;
long lastupdatetimer=0;
long sleeper=0;
boolean lastreflesh = 0;
int page=0;
int maxscan=3;
boolean outputchanged=0;
boolean ssaver = 0;
long basla=0;
float tempread[maxitems] = {0,0,0,0};
int batlvl[3] = {0,0,0};
int avgbatlvl = 0;
long battimer = 0;

void batlevel(){
batlvl[2]=batlvl[1];
batlvl[1]=batlvl[0];
batlvl[0] = analogRead(A0);
avgbatlvl = (batlvl[0]+batlvl[1]+batlvl[2])/3;
} //end of batlevel

void setup() {
pinMode(killsw,OUTPUT);
digitalWrite(killsw,HIGH);
basla=millis();
Serial.begin(115200);
WiFi.mode(WIFI_STA);
WiFi.hostname(deviceName);
WiFi.config(ip, subnet, gateway, dns);
WiFi.begin(ssid, password);
pinMode(PIN_A,INPUT_PULLUP); //enable internal pull-up resistors 
pinMode(PIN_B,INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(PIN_A),encmove,CHANGE);  //call encmove()    when high->low or high->low changes happened
attachInterrupt(digitalPinToInterrupt(PIN_B),encmove,CHANGE);
pinMode(encbtn, INPUT);
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
//display.begin(SSD1306, 0x3C);
display.clearDisplay();
display.setTextSize(1);
display.setTextColor(WHITE);
display.setCursor(32,12);
display.println("Loading...");
display.display();
//batlevel();
client.setServer(mqtt_server, 1883);
client.setCallback(callback);
sleeper=millis();
battimer=millis();
}//end of setup


void loop() { //--------------------------------------------------void loop--------------------
if(!client.connected()) {
if (WiFi.status() == WL_CONNECTED){
reconnect();
if(client.connected()) initdata();
}
else{
delay(10);
Serial.print(".");
}
} //client if
client.loop();
btnupdate();
if(initseq==0)screenhandler();
if(outputchanged&&(lastupdatetimer<millis()-200)){outputhandler();lastupdatetimer=millis();}
else {if(outputchanged&&value0[dispcursor]==100)outputhandler();if(outputchanged&&value0[dispcursor]==0)outputhandler();}
if (millis()>1000+battimer){batlevel();battimer=millis();}//bat level
if (millis()>15000+sleeper)digitalWrite(killsw,LOW);//kill switch
}//end of loop
