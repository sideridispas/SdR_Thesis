//Program by Jeremy Blum
//www.jeremyblum.com
//Give you the temperature

import processing.serial.*;
Serial port;
String dataRaw = "", dt ="";
PFont font;

void setup()
{
  size(550,200);
  port = new Serial(this, "COM4", 9600);
  port.bufferUntil('\n'); 
  font = loadFont("AgencyFB-Bold-200.vlw");
  textFont(font, 100);
}

void draw()
{
  background(0,0,0);
  fill(46, 209, 2);
  text(dt, 50, 140);
  text("mV", 400, 140);
}
  
void serialEvent (Serial port)
{
  dataRaw = port.readStringUntil('\n');
  dt = dataRaw.substring(0, dataRaw.length() - 1);
}