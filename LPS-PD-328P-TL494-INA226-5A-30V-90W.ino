#include <Arduino.h>
#include <GyverOLED.h>
#include <GyverINA.h>

//GyverOLED<SSD1306_128x64> oled; // 0.96"
GyverOLED<SSH1106_128x64> oled;   // 1.3" 
INA226 ina(0.00497f, 15.0f, 0x40); //(0.005R, Imax 15A, ADR)  

#define MaxAdcCNT 8
int DIV_IN = 266;
int DIV_PWM_V = 134;  
int DIV_PWM_C = 465;

int PWM_V = 1606; // Start 12 V
int PWM_C = 1050; // Start  2 A
int PWM_C_MIN = 116; 

float V_IN, OutV, OutC, OutP, ADC_IN, SetI, SetU = 0;
byte Mode, OutST, AdcCicles = 0; 
byte Key_UP_CNT, Key_SEL_CNT, Key_DOWN_CNT, Key_OUT_CNT = 0;

#define Key_UP    13 // !! remove led on board arduino nano !!
#define Key_SEL   11
#define Key_DOWN  12  
#define Key_OUT   7 

#define OUT_LED   8

#define SET_U  1
#define SET_I  2
//******************************************************************

void PrinVal()
{  
  oled.clear(); 
  oled.setScale(1);
   
  oled.setCursorXY(0, 0);
  oled.print("IN ");
  oled.print(V_IN,1);
  oled.print("V");

  SetU = PWM_V;
  SetU = SetU/DIV_PWM_V;
  
  oled.setCursorXY(0, 40);
  oled.print("U ");
  oled.print(SetU, 1);
  oled.print(" V");

  SetI = PWM_C - PWM_C_MIN;
  SetI = SetI/DIV_PWM_C;
  
  oled.setCursorXY(0, 54);
  oled.print("I ");
  oled.print(SetI, 2);
  oled.print(" A");


  if(Mode == SET_U)
  {
    oled.setCursorXY(0,12);
    oled.print("SET U  ");  
    oled.setCursorXY(0,24);
    oled.print(PWM_V);  
    } 

  if(Mode == SET_I)
  {
    oled.setCursorXY(0,12);
    oled.print("SET I  ");  
    oled.setCursorXY(0,24);
    oled.print(PWM_C);      
    }       
  
  oled.setScale(2);
 
  oled.setCursorXY(64, 0);
  if(OutV < 10) oled.print(OutV,2);
  else oled.print(OutV,1);
  oled.setCursorXY(116, 0);
  oled.print("V");
  
  oled.setCursorXY(64, 24);
  if(OutC < 10)oled.print(OutC,2);
  else oled.print(OutC,1);
  oled.setCursorXY(116, 24);
  oled.print("A");
  
  oled.setCursorXY(64, 48);
  oled.print(OutP,1);
  oled.setCursorXY(116, 48);
  oled.print("W");  
  
  oled.update(); 
  }

//*******************************************************************

void setup() 
{
  TCCR1A = 0;  
  TCCR1B = 0; 
  
  TCCR1A |= (1 << WGM11);  
  TCCR1B |= (1 << WGM12) | (1 << WGM13); 
  TCCR1B |= (1 << CS10);
  TCCR1A |= (1 << COM1A1) | (1 << COM1B1);
  
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);

  ICR1 = 0xFFF; // Max CNT 
  OCR1A = 0;  // SET_U // 9 pin PWM	
  OCR1B = 0;  // SET_I // 10 pin PWM 
 
  pinMode(Key_UP, INPUT_PULLUP);
  pinMode(Key_SEL, INPUT_PULLUP);
  pinMode(Key_DOWN, INPUT_PULLUP);
  pinMode(Key_OUT, INPUT_PULLUP);
  pinMode(OUT_LED, OUTPUT);  
  digitalWrite(OUT_LED,LOW); // LOW  // HIGH

  Mode = 0;

  delay(300); 
  
  ina.begin();  
  
  ina.setSampleTime(INA226_VBUS, INA226_CONV_2116US);   
  ina.setSampleTime(INA226_VSHUNT, INA226_CONV_8244US); 
  ina.setAveraging(INA226_AVG_X4);   
  
  oled.init();   
  PrinVal();

}


void loop() 
{
  //--------------------------------------------------------------------


  if (digitalRead(Key_UP) == 0)
  {
    if(Key_UP_CNT < 200)Key_UP_CNT++;
    if(Key_UP_CNT > 20) 
    {
      if(Mode == SET_U)
      {
        if(PWM_V < 4092)PWM_V = PWM_V + 2;
        OCR1A = PWM_V; // SET_U 
      }
      
      if(Mode == SET_I)
      {
        if(PWM_C < 4092)PWM_C = PWM_C + 2;
        OCR1B = PWM_C; // SET_I 
      }        
    }      
  }
  else 
  {
    if(Key_UP_CNT > 5) 
    {
      if(Mode == SET_U)
      {
        if(PWM_V < 4094)PWM_V++;
        OCR1A = PWM_V; // SET_U 
      }
      
      if(Mode == SET_I)
      {
        if(PWM_C < 4094)PWM_C++;
        OCR1B = PWM_C;; // SET_I 
      }     
    }
    
    Key_UP_CNT = 0;
  }
  
  //********************************************************************
  
  if (digitalRead(Key_DOWN) == 0)
  {
    if(Key_DOWN_CNT < 200) Key_DOWN_CNT++;
    if(Key_DOWN_CNT > 20) 
    {
      if(Mode == SET_U)
      {
        if(PWM_V > 2)PWM_V = PWM_V - 2;
        OCR1A = PWM_V; // SET_U 
      }
      
      if(Mode == SET_I)
      {
        if(PWM_C > PWM_C_MIN + 2)PWM_C = PWM_C - 2;
        OCR1B = PWM_C;; // SET_I 
      }
    }     
  }
  else 
  {
    if(Key_DOWN_CNT > 5) 
    {
      if(Mode == SET_U)
      {
        if(PWM_V > 0)PWM_V--;
        OCR1A = PWM_V; // SET_U 
      }
      
      if(Mode == SET_I)
      {
        if(PWM_C > PWM_C_MIN)PWM_C--;
        OCR1B = PWM_C;; // SET_I 
      }    
    }
    
    Key_DOWN_CNT = 0;
  }  
    
  //--------------------------------------------------------------------

  if (digitalRead(Key_OUT) == 0)
  {
    if(Key_OUT_CNT < 200)Key_OUT_CNT++;
    
    if(Key_OUT_CNT == 5)
    {
      if(OutST == 0)
      {
        OutST = 1;
        digitalWrite(OUT_LED,HIGH); 
        OCR1B = PWM_C; // SET_I  
        OCR1A = PWM_V; // SET_U 
       }
      else
      {
        OutST = 0;
        digitalWrite(OUT_LED,LOW);
        OCR1B = 0;  // SET_I  
        OCR1A = 0;  // SET_U 
       }
     } 
  }
  else Key_OUT_CNT = 0;     


  //--------------------------------------------------------------------

  if (digitalRead(Key_SEL) == 0)
  {
    if(Key_SEL_CNT < 200)Key_SEL_CNT++;
    
    if(Key_SEL_CNT == 10)
    {
      Mode++;
      if(Mode > 2) Mode = 0;
     } 
  }
  else Key_SEL_CNT = 0;     

  
  //---------------------------------------------------------------------
  
  if (AdcCicles < MaxAdcCNT) 
  {
    ADC_IN = ADC_IN + analogRead(A7);
    AdcCicles++;
    }  
  else
  {
    V_IN = ADC_IN/DIV_IN;
    OutV = ina.getVoltage();
    OutC = ina.getCurrent();
    if(OutC < 0) OutC = OutC * -1;
    OutP = OutV * OutC;
     
    PrinVal();        

    ADC_IN = 0;
    AdcCicles = 0; 
  }
    
//---------------------------------------------------------------------- 
}
