#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "ArduinoJson.h"

// Update these with values suitable for your network.

const char* ssid = "yourwifiid";
const char* password = "yourwifipass";
const char* mqtt_server = "yourmqttserver";
const char* mqtt_username = "yourmqttuser";
const char* mqtt_password = "mqttpass";
const char* outTopic = "kumanda1/in";
const char* inTopic = "kumanda1/out";

char data[80];
//StaticJsonBuffer<300> JSONBuffer;
const size_t bufferSize = JSON_OBJECT_SIZE(3) + 30;
DynamicJsonBuffer JSONBuffer(bufferSize);
//---
#define A_PIN D6 //pin numbers for rotary encoder
#define B_PIN D7
#define BUT_PIN D3
#define VCC_PIN -1 /*put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(A_PIN, B_PIN, BUT_PIN, VCC_PIN);
//---
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);
#define maxItemSize 13
char Menu0[][maxItemSize] = {"Living Room", "Kitchen", "Kitchen Side", "Hall"}; //----------------------------------------------Name of the items. Static for now.
//char state[3] = "ON";
boolean state[] = {0, 0, 0, 0};
String statetemp;
int slider[] = {0, 0, 0, 0};
boolean sliderpres[] = {1,1,0,0};
boolean updated = 1;
boolean clicked = 0;
boolean mainmenu = 1;
long dclicktimer = 0;
int cursorpos = 0;
long sleeptimer = 0;
//----------some constants
int dclicktimeout = 250;


//------------------------------------------------------------------------------


WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  rotaryEncoder.begin();
  rotaryEncoder.setBoundaries(0, 3, true); //minValue, maxValue, cycle values (when max go to min and vice versa)

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
  // init done
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
//  display.setCursor(0,0);
//  display.println("Hello, world!");
//  display.display();
//  delay(2000);
//  display.clearDisplay();
Serial.begin(115200);
 setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}
//------------------------------------------------------------------------------
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

rotary_loop();
screenhandler();
  
  if ((millis() - sleepTimer >= SLEEP_TIMEOUT)) {
  // SLEEP_TIMEOUT time has passed since last button push
    Serial.println("going to sleep...");
    sleepNow();
  }

  }


  void screenhandler() 
  {
//Menu0 handler
  display.setCursor(0,cursorpos*8);//cursor
  display.println(">");
  for (int i=0; i < sizeof(Menu0)/maxItemSize; i++){
   display.setCursor(8,i*8);
   display.println(Menu0[i]);
   display.setCursor(90,i*8);
   if(state[i])statetemp="ON";
   else statetemp="OFF";
   display.println(statetemp);
   if (sliderpres[i]==1){
   display.setCursor(110,i*8);
   display.println(slider[i]);
   }//end sliderpres
  }//end for menu0
  display.display();
  //endof Menu0 handler
  
  display.clearDisplay();
    }

void rotary_loop() {
  //first lets handle rotary encoder button click
  if (rotaryEncoder.currentButtonState() == BUT_RELEASED) {
 if(clicked==0) {dclicktimer= millis(); clicked=1;}//pressed one counter
  else if (clicked&&sliderpres[cursorpos])
      {
    mainmenu = !mainmenu;
    updated=1;
    clicked=0;
    
    }//dclicked
  }
   if(clicked&&(millis()-dclicktimer)>dclicktimeout){
    state[cursorpos]= !state[cursorpos];
    //if(state[cursorpos]==0&&mainmenu==0) mainmenu=1;
    if(state[cursorpos])slider[cursorpos]=100;
    else slider[cursorpos]=0;
    clicked=0;
    outputhandler();//--------------------------------------------------------------------------
    }//clickedonce


if(updated){//encoder mode definer
if(mainmenu) {
  rotaryEncoder.setBoundaries(0, 3, true);
  rotaryEncoder.reset(cursorpos);
  updated=0;
  }
else {
  rotaryEncoder.setBoundaries(0, 10, false);
  rotaryEncoder.reset(slider[cursorpos]/10);
  updated=0;
  }
}//endif updated

  int16_t encoderDelta = rotaryEncoder.encoderChanged();
  if (encoderDelta == 0) return;//optionally we can ignore whenever there is no change
  if (encoderDelta!=0) {//if value is changed compared to our last read
    int16_t encoderValue = rotaryEncoder.readEncoder();
    if(screensleep)
    if(mainmenu) cursorpos=encoderValue;
    else 
    {
      slider[cursorpos]=10*encoderValue;
      outputhandler();//---------------------------------------------------------------------------
    }
    
  } //endif encoderdelta
  
}//endif rotary loop

void outputhandler() {

  // Format your message to Octoblu here as JSON
  // Include commas between each added element. 
  // This sends off your payload. 
  String payload = "{\"item\": "+ String(cursorpos) +", \"state\": " + String(state[cursorpos]) +", \"slider\": " + String(slider[cursorpos])+ "}";
  payload.toCharArray(data, (payload.length() + 1));
client.publish(outTopic , data);
}

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
}
//-----------------------------------------------callback---------------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
JsonObject& root = JSONBuffer.parseObject(payload); //Parse message
// if (!root.success()) {   //Check for errors in parsing
// 
//    Serial.println("Parsing failed");
//    delay(5000);
//    return;
// 
//  }
cursorpos = root["item"];
 state[cursorpos] = root["state"];
 slider[cursorpos] = root["slider"];
 cursorpos = 0;
  // Switch on the LED if an 1 was received as first character
  
}
//-------------------------------------------------------------------------------------------------
void initdata(){
//init data from HA
  String payload = "{\"item\": 99}";
  payload.toCharArray(data, (payload.length() + 1));
client.publish(outTopic , data);
//end of init
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("deneme", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish(outTopic, "hello world");
      // ... and resubscribe
      initdata();
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

