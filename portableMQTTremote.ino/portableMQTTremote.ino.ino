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
#include "Button2.h";

// Update these with values suitable for your network.
const char* deviceName = "remote1";// if more than 1 remote is exist, change the client id.
const char* ssid = "wifi ssid";
const char* password = "wifi password";
const char* mqtt_server = "mqtt server ip";
const char* mqtt_clientid = deviceName; 
const char* mqtt_username = "mqtt server username";
const char* mqtt_password = "mqtt server password";
const char* outTopic = "kumanda1/in";
const char* inTopic = "kumanda1/out";
IPAddress ip(192, 168, 1, 11); //IP Address for remote //static IP is used for faster connection.
IPAddress gateway(192, 168, 1, 1);   //IP Address of your WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns(192, 168, 1, 1);  //DNS

#define PIN_A D6    //ky-040 clk pin, interrupt, add 100nF/0.1uF capacitors between pin & ground
#define PIN_B D7    //ky-040 dt  pin,            add 100nF/0.1uF capacitors between pin & ground
#define BUTTON D5    //ky-040 sw  pin, interrupt
byte currValueAB;
byte prevValueAB;
int counter=0;
boolean buttonState=1;

Button2 encbtn = Button2(BUTTON);
const size_t bufferSize = JSON_OBJECT_SIZE(4) + 70;
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
int slider[maxitems] = {0, 0, 0, 0};
boolean svalid[maxitems] = {0,0,0,0};
boolean mainmenu = 1;
boolean initseq = 0;
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

void batlevel(){
float raw = analogRead(A0);
if(raw<=735){ // if bat is low display message
display.setCursor(0,0);
display.println("Battery is LOW");
display.display();
  }//end if 
//String payload = "{\"bat\": "+ String(raw) + "}";
//payload.toCharArray(data2, (payload.length() + 1));
//client.publish( "kumanda1/bat", data2);
} //end of batlevel

void setup() {
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
encbtn.setClickHandler(btnhandler);
encbtn.setDoubleClickHandler(btnhandler);
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
display.clearDisplay();
display.setTextSize(1);
display.setTextColor(WHITE);
display.setCursor(32,12);
display.println("Loading...");
display.display();
batlevel();
client.setServer(mqtt_server, 1883);
client.setCallback(callback);
sleeper=millis();
}//end of setup

void reconnect() {
if (client.connect(mqtt_clientid, mqtt_username, mqtt_password)) {
client.subscribe(inTopic);
} else {
Serial.print("failed, rc=");
Serial.print(client.state());
Serial.println(" try again in 5 seconds");
display.setCursor(0,0);//cursor
display.println("MQTT connection failed.");
display.display();
display.clearDisplay();
delay(5000);
}
Serial.println("connected");
Serial.println(millis()-basla);
}//end of reconnect

void initdata(){//init data from HA -call is inside void reconnect()
if (client.connected()){
String payload = "{\"id\": 99}";
payload.toCharArray(data, (payload.length() + 1));
client.publish(outTopic , data);
initseq= 1;
}
}//end of init

void callback(char* topic, byte* payload, unsigned int length) {
JsonObject& root = JSONBuffer.parseObject((const char*)payload); //Parse message
syscursor = root["id"];
state[syscursor] = root["state"];
slider[syscursor] = root["slider"];
if(initseq){
menu0[syscursor] =root["name"];
svalid[syscursor] = root["svalid"];
mid= root["mid"];
mid--;
if(seqcounter>=mid){initseq=0;seqcounter=0;lastreflesh=1;screenhandler();}
seqcounter++;
}//end initseq
Serial.println(mid);
}//end of callback

void outputhandler() {
String payload = "{\"id\": "+ String(dispcursor) +", \"state\": " + String(state[dispcursor]) +", \"slider\": " + String(slider[dispcursor])+ "}";
payload.toCharArray(data, (payload.length() + 1));
client.publish(outTopic , data);
outputchanged=0;
}//end of outputhandler

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
encbtn.loop();
if(initseq==0)screenhandler();
if(outputchanged&&(lastupdatetimer<millis()-200)){outputhandler();lastupdatetimer=millis();}
else {if(outputchanged&&slider[dispcursor]==100)outputhandler();if(outputchanged&&slider[dispcursor]==0)outputhandler();}
if (millis()>15000+sleeper){screensaver();sleeper=millis();}//deep sleep
}//end of loop

void screenhandler(){
page=dispcursor/4;
maxscan=3;
//Serial.println(mid);
if((mid-(page+1)*4<4)&&(page==mid/4))maxscan=mid%4;
display.setCursor(0,(dispcursor%4)*8);//cursor
display.println(">");
for (int i=0; i <= maxscan; i++){
display.setCursor(8,i*8);
display.println(menu0[i+page*4]);
display.setCursor(90,i*8);
if(state[i+page*4])statetemp="ON";
else statetemp="OFF";
display.println(statetemp);
if (svalid[i+page*4]==1){
display.setCursor(110,i*8);
display.println(slider[i+page*4]);
}//end svalid
}//end for
display.display();
display.clearDisplay();
}//end of screen handler

void screensaver()
{
display.ssd1306_command(SSD1306_DISPLAYOFF);
ssaver=1;
mainmenu=1;
Serial.println("byby");
ESP.deepSleep(0);
}//end of screensaver

void disablescreensaver()
{
//display.ssd1306_command(SSD1306_DISPLAYON);
ssaver=0;
//digitalWrite(OLED_VCC,HIGH);
//display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}// end of disscreensavers

void encmove()
{
noInterrupts();
currValueAB  = digitalRead(PIN_A) << 1;
currValueAB |= digitalRead(PIN_B);
switch ((prevValueAB | currValueAB))
{
case 0b0001: case 0b1110://CW states for 1 count  per click, use "case 0b0001: case 0b1110: case 0b1000: case 0b0111:" for CW states for 2 counts per click
if(mainmenu){ //artı gidisler
dispcursor=dispcursor+1;
if(dispcursor<0)dispcursor=0;
if(dispcursor>=mid)dispcursor=mid;
}
else {
slider[dispcursor]=slider[dispcursor]+10;
if(slider[dispcursor]<0)slider[dispcursor]=0;
if(slider[dispcursor]>100)slider[dispcursor]=100;
outputchanged=1;
}//end else
break;
case 0b0100: case 0b1011://CCW states for 1 count  per click, use "case 0b0100: case 0b1011: case 0b0010: case 0b1101:" for CCW states for 2 counts per click
if(mainmenu){ // eksi gidislerrrr
dispcursor=dispcursor-1;
if(dispcursor<0)dispcursor=0;
if(dispcursor>=mid)dispcursor=mid;
}
else {
slider[dispcursor]=slider[dispcursor]-10;
if(slider[dispcursor]<0)slider[dispcursor]=0;
if(slider[dispcursor]>100)slider[dispcursor]=100;
outputchanged=1;
}//end else
break;
}
prevValueAB = currValueAB << 2;
interrupts();
sleeper=millis();
if(ssaver)disablescreensaver();
}//end of encoder move

void btnhandler(Button2& btn) {
switch (btn.getClickType()) {
case SINGLE_CLICK:
state[dispcursor]=!state[dispcursor];
if(mainmenu==0){state[dispcursor]=0;mainmenu=1;}
if(mainmenu&&state[dispcursor])slider[dispcursor]=100;
if(mainmenu&&state[dispcursor]==0)slider[dispcursor]=0;
outputchanged=1;
break;
case DOUBLE_CLICK:
if(svalid){
if(mainmenu&&state[dispcursor]==0){state[dispcursor]=1;slider[dispcursor]=100;}
mainmenu=!mainmenu;
}
break;
}
sleeper=millis();
}//end of btnhandler
