


I am working on this project on left over times from my PhD thesis.

# DESCRIPTION 

Portable remote controller to control smart home devices over MQTT protocol. It is based on ESP8266 with OLED LCD and rotary encoder.

The remote controller communicates with Node-Red flow by MQTT JSON messages. Node-Red flow converts the signals from both remote and HA side and makes implementation easier. Items you would like to control by remote can be edited by "config file" function. Arduino sketch only configures Wifi and MQTT settings. All the item information are called at the initialization stage of the remote.

Wemos kill its power after 15 secs of idle time. Thus, power consumption of the remote is zero while it is unused. 

Hardware is consist of;
1. Wemos D1 mini Pro,
2. A clickable rotary encoder,
3. 0.96" I2C 128x64 OLED display,
4. Wemos battery shield,
5. LIPO battery,
6. 3D printed enclosure (under progress),  
7. TPS27081ADDCR load switch,  
8. 2222A NPN transistor,  
9. 330, 1 k, and 1 MOhm resistors.  

<img src="https://github.com/erdikusdemir/smarthome-wifi-remote/blob/master/remote_insidecover.jpg" width="400">
<img src="https://github.com/erdikusdemir/smarthome-wifi-remote/blob/master/Schematic.PNG" width="400">

# Limitations:  
The number of an item is fixed to 20 because of code restrictions. It can be changed inside the Arduino code.  

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
