import processing.serial.*;
Serial port;
String dataRaw = "";
PFont font;
String[] dt = {"", "", "", "", "", "", "", "", "", "", "", ""};

void setup()
{
  size(800, 600);
  port = new Serial(this, "COM5", 9600);
  port.bufferUntil('\n'); 
  font = loadFont("AgencyFB-Bold-200.vlw");
  textFont(font, 60);
}

void draw()
{
  background(0,0,0);
    
  //title
  fill(200, 200, 200);
  textAlign(CENTER);
  textSize(40);
  text("PV Measurements - Demo",400,50);
  textSize(20);
  text("Sideridis Paschalis - Feb2016",400,80);
  stroke(153);
  line(100, 55, width-100, 55);
  
  //voltages and current
  textAlign(LEFT);
  fill(46, 209, 2);
  textSize(60);
  text("V1:", 30, 220);
  text(dt[0], 130, 220);
  text("V", 320, 220);
  
  text("V2:", 30, 290);
  text(dt[1], 130, 290);
  text("V", 320, 290);
  
  fill(204, 0, 0);
  text("I:", 30, 360);
  text(dt[2], 130, 360);
  text("A", 320, 360);
  
  fill(255, 180, 20);
  text("P:", 30, 430);
  text(dt[4], 130, 430);
  text("W", 320, 430);
  
  //temperatures
  fill(255, 165, 0);
  text("T1:", 490, 220);
  text(dt[7], 560, 220);
  
  text("T2:", 490, 290);
  text(dt[8], 560, 290);
  
  text("T3:", 490, 360);
  text(dt[9], 560, 360);
  
  text("T4:", 490, 430);
  text(dt[10], 560, 430);
  
  text("T5:", 490, 500);
  text(dt[11], 560, 500);
  
  fill(200, 200, 200);
  textSize(30);
  text("Timestamp:", 50, 570);
  text(dt[5], 180, 570);

}
  
void serialEvent (Serial port)
{
  dataRaw = port.readStringUntil('\n');
  dataRaw = dataRaw.substring(0, dataRaw.length()-1);
  dt = split(dataRaw, ',');
}