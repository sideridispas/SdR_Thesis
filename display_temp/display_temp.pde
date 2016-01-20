//Program by Jeremy Blum
//www.jeremyblum.com
//Give you the temperature

import processing.serial.*;
Serial port;
String data = "";
int index = 0;
PFont font;

void setup()
{
  size(400,200);
  port = new Serial(this, "COM4", 9600);
  port.bufferUntil('\n'); 
  font = loadFont("AgencyFB-Bold-200.vlw");
  textFont(font, 100);
}

void draw()
{
  background(0,0,0);
  fill(46, 209, 2);
  text(data, 50, 140);
}
  
void serialEvent (Serial port)
{
  data = port.readStringUntil('\n');
  /*data = data.substring(0, data.length() - 1);
  
  // look for the comma between Celcius and Farenheit
  index = data.indexOf(",");
  // fetch the C Temp
  temp_c = data.substring(0, index);
  // fetch the F Temp
  temp_f = data.substring(index+1, data.length());*/
}