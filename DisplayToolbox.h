/*
	MatrixDisplay Library 2.0
	Author: Miles Burton, www.milesburton.com/
	Need a 16x24 display? Check out www.mnethardware.co.uk
	Copyright (c) 2010 Miles Burton All Rights Reserved

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef DISPLAY_TOOLBOX_GUARD
#define DISPLAY_TOOLBOX_GUARD

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <wiring.h>
#include "HardwareSerial.h"
#include <MatrixDisplay.h>

/*
This is a utility class, it's purpose to provide several useful functions which maybe used frequently but are not core to the MatrixDisplay operation.

Various code has been shamelessly pinched from around the web, credit added where possible. Please drop me a line if I've used it without permission or forgotten associated credit.
*/

class DisplayToolbox
{
private:
	MatrixDisplay* disp;
	uint8_t calcDispNum(int& x);

	
public:	
	// Constructor
    DisplayToolbox(MatrixDisplay*);
    
	// Destructor
    ~DisplayToolbox();
	
	
	void drawCircle(uint8_t xp,uint8_t  yp, uint8_t radius, uint8_t col = 1);
	void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t val );
	void setPixel(int x, int y, int val, bool paint = false);
	uint8_t getPixel(int x, int y, bool fromShadow);
	void setBrightness(uint8_t pwmValue);
	void drawRectangle(uint8_t _x, uint8_t _y, uint8_t width, uint8_t height, uint8_t colour, bool filled = false);
	//void drawFilledRectangle(int, int, int, int, int);
};

#endif
