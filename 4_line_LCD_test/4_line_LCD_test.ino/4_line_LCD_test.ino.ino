/*
 
Newhaven NHD-0420D3Z-FL-GBW-V3 LCD
R1 shorted with a 0R resistor
SCL and SDA pullup resistors already on LCD

*/
#include <Wire.h>
#include"4_line_LCD_i2c.h"

#define OFF 0
#define ON 1

int laser_input;
int laser_status=OFF;
unsigned long TimerCurrent=0;
unsigned long TimerPrevious=0;
unsigned long CutTime=0;
char buffer1[20]={0};

void setup() 
{
 
  Wire.begin();
  /*This line is VERY important, the LCD screen has missing 
  characters if the i2c frequency is left default*/
  TWBR = 130;
  delay(1000);

  /*Clear the screen*/
  Display_Clear();
  Set_Cursor(LINE1,0);
  Wire.beginTransmission(I2C_ADDRESS);  
  Wire.write("CUT TIME:");  
  Wire.endTransmission();
  delay(100);

  Set_Cursor(LINE2,0);
  Wire.beginTransmission(I2C_ADDRESS);  
  Wire.write("AMOUNT OWED:");  
  Wire.endTransmission();
  delay(100);
                
}


void loop() 
{
  
  /*Check the status of the laser output and update the status*/
  
  /*Read laser output*/
  laser_input=analogRead(0);
  
  /*If laser tube is off, and we detected a high signal
  wait for debounce delay, and check again to change status*/
  if (laser_status==OFF)
  {
    if (laser_input>=750)
    {
      delay(100);
      laser_input=analogRead(0);
      if (laser_input>=750) laser_status=ON;
    }
  }
  /*If laser tube is ON, and we detected a low signal
  wait for debounce delay, and check again to change status*/
  else
  {
    if (laser_input<=250)
    {
      delay(100);
      laser_input=analogRead(0);
      if (laser_input<=250) laser_status=OFF;
    }
    
  }

  /*Update timer and LCD depending on laser output status*/
  if (laser_status==ON)
  {
    TimerCurrent = millis();
    
    /*Every 60 seconds*/
    if ((TimerCurrent-TimerPrevious)>= 1000) 
    {
      TimerPrevious=TimerCurrent;
      CutTime++;
      itoa(CutTime,buffer1,10);
      
      Set_Cursor(LINE1,10);
  
      Wire.beginTransmission(I2C_ADDRESS);  
      Wire.write(buffer1);  
      Wire.endTransmission();
      delay(100); 
    }
  
 
  }




}



/****************/
/* LCD functions*/
/****************/
void Set_Cursor(unsigned char line, unsigned char column)
{
 Wire.beginTransmission(I2C_ADDRESS);  
 Wire.write(0xFE);         //Prefix 
 Wire.write(0x45);         //Set Cursor
 Wire.write((line)+column);         
 Wire.endTransmission();
 delay(5);              
}


void Display_On(void)
{
 Wire.beginTransmission(I2C_ADDRESS);  
 Wire.write(0xFE);           //Prefix 
 Wire.write(DISPLAY_ON);     //Display On
 Wire.endTransmission();
 delay(5);                 
}

void Display_Clear(void)
{
 Wire.beginTransmission(I2C_ADDRESS);  
 Wire.write(0xFE);           //Prefix 
 Wire.write(CLEAR_SCREEN);   //Clear Display
 Wire.endTransmission();
 delay(5);                 
}


void Cursor_Home(void)
{
 Wire.beginTransmission(I2C_ADDRESS);  
 Wire.write(0xFE);           //Prefix 
 Wire.write(CURSOR_HOME);     //Cursor Home
 Wire.endTransmission();
 delay(5);                 
}


