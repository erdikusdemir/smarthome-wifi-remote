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
Serial.println((const char*)payload);
JsonObject& root = JSONBuffer.parseObject((const char*)payload); //Parse message
syscursor = root["id"];
state[syscursor] = root["state"];
value0[syscursor] = root["value0"];
if(initseq){
menu0[syscursor] =root["name"];
//Serial.println(menu0[syscursor]);
type[syscursor] = root["type"];
if(type[syscursor]==2) value1[syscursor]=root["value1"];
mid= root["mid"];
mid--;
if(seqcounter>=mid){initseq=0;seqcounter=0;lastreflesh=1;screenhandler();}
seqcounter++;
}//end initseq
if(type[syscursor]==2) value1[syscursor]=root["value1"];
Serial.println(seqcounter);
Serial.println(mid);
}//end of callback

void outputhandler() {
  String payload;
if(type[dispcursor]==2)payload = "{\"id\": "+ String(dispcursor) +", \"state\": " + String(state[dispcursor]) +", \"value1\": " + String(value1[dispcursor])+ "}";
else payload = "{\"id\": "+ String(dispcursor) +", \"state\": " + String(state[dispcursor]) +", \"value0\": " + String(value0[dispcursor])+ "}";
payload.toCharArray(data, (payload.length() + 1));
client.publish(outTopic , data);
outputchanged=0;
}//end of outputhandler
