# portableMQTTremote
ESP8266 based portable remote controller with OLED LCD for smart home implementation

Sorry for poorly compiled description. I will update and make it easy to understanble.

DESCRIPTION 

Portable remote controler to control smart home devices over MQTT protocol.
Hardware is consist of;
1. Wemos D1 mini
2. Clickable rotary encoder
3. 0.96" I2C 128x64 OLED display
4. Wemos battery sheild
5. LIPO battery (under progress)
6. 3D printed enclosure (under progress)

Limitations:
Only 4 items fit in screen. Page system will come soon.
Temperature controlling is possible but not configured yet.

Instructions:
1.Connect your encoder pins as
ROTARY_PIN1  D6
ROTARY_PIN2 D7
BUTTON_PIN  D3
2.Connect your I2C LCD to HW I2C port
3.Open the arduino skecth and update your wifi and mqtt server informations.
4.Copy the libraries to documents/arduino folder.
5.Copy NodeRed flow into your NodeRed server.
6.Configure your HA and MQTT servers.
7.Configure your items by editing config file function.

In this project I connect remote controller to my Home Assistant (HA). 
Remote controller communicates with Node-Red flow by MQTT Json messages. Node-Red flow converts the messages from both remote and HA side and makes implementation easier.
Json messages contains 3 messages. 1. ID number of the item 2. state of item (on/off) 3. "slider" of item if it is dimmable. 
