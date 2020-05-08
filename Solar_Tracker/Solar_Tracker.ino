/*
 Name:		Solar_Tracker.ino
 Created:	5/1/2020 5:32:21 PM
 Author:	swefd
*/
//#include <Wire.h>
//#include <Time.h>
//#include <SPI.h>

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
byte Batery_stat_0[] = {
  B11111,
  B10000,
  B10111,
  B10111,
  B10111,
  B10111,
  B10000,
  B11111
};

byte Batery_stat_1[] = {
  B11111,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B11111
};

byte Batery_stat_2[] = {
  B11110,
  B00010,
  B11010,
  B11011,
  B11011,
  B11010,
  B00010,
  B11110
};


//Servo Settings
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

	SolarPosition::setTimeProvider(RTC.get);

	setSyncProvider(RTC.get);

	Serial.println(lcdTest.getText());

	Serial.println("START kurwa");

	servoH.attach(8);  // attaches the servo on pin 9 to the servo object
	servoV.attach(9);

	Serial.println("1");

	enc1.setType(TYPE1);

	myTimer.setInterval(10000);         //Timers set

	Serial.println("2");

	//lcd.home();
	delay(100);
	lcd.init();
	lcd.createChar(0, Batery_stat_0);
	lcd.createChar(1, Batery_stat_1);
	lcd.createChar(2, Batery_stat_2);
	lcd.home();
	Serial.println("3");
	//lcd.begin(20, 4);
	delay(100);
	Serial.println("4");
	lcd.backlight();
	Serial.println("5");
	lcd.setCursor(0, 0);
	Serial.println("6");
	lcd.print("Hello");
	lcd.setCursor(0, 1);
	lcd.print("MADAFAKA :)");




	//Start
	delay(2000);
	printTime(RTC.get());
	printTimeLCD(RTC.get());
	printSolarPosition(Kiev.getSolarPosition(), digits);
	autoMoveSolarTracker(Kiev.getSolarPosition());
	



	//EEPROM_float_write(0, posLong);
	//EEPROM_float_write(4, posLat);


}




void loop() {
	//Serial.println("loop");

	enc1.tick();
	encoderClickEvents();

	if (myTimer.isReady()) {

		printTime(RTC.get());
		Serial.print(F("Kiev:\t"));
		printTimeLCD(RTC.get());
		printSolarPosition(Kiev.getSolarPosition(), digits);
		autoMoveSolarTracker(Kiev.getSolarPosition());

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
				printTimeLCD(RTC.get());
				printSolarPosition(Kiev.getSolarPosition(), digits);
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
			printTimeLCD(RTC.get());
			printSolarPosition(Kiev.getSolarPosition(), digits);
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
					printTimeLCD(RTC.get());
					printSolarPosition(Kiev.getSolarPosition(), digits);
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



		if (enc1.isRightH()) Serial.println("Right holded");
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

void printSolarPosition(SolarPosition_t pos, int numDigits)
{
	//Serial.print(F("el: "));
	//Serial.print(pos.elevation, numDigits);
	//Serial.print(F(" deg\t"));
	lcd.setCursor(0, 1);
	lcd.print("H ");
	lcd.print(pos.elevation);

	//Serial.print(F("az: "));
	//Serial.print(pos.azimuth, numDigits);
	//Serial.println(F(" deg"));
	lcd.print(" A ");
	lcd.print(pos.azimuth);


	//Serial.println(int(135 - pos.elevation));
	//Serial.println(map(pos.azimuth, 90, 270, 180, 0));
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
	lcd.setCursor(17, 0);

	lcd.write(0);
	lcd.write(1);
	lcd.write(2);

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

