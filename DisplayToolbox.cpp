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


#include <DisplayToolbox.h>
///////////////////////////////////////////////////////////////////////////////
//  CTORS & DTOR
//

	DisplayToolbox::DisplayToolbox(MatrixDisplay* _disp)
	{
		// Take reference to display
		disp = _disp;
	}

	DisplayToolbox::~DisplayToolbox()
	{
		disp = 0;
	}
	
	
// y, y radius, color
// Shamelessly taken from http://actionsnippet.com/?p=492
void DisplayToolbox::drawCircle(uint8_t xp,uint8_t  yp, uint8_t radius, uint8_t col)
{
  int8_t  balance = 0;
  int8_t xoff = 0;
  int8_t yoff = 0;
  xoff=0;
  yoff=radius;
  balance=- radius;
  while (xoff <= yoff) {
    setPixel(xp+xoff, yp+yoff, col);
    setPixel(xp-xoff, yp+yoff, col);
    setPixel(xp-xoff, yp-yoff, col);
    setPixel(xp+xoff, yp-yoff, col);
    setPixel(xp+yoff, yp+xoff, col);
    setPixel(xp-yoff, yp+xoff, col);
    setPixel(xp-yoff, yp-xoff, col);
    setPixel(xp+yoff, yp-xoff, col);
    if ((balance += xoff++ + xoff)>= 0) 
    {
      balance-=--yoff+yoff;
    }
  }
}


// Bresenham's line function
void DisplayToolbox::drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t val )
{
  char deltax = abs(x2 - x1);        // The difference between the x's
  char deltay = abs(y2 - y1);        // The difference between the y's
  char x = x1;                       // Start x off at the first pixel
  char y = y1;                       // Start y off at the first pixel
  char xinc1, xinc2, yinc1, yinc2, den, num, numadd, numpixels, curpixel;

  if (x2 >= x1) {                // The x-values are increasing
    xinc1 = 1;
    xinc2 = 1;
  }  
  else {                          // The x-values are decreasing
    xinc1 = -1;
    xinc2 = -1;
  }

  if (y2 >= y1)                 // The y-values are increasing
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          // The y-values are decreasing
  {
    yinc1 = -1;
    yinc2 = -1;
  }

  if (deltax >= deltay)         // There is at least one x-value for every y-value
  {
    xinc1 = 0;                  // Don't change the x when numerator >= denominator
    yinc2 = 0;                  // Don't change the y for every iteration
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         // There are more x-values than y-values
  }
  else                          // There is at least one y-value for every x-value
  {
    xinc2 = 0;                  // Don't change the x for every iteration
    yinc1 = 0;                  // Don't change the y when numerator >= denominator
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         // There are more y-values than x-values
  }

  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    setPixel(x, y, val);             // Draw the current pixel
    num += numadd;              // Increase the numerator by the top of the fraction
    if (num >= den)             // Check if numerator >= denominator
    {
      num -= den;               // Calculate the new numerator value
      x += xinc1;               // Change the x as appropriate
      y += yinc1;               // Change the y as appropriate
    }
    x += xinc2;                 // Change the x as appropriate
    y += yinc2;                 // Change the y as appropriate
  }
}

// setPixelting function (adds support for multiple displays)
void DisplayToolbox::setPixel(int x, int y, int val, bool paint)
{
  // setPixel
  // Display Number
  // X Cordinate
  // Y Cordinate
  // Value (either on or off, 1, 0)
  // Do you want to write this change straight to the display? (yes: slower)
  disp->setPixel(calcDispNum(x), x, y, val, paint);   
}

// Fetch pixel
// fromShadow - Retrieve from a secondary buffer. 
uint8_t DisplayToolbox::getPixel(int x, int y, bool fromShadow)
{
  return disp->getPixel(calcDispNum(x), x, y, fromShadow);   
}

// Calculate which display x resides and adjust x so it's within the bounds of one display
uint8_t DisplayToolbox::calcDispNum(int& x)
{
  int dispNum = 0;
  if(x >= disp->getDisplayWidth())
  {
    dispNum = x / disp->getDisplayWidth();
    x -= (disp->getDisplayWidth() * dispNum);
  }
  return dispNum;
}

void DisplayToolbox::setBrightness(uint8_t pwmValue)
{
	for(int dispNum=0; dispNum<disp->getDisplayCount(); ++dispNum) disp->setBrightness(dispNum, pwmValue); 
}



void DisplayToolbox::drawRectangle(uint8_t _x, uint8_t _y, uint8_t width, uint8_t height, uint8_t colour, bool filled)
{
	drawLine(_x, _y, _x, _y+height, colour); // Left side of box
	drawLine(_x+width, _y, _x+width, _y+height, colour); // Right side of box
	
	drawLine(_x, _y, _x+width, _y, colour); // top of box
	drawLine(_x, _y+height, _x+width, _y+height, colour); // bottom of box
}
