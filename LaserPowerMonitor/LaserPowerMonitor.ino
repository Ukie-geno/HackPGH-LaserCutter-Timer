// Modified original i3 detroit code:
// replaced lcd code with i2c code for 4 line LCD
// removed most neopixels code
// removed most commented out code. 
// 11/29/17
// Geno Soroka


#include <EEPROM.h>
#include "EEPROMAnything.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <stdio.h>
#include <Adafruit_NeoPixel.h>
#include"4_line_LCD_i2c.h"

//Pin Definitions
int userResetPin = 7;		// pin used for resetting user laser time	(Nano D7)
int analogPin = 0;			// input voltage connected to analog pin 0 from interface board
int pixelPin = 6;			// Neopixel data pin 
//analog input is on pin A0.
//serial communication for LCD is on pins A4 & A5


// Display config
#define MAX_OUT_CHARS 17  //max nbr of characters to be sent on any one serial command,
// plus the terminating character sprintf needs to prevent it from overwriting following
//memory locations, yet still send the complete 16 character string to the LCD

// Setup for the NXP PCF8574A I2C-to-Digital I/O Port
#define I2C_ADDR    0x27  // Define PCF8574A's I2C Address

// Parameters to define the size of the LCD array
#define lineLen 16
#define numLines 2
// LCD backlight state
#define LED_ON  1
#define LED_OFF  0



// Initialize neopixels
int numPixels = 3;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numPixels, pixelPin, NEO_GRB + NEO_KHZ800);
int errorLED = 2;
int laserOnLED = 1;
int activeSessionLED = 0;
uint32_t red = pixels.Color(200,0,0);
uint32_t blue = pixels.Color(0,0,200);
uint32_t green = pixels.Color(0,200,0);
uint32_t off = pixels.Color(0,0,0);

// Time constants
const unsigned long second = 1000;
const unsigned long minute = 60000;		// number of millis in a minute
const unsigned long hour = 3600000;		// number of millis in an hour

// Kept threshold values from DMS timer but swapped the logic in loop()
// because our laser uses active low instead of active high
int analogVal = 0;			// variable to store the analog value read
int anaLowThreshold = 300;	 // if analog goes above this value its considered ON
int anaHighThreshold = 324;	// if analog value goes below this value its considered OFF
int cursorPos = 0;
//Delay after seeing laser is on
int minLaserTime = 500; //effectively the shortest time the laser can be recorded as being active.

unsigned long millisOnLast = 0;
unsigned long millisOffLast = 0;
unsigned long millisTemp = 0;
unsigned long millisDiff = 0;
boolean lastLaserOn = false;
unsigned long userMillis = 0;
int userHours = 0;
int userMinutes = 0;				// number of minutes user has used the laser (resettable when button pressed)
int userSeconds = 0;
int tubeHours = 0;
int tubeMinutes = 0;				// number of minutes tube has been used (not resettable)
int tubeSeconds = 0;
unsigned long tubeMillis = 0;				
unsigned long lastWriteToEEPROMMillis = 0;	 // number of millis that the EEPROM was last written to

char   buffer[MAX_OUT_CHARS];  //buffer used to format a line (+1 is for trailing 0)
char   buffer2[MAX_OUT_CHARS];  //buffer used to format a line (+1 is for trailing 0)
char   costbuffer[10];			//buffer used to hold user cost as a string

float costPerMin = 0.20;

const unsigned int ThisCurrentVersion = 4;	// version number for this program.	simply counting releases
// did not increment version when forking for I2C LCD


struct config_t
{
	unsigned long seconds;				// tube seconds
	unsigned long uSeconds;				// user seconds
	unsigned long EEPROMwriteCount;		// EEPROM write cycle count
	unsigned int thisVersion;			// version number of this software
} laserTime;

void setup() {
	int addr = 0;
	pinMode(userResetPin, INPUT);
	
	EEPROM_readAnything(addr,laserTime);
	Serial.begin(115200);
	tubeMillis = laserTime.seconds*1000;
	userMillis = laserTime.uSeconds*1000;
		// Initialize the version number in EEPROM if this is the first load after a reflash
	if ( laserTime.thisVersion == 0 )
	{
		laserTime.thisVersion = ThisCurrentVersion;
		laserTime.EEPROMwriteCount = laserTime.EEPROMwriteCount + 1;
		EEPROM_writeAnything(0, laserTime);
	}

 
		
	// Briefly show Arduino status
	sprintf(buffer, "Version: %02d", laserTime.thisVersion);
	sprintf(buffer2, "Writes: %06d", laserTime.EEPROMwriteCount);

  /*Initialize LCD i2c*/
  Wire.begin();
  /*This line is VERY important, the LCD screen has missing 
  characters if the i2c frequency is left default*/
  TWBR = 130;
	
	
	/*Clear the screen*/
  Display_Clear();
  /*Set cursor to origin*/
  Set_Cursor(LINE1,0);
  Wire.beginTransmission(I2C_ADDRESS);  
  Wire.write(buffer);  //Display Text
  Wire.endTransmission();
	Set_Cursor(LINE2,0);
  Wire.beginTransmission(I2C_ADDRESS);  
  Wire.write(buffer2);  //Display Text
  Wire.endTransmission();
	
	delay(2000);		

  Set_Cursor(LINE1,0);
  Wire.beginTransmission(I2C_ADDRESS);  
  Wire.write(" LaserTimer 1.0 ");  //Display Text
  Wire.endTransmission();
  Set_Cursor(LINE2,0);
  Wire.beginTransmission(I2C_ADDRESS);  
  Wire.write("   Loading...   ");  //Display Text
  Wire.endTransmission();

  delay(2000);

	Serial.print("Values stored in EEPROM address ");
	Serial.println(addr);


	Serial.print("	laserTime.seconds ie tube: ");
	Serial.println(laserTime.seconds);
	
	Serial.print("	laserTime.uSeconds ie user: ");
	Serial.println(laserTime.uSeconds);
	
	Serial.print("	laserTime.EEPROMwriteCount: ");
	Serial.println(laserTime.EEPROMwriteCount);

	Serial.print("	laserTime.thisVersion: ");
	Serial.println(laserTime.thisVersion);

	Serial.println("setup Complete");
	Serial.println("");
	
	
}

void loop() {
	int addr = 0;
	//debug output to test loop timing
	
	// do a tight loop on checking the laser and keeping track of on/off times	
	for (int i=0; i <= 100; i++) {
		
		// read the input pin
		analogVal = analogRead(analogPin);
		
			// laser has been off, laser turning on here
		if ((analogVal <	anaLowThreshold) && !lastLaserOn) {
			lastLaserOn = true;
			millisOnLast = (unsigned long) millis();
			millisDiff = millisOnLast - millisOffLast;
			
			delay(minLaserTime);
		}
			// laser has been on here, continuing on
		else if ((analogVal <	anaLowThreshold) && lastLaserOn) {
			lastLaserOn = true;

			millisTemp = (unsigned long) millis();
			millisDiff = millisTemp-millisOnLast;
			millisOnLast = millisTemp;
			delay(minLaserTime);
		}
			// laser has been on, turning off
		else if ((analogVal > anaHighThreshold) && lastLaserOn) {
			lastLaserOn = false;
			millisOffLast = (unsigned long) millis();
			
		}
			// laser has been off, staying off
		else {
			lastLaserOn = false;
			millisOffLast = (unsigned long) millis();
		}
			
		int userReset = digitalRead(userResetPin);
			//User presses button when laser isn't firing
		if ((userReset == LOW) && !lastLaserOn && userMillis > 0) {
				//	allow reset and writing once every 10 seconds, but no faster
				//	write values to EPROM every time user hits reset
			if (millis() > (lastWriteToEEPROMMillis+10000)) {
				userMillis = 0;
				laserTime.seconds = tubeMillis/1000;
				laserTime.uSeconds = userMillis/1000;
				laserTime.EEPROMwriteCount = laserTime.EEPROMwriteCount + 1;
				laserTime.thisVersion = ThisCurrentVersion;
				//int addr = ROUND_ROBIN_EEPROM_write(laserTime);
				EEPROM_writeAnything(0, laserTime);

				lastWriteToEEPROMMillis = millis();
				
				Serial.println("User hit reset & Wrote to EEPROM");

				Serial.print("	EEPROM address: ");
				Serial.println(addr);

				Serial.print("	laserTime.seconds ie tube: ");
				Serial.println(laserTime.seconds);

				Serial.print("	laserTime.uSeconds ie user: ");
				Serial.println(laserTime.uSeconds);
		
				Serial.print("	laserTime.EEPROMwriteCount: ");
				Serial.println(laserTime.EEPROMwriteCount);
			
				Serial.print("	laserTime.thisVersion: ");
				Serial.println(laserTime.thisVersion);
			}
				// display laser cost on screen for 5 sec
				//Create screen output
			sprintf(buffer,  "User    %02d:%02d:%02d", userHours,  userMinutes, userSeconds);
			float userCost = (userSeconds/60.0 + userMinutes + userHours*60.0)*costPerMin;
			
			dtostrf(userCost,7,2,costbuffer);
			//sprintf(buffer2, "Cost    $%s", costbuffer);
			sprintf(buffer2, "Cost        $TBD", costbuffer);

     Set_Cursor(LINE1,0);
     Wire.beginTransmission(I2C_ADDRESS);  
     Wire.write(buffer);  //Display Text
     Wire.endTransmission();
     Set_Cursor(LINE2,0);
     Wire.beginTransmission(I2C_ADDRESS);  
     Wire.write(buffer2);  //Display Text
     Wire.endTransmission();
  
     delay(5000);  
			
		}
			
			
		userMillis = userMillis + millisDiff;
		tubeMillis = tubeMillis + millisDiff;
		millisDiff = 0;
			
	}

	tubeHours = tubeMillis/hour;
	tubeMinutes = (tubeMillis-tubeHours*hour)/minute;
	tubeSeconds = (tubeMillis-tubeHours*hour-tubeMinutes*minute)/second;
	userHours = userMillis/hour;
	userMinutes = (userMillis-userHours*hour)/minute;
	userSeconds = (userMillis-userHours*hour-userMinutes*minute)/second;
	
	//Create screen output
	sprintf(buffer,  "User    %02d:%02d:%02d", userHours,  userMinutes, userSeconds);
	//sprintf(buffer, "%12d", userMillis); //show raw milliseconds for debugging
	sprintf(buffer2, "Tube %05d:%02d:%02d", tubeHours,  tubeMinutes, tubeSeconds);
	
		// Only write to EEPROM if the current value is more than 5 minutes from the previous EEPROM value
		// to reduce the number of writes to EEPROM, since any one location is only good for ~ 100,000 writes
	EEPROM_readAnything(addr, laserTime);
	//int addr = ROUND_ROBIN_EEPROM_read(laserTime);
	unsigned long laserSeconds = laserTime.seconds;
	
		// note - it appears that only one of the following If statements is required	
	if ((laserSeconds+300) < (tubeMillis/1000)) {	
		Serial.print("LaserSeconds:");
		Serial.print(laserSeconds);
		Serial.print("adjTubeSecs:");
		Serial.println(((tubeMillis/1000)+300));
		laserTime.seconds = tubeMillis/1000;
		laserTime.uSeconds = userMillis/1000;
		laserTime.EEPROMwriteCount = laserTime.EEPROMwriteCount + 1;
		laserTime.thisVersion = ThisCurrentVersion;
		EEPROM_writeAnything(0, laserTime);
		//addr = ROUND_ROBIN_EEPROM_write(laserTime);
		lastWriteToEEPROMMillis = millis();
		Serial.println("Wrote to EEPROM - tube has another 5 minutes of use");
		
		Serial.print("	EEPROM address: ");
		Serial.println(addr);

		Serial.print("	laserTime.seconds ie tube: ");
		Serial.println(laserTime.seconds);
		
		Serial.print("	laserTime.uSeconds ie user: ");
		Serial.println(laserTime.uSeconds);
		
		Serial.print("	laserTime.EEPROMwriteCount: ");
		Serial.println(laserTime.EEPROMwriteCount);
		 
		Serial.print("	laserTime.thisVersion: ");
		Serial.println(laserTime.thisVersion);
	 }	
   //Print user and tube times to screen
  Set_Cursor(LINE1,0);
  Wire.beginTransmission(I2C_ADDRESS);  
  Wire.write(buffer);  //Display Text
  Wire.endTransmission();
  Set_Cursor(LINE2,0);
  Wire.beginTransmission(I2C_ADDRESS);  
  Wire.write(buffer2);  //Display Text
  Wire.endTransmission();
		
	
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

