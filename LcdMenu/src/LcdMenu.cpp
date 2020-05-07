/*
 Name:		LcdMenu.cpp
 Created:	5/6/2020 8:39:04 PM
 Author:	Chornous Oleksandr (swefd)
 Editor:	http://www.visualmicro.com
*/

#include "LcdMenu.h"

LcdMenu::LcdMenu()
{
    Serial.println("Obj CREATE");
}

LcdMenu::~LcdMenu()
{

}


void LcdMenu::tst()
{
    Serial.println("Class WORK");
}


String LcdMenu::getText()
{
    return "Text From Class";
}
