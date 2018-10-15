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
5. LIPO battery
6. 3D printed enclosure

In this project I connect remote controller to my Home Assistant (HA). 
Remote controller communicates with Node-Red flow by MQTT Json messages. Node-Red flow converts the messages from both remote and HA side and makes implementation easier.
Json messages contains 3 messages. 1. ID number of the item 2. state of item (on/off) 3. "slider" of item if it is dimmable. 
