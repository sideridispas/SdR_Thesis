import processing.serial.*;
Serial port;
String dataRaw = "";
String[] dt = {"", "", "", "", "", "","", "", "", ""};
PFont font;

void setup()
{
  size(800, 450);
  port = new Serial(this, "COM4", 9600);
  port.bufferUntil('\n'); 
  font = loadFont("AgencyFB-Bold-200.vlw");
  textFont(font, 60);
}

void draw()
{
  background(0,0,0);
  fill(46, 209, 2);
  textSize(60);
  
  //voltages and current
  text("V1:", 30, 70);
  text(dt[0], 130, 70);
  text("V", 320, 70);
  
  text("V2:", 30, 140);
  text(dt[1], 130, 140);
  text("V", 320, 140);
  
  fill(204, 0, 0);
  text("I:", 30, 210);
  text(dt[2], 130, 210);
  text("A", 320, 210);
  
  fill(255, 180, 20);
  text("P:", 30, 280);
  text(dt[3], 130, 280);
  text("W", 320, 280);
  
  //temperatures
  fill(255, 165, 0);
  text("T1:", 490, 70);
  text(dt[4], 560, 70);
  
  text("T2:", 490, 140);
  text(dt[5], 560, 140);
  
  text("T3:", 490, 210);
  text(dt[6], 560, 210);
  
  text("T4:", 490, 280);
  text(dt[7], 560, 280);
  
  text("T5:", 490, 350);
  text(dt[8], 560, 350);
  
  fill(200, 200, 200);
  textSize(30);
  text("Elapsed time:", 500, 420);
  text(dt[9], 650, 420);

}
  
void serialEvent (Serial port)
{
  dataRaw = port.readStringUntil('\n');
  dataRaw = dataRaw.substring(0, dataRaw.length()-1);
  dt = split(dataRaw, ':');
}