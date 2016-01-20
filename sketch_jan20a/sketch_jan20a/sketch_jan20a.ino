#include <ads12xx.h>

long data, m_data;
ads12xx ads1256(7,2); //CS:7, DRDY:2

void setup() {
  Serial.begin(9600);
  //Serial.println("======= IN SETUP - start =======");
  ads1256.begin();
  reg_init();  //register initialisation
  //Serial.println("======= IN SETUP - end =======\n");
}

void loop() {
 
  ads1256.SetRegisterValue(MUX,103); //MUX register changed so AINP=AIN6 & AINN=AIN7
  delayMicroseconds(10);
  ads1256.SendCMD(SYNC);
  delayMicroseconds(10);
  ads1256.SendCMD(WAKEUP);
  
  data =  ads1256.GetConversion();

  data = constrain(data, 0, 16777215);
  
  if((data >= 0) && (data <= 8388607)){
    //positive
    m_data = map(data, 0, 8388607, 0, 250);
    m_data = m_data * 20;
    
  }
  else if((data > 8388607) && (data <= 16777215)){
    //negative
    bitClear(data, 23);
    m_data = map(data, 0, 8388607, -250, -1);
    m_data = m_data * 20;
  }
  
  //Serial.print("Converted data: ");  
  Serial.println(m_data);
  delay(500);
}

void reg_init(){
  unsigned long reg;
  
  /***** REGISTER INITIALISATION ******/
  //STATUS register -> BUFEN changed to 1 (default: 48)
  ads1256.SetRegisterValue(STATUS,50); 
  delayMicroseconds(10);
  
  //MUX register changed to 0110 0111 (dec:103) so AINP=AIN6 & AINN=AIN7
  ads1256.SetRegisterValue(MUX,103);
  delayMicroseconds(10);

  //ADCON register -> Clock Out OFF (default: 32)
  ads1256.SetRegisterValue(ADCON,32); 
  delayMicroseconds(10);
  
  //DRATE register changed to 1011 0000 (dec:176) so 2.000 sps
  ads1256.SetRegisterValue(DRATE,130); 
  delayMicroseconds(10);
  
  //Registers' printing
/*  for(int i=0;i<5;i++){
    reg = ads1256.GetRegisterValue(i);
    Serial.print("Reg 0x0");Serial.print(i);Serial.print(": ");
    Serial.println(reg);
  }*/

}



