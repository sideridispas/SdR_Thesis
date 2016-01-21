//Program by Jeremy Blum
//www.jeremyblum.com
//Give you the temperature

import processing.serial.*;
Serial port;
String dataRaw = "", dt ="";
String[] data = new String[2];
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
  text(dt, 70, 140);
  text("uV", 230, 140);
}
  
void serialEvent (Serial port)
{
  dataRaw = port.readStringUntil('\n');
  data[0] = dataRaw.substring(0, dataRaw.length() - 3);
  data[1] = dataRaw.substring(dataRaw.length() - 3, dataRaw.length()-2);
  
  dt = join(data, ",");
}