#include "IC_Libs/ads12xx.h"

long data, m_data;
float f_data;
ads12xx ads1256(7,2); //CS:7, DRDY:2

void reg_init(){
  unsigned long reg;
  
  /***** REGISTER INITIALISATION ******/
  //STATUS register (default: 48)
  reg = ads1256.GetRegisterValue(STATUS);
  //reg = reg | B00000010; //BUFEN:1
  reg = reg & B11110001; //ORDER, ACAL:0
  ads1256.SetRegisterValue(STATUS,reg);
  
  //MUX register
  ads1256.SetRegisterValue(MUX,B01000101); //AIN4 - AIN5
  
  //ADCON register (default: 32)
  ads1256.SetRegisterValue(ADCON,B00100110); //PGA:64
  
  //DRATE register
  ads1256.SetRegisterValue(DRATE,B00000011);
 
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
}

void loop() {
 
  ads1256.SetRegisterValue(MUX,B01000101); //AIN4 - AIN5
  delayMicroseconds(5);
  ads1256.SendCMD(SYNC);
  delayMicroseconds(5);
  ads1256.SendCMD(WAKEUP);
  
  data =  ads1256.GetConversion();
  data = constrain(data, 0, 16777215);
  
  if((data >= 0) && (data <= 8388607)){
    //positive
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data-1.0678)/0.8237;
  }
  else if((data > 8388607) && (data <= 16777215)){
    //negative
    /*bitClear(data, 23);
    m_data = map(data, 0, 8388607, -255, -1);
    m_data = map(m_data, -255, -1, -1, -78);*/
    Serial.println("-");
  }
  
  //Serial.print("Converted data: ");  
  Serial.println(f_data,4);
  //delay(500);
}





