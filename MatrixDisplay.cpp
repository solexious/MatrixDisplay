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


#if defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__) // compiled as ATMega644
//Atmega644 Version of fastWrite - for pins 0-15
#define fWriteA(_pin_, _state_) ( _pin_ < 8 ? (_state_ ?  PORTB |= 1 << _pin_ : \
PORTB &= ~(1 << _pin_ )) : (_state_ ?  PORTD |= 1 << (_pin_ -8) : PORTD &= ~(1 << (_pin_ -8)  )))

//Atmega644 Version of fastWrite - for pins 16-31 (Note: PORTA mapping reversed from others)
#define fWriteB(_pin_, _state_) ( _pin_ < 24 ? (_state_ ?  PORTC |= 1 << (_pin_ -16) : \
PORTC &= ~(1 << (_pin_ -16))) : (_state_ ?  PORTA |= 1 << (31- _pin_) : PORTA &= ~(1 << (31- _pin_)  )))

#else  // if ATMega328
//Atmega328 Version of fastWrite - for pins 0-13
#define fWriteA(_pin_, _state_) ( _pin_ < 8 ? (_state_ ?  PORTD |= 1 << _pin_ : \
PORTD &= ~(1 << _pin_ )) : (_state_ ?  PORTB |= 1 << (_pin_ -8) : PORTB &= ~(1 << (_pin_ -8)  )))

//Atmega328 Version of fastWrite - for pins 14-19
#define fWriteB(_pin, _state_) (_state_ ?  PORTC |= 1 << (_pin - 14) : \
PORTC &= ~(1 << (_pin -14) ))  
#endif

// Encode the Y coordinate to a bit #
#define CalcBit(y) (1 << (y > 7 ? y -8 : y))

#include "MatrixDisplay.h"

#define NULL                0
#define BACKBUFFER_SIZE     32

#define DIRTY_BIT           0x80


///////////////////////////////////////////////////////////////////////////////
//  CTORS & DTOR
//
// Setup the buffers within the constructor, a little more inflexible but saves pain later on
MatrixDisplay::MatrixDisplay(uint8_t numDisplays, uint8_t clkPin, uint8_t dataPin, bool buildShadow)
    : pDisplayBuffers(NULL)
    , pDisplayPins(NULL)
    , dataPin(dataPin)
    , clkPin(clkPin)
    , displayCount(numDisplays)
	, backBufferSize(sizeof(uint8_t) * BACKBUFFER_SIZE)
{
    // allocate RAM buffer for display bits
    // 32 columns * 8 rows / 8 bits = 32 bytes
    uint8_t sz = displayCount * backBufferSize;
    pDisplayBuffers = (uint8_t *)malloc(sz);
    memset(pDisplayBuffers, 0, sz); 
	
	if(buildShadow)
	{
		// allocate RAM buffer for display bits
		pShadowBuffers = (uint8_t *)malloc(sz);
		memset(pShadowBuffers, 0, sz); 
	}
    
    // allocate a buffer for pin assignments
    pDisplayPins = (uint8_t *) malloc( sizeof(uint8_t) * numDisplays );
    memset(pDisplayPins, 0, sizeof(uint8_t) * numDisplays);
    
    // set data & clock pin modes
    pinMode(dataPin, OUTPUT);
    pinMode(clkPin, OUTPUT);
    
    bitBlast(dataPin, 1);
    bitBlast(clkPin, 1);
}

// Destructor
MatrixDisplay::~MatrixDisplay() 
{
    if(pDisplayBuffers)
    {
        free(pDisplayBuffers);
        pDisplayBuffers = NULL;
    }
    
    if(pDisplayPins)
    {
        free(pDisplayPins);
        pDisplayPins = NULL;
    }
	
	if(pShadowBuffers)
	{
		free(pShadowBuffers);
		pShadowBuffers = NULL;		
	}
}


///////////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS
//
void MatrixDisplay::initDisplay(uint8_t displayNum, uint8_t pin, bool master)
{        
    int myint = (int) displayNum;
	// Associate the pin with this display
	pDisplayPins[displayNum] = pin;
	// init the hardware
	pinMode(pin, OUTPUT);
	bitBlast(pin, 1); // Disable chip (pull high)
	
	selectDisplay(displayNum);
	// Send Precommand
	preCommand();
	// Take advantage of successive mode and write the options
	writeDataBE(8,HT1632_CMD_SYSDIS, true);
	writeDataBE(8,HT1632_CMD_COMS10,true);
	if(master)
    {
        writeDataBE(8,HT1632_CMD_MSTMD,true);
        Serial.print(myint);
        Serial.println(" is Master");
    }
    else
    {
        writeDataBE(8,HT1632_CMD_SLVMD,true);        
        Serial.print(myint);
        Serial.println(" is Slave");
    }
	writeDataBE(8,HT1632_CMD_SYSEN,true);
	writeDataBE(8,HT1632_CMD_LEDON,true);
	writeDataBE(8,HT1632_CMD_BLOFF,true);
	writeDataBE(8,HT1632_CMD_PWM+15,true);
	releaseDisplay(displayNum);
	clear(displayNum,true);
}

uint8_t MatrixDisplay::getPixel(uint8_t displayNum, uint8_t x, uint8_t y, bool useShadow)
{
    // Encode XY to an appropriate XY address
    uint8_t address = xyToIndex(x, y);
	
	// offset to the correct buffer for the display
    address += backBufferSize * displayNum;
    	

    uint8_t bit = CalcBit(y);
	
    // fetch the value byte from the buffer
	address = (backBufferSize * displayNum) + address;
	uint8_t value = 0;
	
	if(useShadow)
	{
		value = pShadowBuffers[address];
	}else{
		value = pDisplayBuffers[address];
	}
    
  	
	return (value & bit) ? 1 : 0; 
}


void MatrixDisplay::setPixel(uint8_t displayNum, uint8_t x, uint8_t y, uint8_t value, bool paint, bool useShadow)
{
    // calculate a pointer into the display buffer (6 bit offset)
    uint8_t address = xyToIndex(x, y);
	uint8_t dispAddress = address;
   
    // offset to the correct buffer for the display
    address += backBufferSize * displayNum;	
    uint8_t bit = CalcBit(y);
	
	
	// ...and apply the value
    if(value)
    {
		if(useShadow)
		{
			pShadowBuffers[address] |= bit;
		}else{
			pDisplayBuffers[address] |= bit;
		}
    }
    else
    {
		if(useShadow)
		{
			pShadowBuffers[address] &= ~bit;
		}else{
			pDisplayBuffers[address] &= ~bit;
		}
    }
	
    if(useShadow) return;
	
	if(!paint)
	{
		// flag the display as dirty
		//pDisplayPins[displayNum] |= DIRTY_BIT;
	}else{
		uint8_t dispAddress = displayXYToIndex(x, y);
	    uint8_t value = pDisplayBuffers[address];
		if(y>=4) // Devide y by 4. Work out whether it's odd or even. 8 pixels packed into 1 byte. 16 pixels are in two bytes. We need to figure out whether to shift the buffer
		{
			value = pDisplayBuffers[address]  >> 4;
		}
	
		writeNibbles(displayNum, dispAddress, &value, 1);
	}
}

void MatrixDisplay::dumpByte(uint8_t aByte)
{
    Serial.println("Byte value");
	for(int8_t k=7; k>=0; --k)
	{
		Serial.print((aByte >> k) & 1, DEC);				
	}
	Serial.print("\n\n");
}

// Write the backbuffer out to all displays (which have the dirty bit set!)
void MatrixDisplay::syncDisplays() 
{
	uint8_t bufferOffset = 0;
	uint8_t value  = 0;
	
    for(int8_t dispNum=0; dispNum < displayCount; ++dispNum)
    {
	
	   // if(pDisplayPins[dispNum] & DIRTY_BIT == 0) continue;
		
		bufferOffset = backBufferSize * dispNum;
		
		 // flip the "dirty" bit
		 // pDisplayPins[dispNum] ^= DIRTY_BIT;
		
		selectDisplay(dispNum);
		writeDataBE(3, HT1632_ID_WR); // Send "write to display" command
		writeDataBE(7, 0); // Send initial address (aka 0)
		
		// Operating in progressive addressing mode
		for(int8_t addr = 0; addr < backBufferSize; ++addr) // Thought i'd simplify this a touch
		{
			value = pDisplayBuffers[addr + bufferOffset];
            writeDataLE(8, value);  
		}
		
		releaseDisplay(dispNum);

	}
}

void MatrixDisplay::writeNibbles(uint8_t displayNum, uint8_t addr, uint8_t* data, uint8_t nybbleCount)
{
  selectDisplay(displayNum);  // Select chip
  writeDataBE(3, HT1632_ID_WR);  // send ID: WRITE to RAM
  writeDataBE(7,addr); // Send address
  for(int8_t i = 0; i < nybbleCount; ++i) writeDataLE(4,data[i]); // send multiples of 4 bits of data
  releaseDisplay(displayNum); // done
}


void MatrixDisplay::clear(uint8_t displayNum, bool paint, bool useShadow)
{
    // clear the display's backbuffer
	if(useShadow)
	{
		memset( pShadowBuffers + (backBufferSize * displayNum), 
				0, 
				backBufferSize
			   );	
	
	}else{
		memset( pDisplayBuffers + (backBufferSize * displayNum), 
				0, 
				backBufferSize
			   );
		   
		// Flag dirty bit
		// pDisplayPins[displayNum] |= DIRTY_BIT;
	}
	

	
	// Write out change
	if(paint && !useShadow) syncDisplays();
}

void MatrixDisplay::clear(bool paint, bool useShadow)
{
	if(useShadow)
	{
		memset(pShadowBuffers,0, backBufferSize*displayCount);
	}else{
		memset(pDisplayBuffers,0, backBufferSize*displayCount);
	}
	
	// Select all displays and clear
	if(paint && !useShadow)
	{
	
		for(int8_t i=0; i<displayCount; ++i) selectDisplay(i); // Enable all displays
	
		// Use progressive write mode, faster
		writeDataBE(3, HT1632_ID_WR); // Send "write to display" command
		writeDataBE(7, 0); // Send initial address (aka 0)
			
		for(uint8_t i = 0; i<32; ++i)
		{
			writeDataLE(4,0xff); // Write nada
		}
	
		for(int8_t i=0; i<displayCount; ++i) releaseDisplay(i); // Disable all displays
	}
}


///////////////////////////////////////////////////////////////////////////////
//  PRIVATE FUNCTIONS
//
inline uint8_t MatrixDisplay::xyToIndex(uint8_t x, uint8_t y)
{

    // cap X coordinate at 32 column
    x &= 0x1F;
    // cap Y coordinate at 8
   // y &= 0x7;
	
	
	return x; // (64 *4) packed into 8 bits = 32.. 32 columns. 32 indices.. return x
}

inline uint8_t MatrixDisplay::displayXYToIndex(uint8_t x, uint8_t y)
{
	uint8_t addresss = x << 1; // Calculate which quandrant[?] it's in 
	addresss += y >=4 ? 1 : 0;
	return addresss;
}


inline void MatrixDisplay::selectDisplay(uint8_t displayNum)
{
//	Serial.println(pDisplayPins[displayNum],DEC);
    bitBlast(pDisplayPins[displayNum], 0);
	//digitalWrite(5,0); 
}

inline void MatrixDisplay::releaseDisplay(uint8_t displayNum)

{
//	Serial.println(pDisplayPins[displayNum],DEC);
    bitBlast(pDisplayPins[displayNum], 1);
	//digitalWrite(5,1); 
}


void MatrixDisplay::writeCommand(uint8_t displayNum, uint8_t command)
{
    selectDisplay(displayNum);
    bitBlast(dataPin, 1);
    writeDataBE(3, HT1632_ID_CMD); // Write out MSB [3 bits]
    writeDataBE(8, command); // Then MSB [7 8 bits]
    writeDataBE(1, 0); // 1 bit extra 
    bitBlast(dataPin, 0);
    releaseDisplay(displayNum);
}

// Writes out LSB first
void MatrixDisplay::writeDataLE(int8_t bitCount, uint8_t data)
{
    //if(bitCount <= 0 || bitCount > 8) return;
    
    // assumes correct display is selected
    for(int8_t i = 0; i < bitCount; ++i)
    {
        bitBlast(clkPin, 0);
        bitBlast(dataPin, (data >> i) & 1);
        bitBlast(clkPin, 1);
    }
}

// Writes out MSB first
void MatrixDisplay::writeDataBE(int8_t bitCount, uint8_t data, bool useNop)
{
    //if(bitCount <= 0 || bitCount > 8) return;
    
    // assumes correct display is selected
    for(int8_t i = bitCount - 1; i >= 0; --i)
    {
        bitBlast(clkPin, 0);
        bitBlast(dataPin, (data >> i) & 1);
        bitBlast(clkPin, 1);
    }
	
	if(useNop)
	{
		bitBlast(clkPin, 0);				//clk = 0 for data ready
		_nop();
		_nop();
		bitBlast(clkPin, 1);				//clk = 1 for data write into 1632
	}
}


// Writes out MSB first
void MatrixDisplay::preCommand()
{
   
	bitBlast(clkPin, 0);
    bitBlast(dataPin, 1);
	_nop();
	
	bitBlast(clkPin, 1);
	_nop();
	_nop();
	
	bitBlast(clkPin, 0);
    bitBlast(dataPin, 0);
	_nop();
	
	bitBlast(clkPin, 1);
	_nop();
	_nop();
	
	bitBlast(clkPin, 0);
    bitBlast(dataPin, 0);
	_nop();
	
	bitBlast(clkPin, 1);
	_nop();
	_nop();
}

void MatrixDisplay::bitBlast(uint8_t pin, uint8_t data)
{
    // TODO: Only supports 328
    if(pin < 14)
    {
        fWriteA(pin, data);
    }
    else
    {
        fWriteB(pin, data);
    }
}


uint8_t MatrixDisplay::getDisplayCount()
{
	return displayCount;
}

uint8_t MatrixDisplay::getDisplayHeight()
{
	return 8;
}

uint8_t MatrixDisplay::getDisplayWidth()
{
	return 32;
}

// Copy from the display buffer to the shadow buffer (takes a snapshot)
void MatrixDisplay::copyBuffer()
{
	if(pShadowBuffers==0) return;
	memcpy (pShadowBuffers, pDisplayBuffers, (backBufferSize * displayCount) );
}

void MatrixDisplay::shiftLeft()
{
	memcpy ( pDisplayBuffers, pDisplayBuffers+2, (backBufferSize * displayCount));
}

void MatrixDisplay::shiftRight()
{
	memcpy ( pDisplayBuffers+2, pDisplayBuffers, (backBufferSize * displayCount)-2);
}

void MatrixDisplay::setBrightness(uint8_t dispNum, uint8_t pwmValue)
{  
	// Check boundaries
	if(pwmValue > 15)  pwmValue = 15;
	else if(pwmValue < 0) pwmValue = 0;
	
	selectDisplay(dispNum);
	preCommand();
	writeDataBE(8,HT1632_CMD_PWM+pwmValue,true);
	releaseDisplay(dispNum);
}
