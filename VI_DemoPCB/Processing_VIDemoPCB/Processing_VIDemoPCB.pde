//Program by Jeremy Blum
//www.jeremyblum.com
//Give you the temperature

import processing.serial.*;
Serial port;
String dataRaw = "";
String[] dt = {"", "", ""};
PFont font;

void setup()
{
  size(550,350);
  port = new Serial(this, "COM4", 9600);
  port.bufferUntil('\n'); 
  font = loadFont("AgencyFB-Bold-200.vlw");
  textFont(font, 80);
}

void draw()
{
  background(0,0,0);
  fill(46, 209, 2);
  text("V1:", 65, 100);
  text(dt[0], 170, 100);
  text("V", 420, 100);
  
  text("V2:", 50, 200);
  text(dt[1], 170, 200);
  text("V", 420, 200);
  
  fill(204, 0, 0);
  text("I:", 100, 300);
  text(dt[2], 170, 300);
  text("A", 420, 300);
}
  
void serialEvent (Serial port)
{
  dataRaw = port.readStringUntil('\n');
  dataRaw = dataRaw.substring(0, dataRaw.length()-1);
  dt = split(dataRaw, ':');
}