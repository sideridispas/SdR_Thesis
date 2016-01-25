#include "IC_Libs/ads12xx.h"

long data, m_data;
float f_data;
ads12xx ads1256(7,2); //CS:7, DRDY:2
int visual = 1; //processing app:1 - arduino serial monitor:0

void reg_init(){
  unsigned long reg;
  
  /***** REGISTER INITIALISATION ******/
  //STATUS register (default: 48)
  reg = ads1256.GetRegisterValue(STATUS);
  //reg = reg | B00000010; //BUFEN:1
  reg = reg & B11110001; //ORDER, ACAL:0
  ads1256.SetRegisterValue(STATUS,reg);
  
  //ADCON register (default: 32)
  ads1256.SetRegisterValue(ADCON,B00100111); //PGA:64
  
  //DRATE register
  ads1256.SetRegisterValue(DRATE,B01100011); //01100011
 
  //Registers' printing
/*  for(int i=0;i<5;i++){
    reg = ads1256.GetRegisterValue(i);
    Serial.print("Reg 0x0");Serial.print(i);Serial.print(": ");
    Serial.println(reg);
  }*/
}

void setup() {
  Serial.begin(9600);
  ads1256.begin();
  reg_init();  //register initialisation
  delay(100);
  ads1256.SendCMD(SELFCAL); //self-calibration command
  delay(100);
}

void loop() {

  f_data = 0;
  data = 0;
  
  /************** VOLTAGE 1 MEASUREMENTS **************/
  ads1256.SetRegisterValue(MUX,B00100011); //AIN2 - AIN3
  delay(50);
  delayMicroseconds(10);
  ads1256.SendCMD(SYNC);
  delayMicroseconds(10);
  ads1256.SendCMD(WAKEUP);
  
  data =  ads1256.GetConversion();
  data = constrain(data, 0, 16777215);
  
  if((data >= 0) && (data <= 8388607)){
    //positive
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data+0.1386)/1.4742;
  }
  else if((data > 8388607) && (data <= 16777215)){
    //negative
    bitClear(data, 23);
    data = 8388607 - data;
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data+0.1386)/1.4742;
    f_data = -1*f_data;
  }

  if(visual){
    Serial.print(f_data,4);
    Serial.print(":");
  }
  else{
    Serial.print("V1:");
    Serial.print(f_data,4);
    Serial.print(" | ");
  }
  
  /************** VOLTAGE 2 MEASUREMENTS **************/
  ads1256.SetRegisterValue(MUX,B01000101); //AIN4 - AIN5
  delay(50);
  delayMicroseconds(10);
  ads1256.SendCMD(SYNC);
  delayMicroseconds(10);
  ads1256.SendCMD(WAKEUP);
  
  data =  ads1256.GetConversion();
  data = constrain(data, 0, 16777215);
  
  if((data >= 0) && (data <= 8388607)){
    //positive
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data+0.1161)/1.4968;
  }
  else if((data > 8388607) && (data <= 16777215)){
    //negative
    bitClear(data, 23);
    data = 8388607 - data;
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data+0.1161)/1.4968;
    f_data = -1*f_data;
  }
  
   if(visual){
      Serial.print(f_data,4);
      Serial.print(":");
    }
    else{
      Serial.print("V2:");
      Serial.print(f_data,4);
      Serial.print(" | ");
    }

  /************** CURRENT MEASUREMENTS **************/
  ads1256.SetRegisterValue(MUX,B00010000); //AIN0 - AIN1
  delay(50);
  delayMicroseconds(10);
  ads1256.SendCMD(SYNC);
  delayMicroseconds(10);
  ads1256.SendCMD(WAKEUP);
  
  data =  ads1256.GetConversion();
  data = constrain(data, 0, 16777215);
  
  if((data >= 0) && (data <= 8388607)){
    //positive
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data-0.321)/4.9331;
  }
  else if((data > 8388607) && (data <= 16777215)){
    //negative
    bitClear(data, 23);
    data = 8388607 - data;
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data-0.321)/4.9331;
    f_data = -1*f_data;
  }
  if(visual){
    Serial.println(f_data,4);
  }
  else{
    Serial.print("I:");
    Serial.println(f_data,4);
  }
  
  //delay(500);
}





