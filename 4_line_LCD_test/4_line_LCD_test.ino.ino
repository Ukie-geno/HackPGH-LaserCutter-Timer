/*
 
Newhaven NHD-0420D3Z-FL-GBW-V3 LCD
R1 shorted with a 0R resistor
SCL and SDA pullup resistors already on LCD

*/
#include <Wire.h>

#define I2C_ADDRESS        0x28  //LCD I2C address  0x28??? It should be 0x50 
#define DISPLAY_ON         0x41  //DISPLAY ON
#define DISPLAY_OFF        0x42  //DISPLAY OFF
#define SET_CURSOR         0x45  //SET CURSOR
#define CURSOR_HOME        0x46  //CURSOR HOME
#define MOVE_CURSOR_LEFT   0x49  //MOVE CURSOR LEFT
#define MOVE_CURSOR_RIGHT  0x4A  //MOVE CURSOR RIGHT
#define BLINK_CURSOR_ON    0x4B  //BLINK CURSOR ON
#define BLINK_CURSOR_OFF   0x4C  //BLINK CURSOR OFF
#define BACKSPACE          0x4E  //BACKSPACE
#define CLEAR_SCREEN       0x51  //CLEAR SCREEN
#define SET_CONTRAST       0x52  //SET CONTRAST
#define SET_BACKLIGHT      0x53  //SET BACKLIGHT
#define MOVE_DISP_LEFT     0x55  //MOVE DISPLAY LEFT
#define MOVE_DISP_RIGHT    0x56  //MOVE DISPLAY RIGHT
#define DISP_I2C_ADDR      0x72  //DISPLAY I2C ADDRESS

#define LINE1 0x00
#define LINE2 0x40
#define LINE3 0x14
#define LINE4 0x54

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


