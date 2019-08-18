void screenhandler(){
if(dispcursor==-1)page=-1;
else page=dispcursor/4;
if(page==-1){ //maintenance menu
display.setCursor(8,0); //setup item1
display.println("BAT lvl: " + String(avgbatlvl));
display.setCursor(8,8); //setup item2
display.println("SSID: ");
display.setCursor(42,8); //setup item2
display.println(WiFi.SSID());
display.setCursor(8,16); //setup item3
display.println("IP: ");
display.setCursor(26,16); //setup item3
display.println(WiFi.localIP());
display.setCursor(8,24); //setup item4
//display.println("TCP connection: ");
if(client.state()==0)display.println("MQTT is connected.");
else display.println("MQTT is not connected.");
}// end of if page
else {
maxscan=3;
if((mid-(page+1)*4<4)&&(page==mid/4))maxscan=mid%4;
display.setCursor(0,(dispcursor%4)*8);//cursor
display.println(">");
for (int i=0; i <= maxscan; i++){
display.setCursor(8,i*8);
display.println(menu0[i+page*4]);
display.setCursor(90,i*8);
if ((type[i+page*4]==2)&&(state[i+page*4]!=0)){
display.println(value1[i+page*4]);
}
else{
if(state[i+page*4])statetemp="ON";
else statetemp="OFF";
display.println(statetemp);
}
if (type[i+page*4]>=1){
display.setCursor(110,i*8);
display.println(value0[i+page*4]);
}//end type
}//end for
}//end of else page
display.display();
display.clearDisplay();
}//end screenhandler

void encmove()
{
noInterrupts();
currValueAB  = digitalRead(PIN_A) << 1;
currValueAB |= digitalRead(PIN_B);
switch ((prevValueAB | currValueAB))
{
case 0b0001: case 0b1110://CW states for 1 count  per click, use "case 0b0001: case 0b1110: case 0b1000: case 0b0111:" for CW states for 2 counts per click
dispcursorupdate(+1);
break;
case 0b0100: case 0b1011://CCW states for 1 count  per click, use "case 0b0100: case 0b1011: case 0b0010: case 0b1101:" for CCW states for 2 counts per click
dispcursorupdate(-1);
break;
}
prevValueAB = currValueAB << 2;
interrupts();
sleeper=millis();
}//end of encoder move

void dispcursorupdate(int incdec) {
  if(mainmenu){ //artı gidisler
  dispcursor=dispcursor+incdec;
  if(dispcursor<-1)dispcursor=-1;
  if(dispcursor>=mid)dispcursor=mid;
}
else {
  if(type[dispcursor]==2){ //value1 is a set temp
  value1[dispcursor]=value1[dispcursor]+incdec;
  if(value1[dispcursor]<18)value1[dispcursor]=18;
  if(value1[dispcursor]>30)value1[dispcursor]=30;
  }//end if temp 
  if(type[dispcursor]==1){// value0 is set dimmer
  value0[dispcursor]=value0[dispcursor]+10*incdec;
  if(value0[dispcursor]<0)value0[dispcursor]=0;
  if(value0[dispcursor]>100)value0[dispcursor]=100;
  }//end if dimmer
outputchanged=1;
}//end else
}//end of dispcursorupdate

void btnintr()
{
  noInterrupts();
  now =millis();      // get current time
  if (now - lastbouncetime > debouncetime )
  {
  btnpressed = HIGH;
  clickcount++;
  }
  lastbouncetime = now;
  interrupts();
}//end of btnintr

void btnupdate()
{
  now =millis();      // get current time
  if (btnpressed && (now - lastbouncetime) > multiclicktime)
  {
  btnpressed=0;
    if(clickcount != 0) 
    {
      //click sayısı kadar işlem yap
    if(clickcount==1)
    {
    state[dispcursor]=!state[dispcursor];
    if(mainmenu==0){state[dispcursor]=0;mainmenu=1;}
    if(mainmenu&&state[dispcursor]&&(type[dispcursor]==1))value0[dispcursor]=100;
    if(mainmenu&&state[dispcursor]==0&&(type[dispcursor]==1))value0[dispcursor]=0;
    outputchanged=1;
    }
    
    if(clickcount==2)
    {
    if(type){
    if(mainmenu&&state[dispcursor]==0){state[dispcursor]=1;value0[dispcursor]=100;}
    mainmenu=!mainmenu;
    }
    }
    clickcount=0;
    sleeper=millis();
    }
  }

  // Check for "long click"
  if (btnpressed && (now - lastbouncetime > longclicktime))
  {
  if(clickcount != 0) 
  {
  //do sth for long click
  }
  clickcount=0;
  btnpressed=0;
  }
  
}//btnupdate
