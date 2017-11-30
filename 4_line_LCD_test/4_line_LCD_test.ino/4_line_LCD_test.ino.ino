/*
 
Newhaven NHD-0420D3Z-FL-GBW-V3 LCD
R1 shorted with a 0R resistor
SCL and SDA pullup resistors already on LCD

*/
#include <Wire.h>
#include"4_line_LCD_i2c.h"

void setup() 
{
 
  Wire.begin();
  /*This line is VERY important, the LCD screen has missing 
  characters if the i2c frequency is left default*/
  TWBR = 130;
  delay(1000);

  /*Clear the screen*/
  Display_Clear();
  
  /*Set cursor to origin*/
  Set_Cursor(LINE1,0);
  
  Wire.beginTransmission(I2C_ADDRESS);  
  Wire.write("Hello World!");  //Display Text
  Wire.endTransmission();
  delay(1000);               
}


void loop() 
{
 
}

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


