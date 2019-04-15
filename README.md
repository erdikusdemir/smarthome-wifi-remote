


Sorry for poorly compiled description. I will update and make it easy to understand.  

# DESCRIPTION 

Portable remote controller to control smart home devices over MQTT protocol. It is based on ESP8266 with OLED LCD and rotary encoder.

The remote controller communicates with Node-Red flow by MQTT JSON messages. Node-Red flow converts the signals from both remote and HA side and makes implementation easier. Items you would like to control by remote can be edited by "config file" function. Arduino sketch only configures Wifi and MQTT settings. All the item information are called at the initialization stage of the remote.
JSON messages contain 3 messages. 1. The ID number of the item 2. state of the item (on/off) 3. "slider" of the item if it is dimmable. 

Wemos chip falls in a deep sleep to reduce power consumption after 30 seconds of idle. Reset switch needs to be pressed to awaken the board again.

Hardware is consist of;
1. Wemos D1 mini Pro,
2. A clickable rotary encoder,
3. 0.96" I2C 128x64 OLED display,
4. Wemos battery shield,
5. LIPO battery,
6. 3D printed enclosure (under progress),

<img src="https://github.com/erdikusdemir/smarthome-wifi-remote/blob/master/remote_sizing.jpg" width="400">
<img src="https://github.com/erdikusdemir/smarthome-wifi-remote/blob/master/remote_extended.jpg" width="400">
<img src="https://github.com/erdikusdemir/smarthome-wifi-remote/blob/master/remote_wemos.jpg" width="400">

# Limitations:  
The number of an item is fixed to 20 because of code restrictions. It can be changed inside the Arduino code.  
Temperature controlling is possible but not configured yet.

# Instructions:  
1. Connect your encoder pins as:  
ROTARY_PIN1  D6,  
ROTARY_PIN2 D7,  
BUTTON_PIN  D3,  
2. Connect your I2C LCD to HW I2C port,  
3.1. Open the Arduino sketch and update your wifi and MQTT server information,  
3.2. Copy the libraries to documents/Arduino folder,  
4.1. Copy NodeRed flow into your NodeRed server,  
4.2. Configure your HA and MQTT servers,  
4.3. Configure your items by editing "config file" function,  
(id: order in your OLED screen, HAid: id name of item you want to control, svalid: is item dimmable?)  
4.4. Deploy the node and everything should works.
