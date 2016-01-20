import processing.core.*; 
import processing.data.*; 
import processing.event.*; 
import processing.opengl.*; 

import processing.serial.*; 

import java.util.HashMap; 
import java.util.ArrayList; 
import java.io.File; 
import java.io.BufferedReader; 
import java.io.PrintWriter; 
import java.io.InputStream; 
import java.io.OutputStream; 
import java.io.IOException; 

public class display_temp extends PApplet {

//Program by Jeremy Blum
//www.jeremyblum.com
//Give you the temperature


Serial port;
String data = "";
int index = 0;
PFont font;

public void setup()
{
  
  port = new Serial(this, "COM4", 9600);
  port.bufferUntil('\n'); 
  font = loadFont("AgencyFB-Bold-200.vlw");
  textFont(font, 100);
}

public void draw()
{
  background(0,0,0);
  fill(46, 209, 2);
  text(data, 50, 140);
}
  
public void serialEvent (Serial port)
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
  public void settings() {  size(400,200); }
  static public void main(String[] passedArgs) {
    String[] appletArgs = new String[] { "display_temp" };
    if (passedArgs != null) {
      PApplet.main(concat(appletArgs, passedArgs));
    } else {
      PApplet.main(appletArgs);
    }
  }
}
