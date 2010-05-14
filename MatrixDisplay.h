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

#ifndef MATRIX_DISPLAY_GUARD
#define MATRIX_DISPLAY_GUARD

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <wiring.h>
#include "HardwareSerial.h"

#include "ht1632_cmd.h"
// No operation ASM instruction. Forces a delay
#define _nop() do { __asm__ __volatile__ ("nop"); } while (0)

class MatrixDisplay
{
private:
	uint8_t *pShadowBuffers; // Will store the pixel data for each display
    uint8_t *pDisplayBuffers; // Will store the pixel data for each display
    uint8_t *pDisplayPins; // Will contain the pins for each CS
    
	// Associated pins
    uint8_t  dataPin;
    uint8_t  clkPin;
	
    uint8_t  displayCount;
    uint8_t  backBufferSize;
	
	// Converts a cartesian coordinate to a display index
	uint8_t displayXYToIndex(uint8_t x, uint8_t y);
	
	// Converts caretesian coordinate to the custom display buffer index
    uint8_t xyToIndex(uint8_t x, uint8_t y);
    
	// Enables/disables a specific display in the series
    void    selectDisplay(uint8_t displayNum);  
    void    releaseDisplay(uint8_t displayNum);
    
	// Todo combine methods using bitwise shift
	// Writes data to the write MSB first
    void    writeDataBE(int8_t bitCount, uint8_t data, bool useNop = false);
    
	// Writes data to the wire LSB first
    void    writeDataLE(int8_t bitCount, uint8_t data);
    
	// Write command to write
    void    writeCommand(uint8_t displayNum, uint8_t command);

	// High speed write to write (AtMega328 only)
    void    bitBlast(uint8_t pin, uint8_t data);
	
	// Debug, write a byte to serial
	void	dumpByte(uint8_t byte);
	
	// Debug
	void	preCommand(); // Sends 100 down the line
    	
	//TODO:
	// Write Column
	// Write Block etc
	// Take advantage of progresswrite
public:	
	// Constructor
	// Number of displays (1-4)
	// Shared clock pin
	// Shared data pin
    MatrixDisplay(uint8_t numDisplays, uint8_t clkPin, uint8_t dataPin, bool buildShadow = false);
    
	// Destructor
    ~MatrixDisplay();
    
	// Fetch a pixel from a specific one display coordinate 
    uint8_t getPixel(uint8_t displayNum, uint8_t x, uint8_t y, bool useShadow = false);
    
	// Set pixel from a specific one display coordinate
    void    setPixel(uint8_t displayNum, uint8_t x, uint8_t y, uint8_t value, bool paint = false, bool useShadow = false);

	// Initalise a display
    void    initDisplay(uint8_t displayNum, uint8_t pin, bool isMaster);
    
	// Sync display using progressive write (Can be buggy, very fast)
    void    syncDisplays();
	
	// Clear a single display. 
	// paint ? Send data to display : Only clear data
	void	clear(uint8_t displayNum, bool paint = false, bool useShadow = false);
	
	// Clear all displays
	void 	clear(bool paint = false, bool useShadow = false);
	
	// Write a single nybble to the display (the display writes 4 bits at a time min)
	void	writeNibbles(uint8_t displayNum, uint8_t addr, uint8_t* data, uint8_t nybbleCount);
	
	// Helper functions
	uint8_t getDisplayCount();
	
	// Defaults
	uint8_t getDisplayHeight();
	uint8_t getDisplayWidth();
	
	// Shadow 
	void	copyBuffer();
	
	// Shift the buffer Left|Right
	void	shiftLeft();
	void	shiftRight();
	
	// Set PWN brightness
	void	setBrightness(uint8_t dispNum, uint8_t pwmValue);
	
	
};

#endif
