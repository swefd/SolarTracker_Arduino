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

#define SW 4     //Encoder pins
#define DT 3
#define CLK 2

LiquidCrystal_I2C lcd(0x27, 20, 4);

//Servo Settings
Servo servoH;  // create servo object to control a servo
Servo servoV;

LcdMenu lcdTest;

Encoder enc1(CLK, DT, SW);  //Encoder object
GTimer myTimer(MS);         //Timer for update data
GTimer lcdTimeUpdate(MS);



bool modeAuto = false;
byte valH = 90, valV = 90;
bool ServoDirection = false;
const uint8_t digits = 3;

// Menu Settings
bool menuIsVisible = false;
byte menuLvl = 0;
byte menuScreen = 0;
byte arrowPos = 0;
String menuList[] = { "Time","Date","Pos","Mode" };



//float posLong = EEPROM_float_read(0); //Read pos from EEPROM float (4 byte) addr 0-3     
float posLong = 50.385214f; //Read pos from EEPROM float (4 byte) addr 0-3     
//float posLat = EEPROM_float_read(4); // addr 4-8
float posLat = 30.446535f; // addr 4-8


SolarPosition Kiev(50.385214, 30.446535);  // Kiev, UA


//SolarPosition Kiev(posLong, posLat);  // Kiev, UA


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



void setup() {
	Serial.begin(9600);

	SolarPosition::setTimeProvider(RTC.get);

	Serial.println(lcdTest.getText());

	Serial.println("START kurwa");

	servoH.attach(8);  // attaches the servo on pin 9 to the servo object
	servoV.attach(9);
	Serial.println("1");
	enc1.setType(TYPE1);
	myTimer.setInterval(5000);
	lcdTimeUpdate.setInterval(1000);
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

	if (lcdTimeUpdate.isReady() && modeAuto) {

	}

}


void encoderClickEvents() {

	if (!modeAuto && !menuIsVisible) {

		if (enc1.isClick()) {
			ServoDirection = !ServoDirection;
			Serial.println(ServoDirection ? "Vertical" : "Horizontal");
		}

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
			MoveSolarTracker(valH, valV);
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
			MoveSolarTracker(valH, valV);
		}
	}


	if (enc1.isHolded()) {
		menuIsVisible = !menuIsVisible;
		delay(15);
		Serial.println("Menu");

		if (menuIsVisible)
		{
			arrowPos = 0;
			menuLvl = 1;
			menuLcd(menuLvl, menuScreen);
			delay(500);
		}
		else
		{
			menuLvl = 0;
			printTimeLCD(RTC.get());
			printSolarPosition(Kiev.getSolarPosition(), digits);
			myTimer.reset();
			delay(500);
		}


	}


	//MENU

	if (menuIsVisible) {

		if (enc1.isTurn()) {

			if (enc1.isRight()) arrowPos++;
			if (enc1.isLeft()) arrowPos--;

			menuLcd(menuLvl, menuScreen);
		}

		if (enc1.isClick()) {
			
			
			if (arrowPos == 0) {
				menuLvl--;

			}else{
				if(arrowPos!=4)
				menuLvl++;
			}
			
			menuScreen = arrowPos;
			arrowPos = 1;
			menuLcd(menuLvl, menuScreen);

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
	Serial.print(F("el: "));
	Serial.print(pos.elevation, numDigits);
	Serial.print(F(" deg\t"));
	lcd.setCursor(0, 1);
	lcd.print("H ");
	lcd.print(pos.elevation);

	Serial.print(F("az: "));
	Serial.print(pos.azimuth, numDigits);
	Serial.println(F(" deg"));
	lcd.print(" A ");
	lcd.print(pos.azimuth);


	Serial.println(int(135 - pos.elevation));
	Serial.println(map(pos.azimuth, 90, 270, 180, 0));
}

void printTimeLCD(time_t t)
{
	tmElements_t someTime;
	breakTime(t, someTime);

	lcd.clear();
	lcd.setCursor(0, 0);

	if (modeAuto) {
		lcd.print("AUTO ");

	}
	else {
		lcd.print("MANUAL ");
	}
	lcd.print(someTime.Hour);
	lcd.print(":");
	lcd.print(someTime.Minute);
	//  lcd.print(":");
	//  lcd.print(someTime.Second);
	lcd.print(" UTC");
	lcd.setCursor(17, 0);

	lcd.write(0);
	lcd.write(1);
	lcd.write(2);

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

void menuLcd(byte lvl, byte screen) {

	myTimer.reset();
	lcd.clear();

	if (lvl == 1) {
		arrowPos = constrain(arrowPos, 0, 4);
		lcd.setCursor(1, 0); lcd.print(menuList[0]);
		lcd.setCursor(1, 1); lcd.print(menuList[1]);
		lcd.setCursor(9, 0); lcd.print(menuList[2]);
		lcd.setCursor(9, 1); lcd.print(menuList[3]);
		lcd.print("-");

		modeAuto ? lcd.print("A") : lcd.print("M");
	}

	else if (lvl == 2) {

		switch (screen)
		{
		case 0:  //Time
			break;
		case 1:  //Date
			break;
		case 2:  //Pos
			break;
		case 3:  //Mode
			arrowPos = constrain(arrowPos, 0, 2);
			float editPosLong = posLong;
			float editPosLat = posLat;
			lcd.setCursor(1, 0);
			lcd.print("long ");
			lcd.print(posLong, 6);
			lcd.setCursor(1, 1);
			lcd.print("lat ");
			lcd.print(posLat, 6);
			break;
		case 4:
			modeAuto = !modeAuto;
			menuLcd(menuLvl, menuScreen);
			break;
		default:
			break;
		}

	}
	else {
		menuIsVisible = !menuIsVisible;
		printTimeLCD(RTC.get());
		printSolarPosition(Kiev.getSolarPosition(), digits);
		myTimer.reset();
		delay(500);
	}

	switch (arrowPos) {                    //arrow print 
	case 0: lcd.setCursor(0, 0);
		lcd.write(127);
		break;
	case 1: lcd.setCursor(0, 0);
		lcd.write(126);
		break;
	case 2: lcd.setCursor(0, 1);
		lcd.write(126);
		break;
	case 3: lcd.setCursor(8, 0);
		lcd.write(126);
		break;
	case 4: lcd.setCursor(8, 1);
		lcd.write(126);
		break;

	}
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

