/*
 Name:		Solar_Tracker.ino
 Created:	5/1/2020 5:32:21 PM
 Author:	swefd
*/
//#include <Wire.h>
//#include <Time.h>
//#include <SPI.h>

#include <Wire.h>
#include "SDL_Arduino_INA3221.h"
#include <DS3232RTC.h>
#include <Servo.h>
#include <SolarPosition.h>
#include "GyverTimer.h"
#include "GyverEncoder.h"
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include "LcdMenu.h"



//Encoder settings
#define SW 4     
#define DT 3
#define CLK 2
Encoder enc1(CLK, DT, SW);  //Encoder object

//LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

//Custom chars for LCD
byte Batery_stat[][8] = { 

	//BAT1 1
	{B11111,B10000,B10000,B10000,B10000,B10000,B10000,B11111}, //bat 1 0%
	{B11111,B10000,B10100,B10100,B10100,B10100,B10000,B11111}, //10%
	{B11111,B10000,B10110,B10110,B10110,B10110,B10000,B11111}, //20%
	{B11111,B10000,B10111,B10111,B10111,B10111,B10000,B11111}, //30%
	//BAT 2
	{B11111,B00000,B00000,B00000,B00000,B00000,B00000,B11111}, //Bat 2 <40%
	{B11111,B00000,B10000,B10000,B10000,B10000,B00000,B11111}, //40%
	{B11111,B00000,B11000,B11000,B11000,B11000,B00000,B11111}, //50%
	{B11111,B00000,B11100,B11100,B11100,B11100,B00000,B11111}, //60%
	{B11111,B00000,B11110,B11110,B11110,B11110,B00000,B11111}, //70%
	{B11111,B00000,B11111,B11111,B11111,B11111,B00000,B11111}, //80%
	//BAT 3
	{B11110,B00010,B00010,B00011,B00011,B00010,B00010,B11110}, //Bat 3 <90%
	{B11110,B00010,B10010,B10011,B10011,B10010,B00010,B11110}, //90%
	{B11110,B00010,B11010,B11011,B11011,B11010,B00010,B11110}, //100%

};      //Byte-Array of Custom charters for battery status


//Voltage sensor
SDL_Arduino_INA3221 ina3221;
float shuntvoltage1 = 0;
float busvoltage1 = 0;
float current_mA1 = 0;
float loadvoltage1 = 0;

//Servo Settings
#define SERVO_H_PIN 8
#define SERVO_V_PIN 9
Servo servoH;  // create servo object to control a servo
Servo servoV;
byte valH = 90, valV = 90;
bool ServoDirection = false;


//Timer for update data
GTimer myTimer(MS);         
GTimer Timer1sec(MS); //not used


// Menu Settings
const uint8_t digits = 3;
bool menuIsVisible = false;
bool menuEdit = false;
byte menuLvl = 0;
byte menuScreen = 0;
byte arrowPos = 1;
String menuList[] = { "Time","Date","Pos","Mode" };

LcdMenu lcdTest; //TEst


//Global settings
tmElements_t editTime;    //Time edit for time settings settings


bool modeAuto = false;
//float posLong = EEPROM_float_read(0); //Read pos from EEPROM float (4 byte) addr 0-3   
//float posLat = EEPROM_float_read(4); // addr 4-8

float posLong = 50.385214f; //Read pos from EEPROM float (4 byte) addr 0-3     
float posLat = 30.446535f; // addr 4-8

SolarPosition Kiev(50.385214, 30.446535);  // Kiev, UA

//SolarPosition TEST;  // Test not init for make changes in code


//SolarPosition Kiev(posLong, posLat);  // Kiev, UA

void setup() {
	Serial.begin(9600);
	ina3221.begin();

	setSyncProvider(RTC.get);
	SolarPosition::setTimeProvider(RTC.get);

	servoH.attach(SERVO_H_PIN);  // attaches the servo on pin 8-9 to the servo object
	servoV.attach(SERVO_V_PIN);


	enc1.setType(TYPE1);

	myTimer.setInterval(10000);         //Timers set


	//lcd.home();
	delay(100);
	lcd.init();

	lcd.home();
	delay(100);
	lcd.backlight();


	lcd.setCursor(1, 1);
	lcd.print("Solar Track System");
	lcd.setCursor(0, 3);
	lcd.print("dev. by SWEFD");
	lcd.setCursor(16, 3);
	lcd.print("v1.1");


	//Start
	delay(3000);
	printMainScreenLCD();

	//EEPROM_float_write(0, posLong);
	//EEPROM_float_write(4, posLat);


}




void loop() {
	//Serial.println("loop");

	enc1.tick();
	encoderClickEvents();

	if (myTimer.isReady()) {

		printMainScreenLCD();

		menuIsVisible = false;
		menuEdit = false;

		lcd.noBacklight();
		Timer1sec.stop();

		//Serial.print("BYTE = ");

		/*Serial.print("Long = ");
		Serial.println(posLong, 6);
		Serial.print("Lat = ");
		Serial.println(posLat, 6);

		Serial.println("EEPROM:");
		Serial.print("Long = ");
		Serial.println(EEPROM_float_read(0), 6);
		Serial.print("Lat = ");
		Serial.println(EEPROM_float_read(4), 6);*/
		

		//Serial.println((int8_t)test);
	}

	if (Timer1sec.isReady() && menuLvl == 2 && menuScreen == 1) {

		//editTime = getElementTime(now());
		menuLcdTimeSettings(getElementTime(now()));

	}


}


void encoderClickEvents() {

	if (!modeAuto && !menuIsVisible) {

		if (enc1.isClick()) {
			myTimer.reset();
			lcd.backlight();

			ServoDirection = !ServoDirection;
			Serial.println(ServoDirection ? "Vertical" : "Horizontal");
		}
		
		if (enc1.isTurn()) {

			lcd.backlight();
			myTimer.reset();

			if (enc1.isRight()) {
				//Serial.println("Right");
				if (!ServoDirection && valH < 180)
				{
					valH++;
					delay(15);

	
				}
				else if (ServoDirection && valV < 180)
				{
					valV++;
					delay(15);

				}
				//MoveSolarTracker(valH, valV);
			}
			if (enc1.isLeft()) {

				if (!ServoDirection && valH > 0)
				{
					valH--;
					delay(15);
				}
				else if (ServoDirection && valV > 0)
				{
					valV--;
					delay(15);
				}
				
			}

			if (valH == 99 || valV == 99) {
				/*printTimeLCD(RTC.get());
				printSolarPosition(Kiev.getSolarPosition(), digits);*/
				printMainScreenLCD();
			}
			else {
				lcd.setCursor(7, 2);
				lcd.print("H: ");
				lcd.print(valH);
				lcd.print(" V: ");
				lcd.print(valV);
			}

			MoveSolarTracker(valH, valV);

		}
	}

	//MENU Open

	if (enc1.isHolded()) {
		lcd.backlight();
		myTimer.reset();
		menuIsVisible = !menuIsVisible;
		delay(15);
		Serial.println("Menu");

		if (menuIsVisible)
		{
			arrowPos = 1;
			menuLvl = 1;
			menuLcd();
			delay(500);
		}
		else
		{
			menuLvl = 0;
			/*printTimeLCD(RTC.get());
			printSolarPosition(Kiev.getSolarPosition(), digits);*/
			printMainScreenLCD();
			delay(500);
		}
	}


	//MENU

	if (menuIsVisible) {

		if (enc1.isTurn()) {
			myTimer.reset();

			if (menuLvl == 1) {
				if (enc1.isRight()) arrowPos++;
				if (enc1.isLeft()) arrowPos--;	
				menuLcd();	
			}
			else if (menuLvl == 2)
			{
												//Time Settings
				if (menuScreen == 1 ) {

					if (menuEdit) {				//Time Edit in menu

						if (arrowPos == 1) {
							
							if (enc1.isRight()) {           //Hour ++

								editTime.Hour++;
								if (editTime.Hour > 23)
									editTime.Hour = 0;	
								Serial.println(editTime.Hour);
							}


							if (enc1.isLeft()) {

								editTime.Hour--;
								if (editTime.Hour > 23)
									editTime.Hour = 23;
								
								Serial.println(editTime.Hour);
							}


						}
						else if (arrowPos == 2) {
							if (enc1.isRight()) editTime.Minute++;
								if (editTime.Minute == 61) editTime.Minute = 0;

							if (enc1.isLeft()) editTime.Minute--;
								if (editTime.Minute > 61) editTime.Minute = 60;
							
						}
						else if (arrowPos == 3) {
							if (enc1.isRight()) {
								editTime.Second++;
								if (editTime.Second == 61) editTime.Second = 0;
							}
							if (enc1.isLeft()) {
								editTime.Second--;
								if (editTime.Second > 61) editTime.Second = 60;
							}
						}

						//menuLcdTimeSettings(editTime);
					}
					else
					{
						if (enc1.isRight()) arrowPos++;
						if (enc1.isLeft()) arrowPos--;
					}

					//menuLcdTimeSettings(getElementTime(now()));
					menuLcdTimeSettings(editTime);
					//lcdArrow();


					
				}    
				
												//Date Settings 
				else if (menuScreen == 2 ) {			                   

					if (menuEdit) {				//Date Edit in menu

						if (arrowPos == 1) {

							if (enc1.isRight()) {           //Day ++

								editTime.Day++;
								if (editTime.Day > 31)
									editTime.Hour = 0;	
							}


							if (enc1.isLeft()) {			//Day--

								editTime.Day--;
								if (editTime.Day > 31)
									editTime.Day = 31;
							}


						}
						else if (arrowPos == 2) {
							if (enc1.isRight()) editTime.Month++;		//Month
							if (editTime.Month == 13) editTime.Month = 0;

							if (enc1.isLeft()) editTime.Month--;
							if (editTime.Month > 13) editTime.Month = 12;

						}
						else if (arrowPos == 3) {					
							if (enc1.isRight()) {							//Year
								editTime.Year++;
							}
							if (enc1.isLeft()) {
								editTime.Year--;
							}
						}

						//menuLcdTimeSettings(editTime);
					}
					else
					{
						if (enc1.isRight()) arrowPos++;
						if (enc1.isLeft()) arrowPos--;
					}

					//menuLcdTimeSettings(getElementTime(now()));
					menuLcdDateSettings(editTime);
					//lcdArrow();


				}
			}

		}




		if (enc1.isClick()) {
			
			myTimer.reset();

			if (menuLvl == 1) {

				if (arrowPos == 0) {
					menuLvl--;
					menuIsVisible = !menuIsVisible;
					/*printTimeLCD(RTC.get());
					printSolarPosition(Kiev.getSolarPosition(), digits);*/
					printMainScreenLCD();
					delay(500);

				}

													//Go to Time Settings
				if (arrowPos == 1) {      
					menuScreen = 1;
					menuLvl++;
					arrowPos = 1;
					editTime = getElementTime(now());
					menuLcdTimeSettings(editTime);

				}
													//Go to Date Settings
				else if (arrowPos == 2) {		
					menuScreen = 2;
					menuLvl++;
					arrowPos = 1;
					editTime = getElementTime(now());
					menuLcdDateSettings(editTime);
				}
				else if (arrowPos == 3) {
					menuLvl++;
				}
				else if (arrowPos == 4) {
					//menuLvl++;
					modeAuto = !modeAuto;
					menuLcd();
					autoMoveSolarTracker(Kiev.getSolarPosition());
				}

			}
			
			else if (menuLvl==2)
			{
				if (menuScreen == 1)				//Time Settings Click
				{
					if (arrowPos == 0) {
						menuLvl--;
						menuScreen = 0;
						menuLcd();
						//Timer1sec.stop();
					}
					else if (arrowPos == 4) {
						arrowPos = 1;
						setRtcTime(editTime);
						menuLvl--;
						menuScreen = 0;
						menuLcd();
						//Timer1sec.setInterval(1000);
					}
					
					else {       // arrow 1-3
						menuEdit = !menuEdit;
						delay(15);
						menuLcdTimeSettings(editTime);
						//Timer1sec.stop();
						//if (menuEdit) {
						//	Timer1sec.stop();
						//}else{
						//	Timer1sec.start();
						//}

					}
				}
				else if (menuScreen == 2) {				//Date settings Click
					if (arrowPos == 0) {
						menuLvl--;
						menuScreen = 0;
						menuLcd();
						//Timer1sec.stop();
					}
					else if (arrowPos == 4) {
						arrowPos = 2;
						setRtcTime(editTime);
						menuLvl--;
						menuScreen = 0;
						menuLcd();
						//Timer1sec.setInterval(1000);
					}

					else {       // arrow 1-3
						menuEdit = !menuEdit;
						delay(15);
						menuLcdDateSettings(editTime);
						//Timer1sec.stop();
						//if (menuEdit) {
						//	Timer1sec.stop();
						//}else{
						//	Timer1sec.start();
						//}

					}


				}

			}


			//menuScreen = arrowPos;
			//arrowPos = 1;
			//menuLcd();

		}


		if (enc1.isRightH()) Serial.println("Right holded");    //Test
		if (enc1.isLeftH()) Serial.println("Left holded ");
	}



}






void autoMoveSolarTracker(SolarPosition_t pos) {

	if (modeAuto)
	{
		valV = int(135 - pos.elevation);
		valH = map(pos.azimuth, 90, 270, 180, 0);

		if (pos.elevation > 0)
		{
			servoV.write(valV);
			servoH.write(valH);
		}
		else
		{
			Serial.println("Night.");
		}
	}
}

void MoveSolarTracker(byte H, byte V) {
	servoH.write(H);
	servoV.write(V);

	Serial.print("H = ");
	Serial.println(H);
	Serial.print("V = ");
	Serial.println(V);
}


void printMainScreenLCD() {
	printTime(RTC.get());
	Serial.print(F("Kiev:\t"));
	printTimeLCD(RTC.get());
	printVoltageLCD();
	printBatteryStatusLCD();
	printSolarPosition(Kiev.getSolarPosition(), digits);
	autoMoveSolarTracker(Kiev.getSolarPosition());
}


void printSolarPosition(SolarPosition_t pos, int numDigits)
{
	lcd.setCursor(0, 1);
	lcd.print("H ");
	lcd.print(pos.elevation);

	lcd.print(" A ");
	lcd.print(pos.azimuth);
}

void printTime(time_t t)
{
	tmElements_t someTime;
	breakTime(t, someTime);

	Serial.print(someTime.Hour);
	Serial.print(F(":"));
	Serial.print(someTime.Minute);
	Serial.print(F(":"));
	Serial.print(someTime.Second);
	Serial.print(F(" UTC on "));
	Serial.print(dayStr(someTime.Wday));
	Serial.print(F(", "));
	Serial.print(monthStr(someTime.Month));
	Serial.print(F(" "));
	Serial.print(someTime.Day);
	Serial.print(F(", "));
	Serial.println(tmYearToCalendar(someTime.Year));
}

void printTimeLCD(time_t t)
{
	tmElements_t someTime;
	breakTime(t, someTime);

	lcd.clear();
	lcd.setCursor(0, 0);

	if (someTime.Day < 10) {
		lcd.print("0");
	}
	lcd.print(someTime.Day);
	lcd.print("/");

	if (someTime.Month < 10)
	{
		lcd.print("0");
	}
	lcd.print(someTime.Month);
	lcd.print(" ");

	if (someTime.Hour < 10)
	{
		lcd.print("0");
	}
	lcd.print(someTime.Hour);

	lcd.print(":");
	if (someTime.Minute < 10)
	{
		lcd.print("0");
	}
	lcd.print(someTime.Minute);


	//  lcd.print(":");
	//  lcd.print(someTime.Second);
	lcd.print(" UTC");


	lcd.setCursor(0, 2);
	//lcd.print("Mode: ");
	if (modeAuto) {
		lcd.print("AUTO ");

	}
	else {
		lcd.print("MANUAL ");
	}

	lcd.setCursor(7, 2);
	lcd.print("H: ");
	lcd.print(valH);
	lcd.print(" V: ");
	lcd.print(valV);


}



void printVoltageLCD() {

	busvoltage1 = ina3221.getBusVoltage_V(1);			// 1 CH
	shuntvoltage1 = ina3221.getShuntVoltage_mV(1);
	current_mA1 = ina3221.getCurrent_mA(1);  
	loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);

	lcd.setCursor(0, 3);
	lcd.print(" V: ");
	lcd.print(busvoltage1);
	lcd.print("  mA: ");
	lcd.print(current_mA1);
}

void printBatteryStatusLCD() {


	if (busvoltage1 < 3)
	{
		lcd.createChar(1, Batery_stat[4]); //<40%
		lcd.createChar(2, Batery_stat[10]); //<80%

		if (busvoltage1 >= 2.8) {
			lcd.createChar(0, Batery_stat[3]);  //>30%
		}
		else if (busvoltage1 >= 2.7) {
			lcd.createChar(0, Batery_stat[2]);  //>20%
		}
		else if (busvoltage1 >= 2.6) {
			lcd.createChar(0, Batery_stat[1]);  //>10%
		}
		else if (busvoltage1 >= 2.5) {
			lcd.createChar(0, Batery_stat[0]);  //>0%
		}

	}
	else if (busvoltage1 < 4) {
		lcd.createChar(0, Batery_stat[3]);  //>30%
		lcd.createChar(2, Batery_stat[10]); //80%

		if (busvoltage1 >= 3.8)
		{
			lcd.createChar(1, Batery_stat[9]); //>=80%
		}
		else if(busvoltage1 >= 3.7)
		{
			lcd.createChar(1, Batery_stat[8]); //>=70%
		}
		else if(busvoltage1 >= 3.6)
		{
			lcd.createChar(1, Batery_stat[7]); //>=60%
		}
		else if(busvoltage1 >= 3.4)
		{
			lcd.createChar(1, Batery_stat[6]); //>=50%
		}
		else if(busvoltage1 >= 3.2)
		{
			lcd.createChar(1, Batery_stat[5]); //>=40%
		}

	}
	else
	{
		lcd.createChar(0, Batery_stat[3]);  //>30%
		lcd.createChar(1, Batery_stat[9]); //>=80%
		if (busvoltage1 >= 4.1)

		{
			lcd.createChar(2, Batery_stat[12]); //100%
		}
		else {
			lcd.createChar(2, Batery_stat[12]); //90%
		}
	}






	if (busvoltage1 >= 4.1) {
		lcd.createChar(0, Batery_stat[3]);  //Full charge bat
		lcd.createChar(1, Batery_stat[9]);
		lcd.createChar(2, Batery_stat[12]);
	}
	else if (busvoltage1 >= 4)
	{
		lcd.createChar(0, Batery_stat[3]);  //>30%
		lcd.createChar(1, Batery_stat[9]);	//>80%
		lcd.createChar(2, Batery_stat[11]); // 90%

	}
	else if (busvoltage1 >= 3.8) {
		lcd.createChar(0, Batery_stat[3]);  //>30%
		lcd.createChar(1, Batery_stat[9]);	// 80%
		lcd.createChar(2, Batery_stat[10]); // <90%
	}
	else if (busvoltage1 >= 3.6) {
		lcd.createChar(0, Batery_stat[3]);  //>30%
		lcd.createChar(1, Batery_stat[8]);	// 70%
		lcd.createChar(2, Batery_stat[10]); // <90%
	}


	lcd.setCursor(17, 0);
	lcd.write(0);
	lcd.write(1);
	lcd.write(2);

}

void menuLcd() {
	arrowPos = constrain(arrowPos, 0, 4);

	lcd.clear();
	lcdArrow();
	lcd.setCursor(0, 0); lcd.print("...");
	lcd.setCursor(6, 0); lcd.print("SETTINGS");
	lcd.setCursor(1, 1); lcd.print(menuList[0]);
	lcd.setCursor(1, 2); lcd.print(menuList[1]);
	lcd.setCursor(9, 1); lcd.print(menuList[2]);
	lcd.setCursor(9, 2); lcd.print(menuList[3]);
	lcd.print("-");

	modeAuto ? lcd.print("A") : lcd.print("M");
}

void lcdArrow() {

	Serial.println("LcdArrow();");
	Serial.println(menuLvl);
	Serial.println(menuScreen);

	if (menuLvl == 1) {
		switch (arrowPos) {                    //arrow print 
		case 0: lcd.setCursor(3, 0);
			lcd.write(127);
			break;
		case 1: lcd.setCursor(0, 1);
			lcd.write(126);
			break;
		case 2: lcd.setCursor(0, 2);
			lcd.write(126);
			break;
		case 3: lcd.setCursor(8, 1);
			lcd.write(126);
			break;
		case 4: lcd.setCursor(8, 2);
			lcd.write(126);
			break;
		}
	}

	else if (menuLvl == 2) {
		if (menuScreen == 1 || menuScreen == 2) {
			if (arrowPos > 4)
				arrowPos = 0;

			switch (arrowPos) {                    //arrow print 
			case 0: lcd.setCursor(3, 0);
				lcd.write(127);
				break;
			case 1: lcd.setCursor(3, 1);
				lcd.write(60);
				lcd.write(62);
				break;
			case 2: lcd.setCursor(6, 1);
				lcd.write(60);
				lcd.write(62);
				break;
			case 3: lcd.setCursor(9, 1);
				lcd.write(60);
				if (menuScreen == 2) lcd.setCursor(12, 1); //Date x4 char
				lcd.write(62);
				break;
			case 4: lcd.setCursor(16, 3);
				lcd.write(126);
				break;
			}
		}

	}





}

void menuLcdTimeSettings(tmElements_t someTime) {

	lcd.clear();

	lcdArrow();


	lcd.setCursor(0, 0);
	lcd.print("...");

	lcd.setCursor(4, 0);
	lcd.print("TIME SETTINGS");

	if (menuEdit) {
		lcd.setCursor(2, 2);
		lcd.print('*');
	}

	lcd.setCursor(3, 2);
	if (someTime.Hour < 10)
	{
		lcd.print("0");
	}
	lcd.print(someTime.Hour);
	lcd.print(":");

	if (someTime.Minute < 10)
	{
		lcd.print("0");
	}
	lcd.print(someTime.Minute);
	lcd.print(":");

	if (someTime.Second < 10) {
		lcd.print("0");
	}
	lcd.print(someTime.Second);
	lcd.print(F(" UTC"));
	lcd.setCursor(17, 3);
	lcd.print("OK");
}



void menuLcdDateSettings(tmElements_t someTime) {

	lcd.clear();

	lcdArrow();


	lcd.setCursor(0, 0);
	lcd.print("...");

	lcd.setCursor(4, 0);
	lcd.print("DATE SETTINGS");

	if (menuEdit) {
		lcd.setCursor(2, 2);
		lcd.print('*');
	}

	lcd.setCursor(3, 2);
	if (someTime.Day < 10)
	{
		lcd.print("0");
	}
	lcd.print(someTime.Day);
	lcd.print("/");

	if (someTime.Month < 10)
	{
		lcd.print("0");
	}
	lcd.print(someTime.Month);
	lcd.print("/");

	lcd.print(tmYearToCalendar(someTime.Year));
	//lcd.print(F(" UTC"));
	lcd.setCursor(17, 3);
	lcd.print("OK");
}



void setRtcTime(tmElements_t tm) {

	time_t et;

	//int y = year(); //Need edit

	//if (year() >= 1000)
	//	tm.Year = CalendarYrToTm(year());
	//else    // (y < 100)
	//	tm.Year = y2kYearToTm(year());
	//tm.Month = month();
	//tm.Day = day();

	et = makeTime(tm);
	RTC.set(et);        // use the time_t value to ensure correct weekday is set
	setTime(et);
}


tmElements_t getElementTime(time_t t) {
	tmElements_t res;
	breakTime(t, res);
	return res;
}


void EEPROM_float_write(int addr, float val) // çàïèñü â ÅÅÏÐÎÌ
{
	byte* x = (byte*)&val;
	for (byte i = 0; i < 4; i++) EEPROM.write(i + addr, x[i]);
}

float EEPROM_float_read(int addr) // ÷òåíèå èç ÅÅÏÐÎÌ
{
	byte x[4];
	for (byte i = 0; i < 4; i++) x[i] = EEPROM.read(i + addr);
	float* y = (float*)&x;
	return y[0];
}

