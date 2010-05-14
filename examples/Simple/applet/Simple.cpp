#include "MatrixDisplay.h"
#include "DisplayToolbox.h"

// Easy to use function
#define setMaster(dispNum, CSPin) initDisplay(dispNum,CSPin,true)
#define setSlave(dispNum, CSPin) initDisplay(dispNum,CSPin,false)

// 4 = Number of displays
// Data = 10
// WR == 11
// False - we dont need a shadow buffer for this example. saves 50% memory!

// Init Matrix
#include "WProgram.h"
void setup();
void loop();
MatrixDisplay disp(1,11,10, false);
// Pass a copy of the display into the toolbox
DisplayToolbox toolbox(&disp);

// Prepare boundaries
uint8_t X_MAX = 0;
uint8_t Y_MAX = 0;

void setup() {
  Serial.begin(9600); 

  // Fetch bounds
  X_MAX = disp.getDisplayCount() * (disp.getDisplayWidth()-1)+1;
  Y_MAX = disp.getDisplayHeight();
  
  // Prepare displays
  // The first number represents how the buffer/display is stored in memory. Could be useful for reorganising the displays or matching the physical layout
  // The number is a array index and is sequential from 0. You can't use 4-8. You must use the numbers 0-4
  disp.setMaster(0,4);
 // disp.setSlave(1,5);
 // disp.setSlave(2,6);
 // disp.setSlave(3,7);
}


void loop()
{

	/*
		The MatrixDisplay library treats each display individually. As a result you can set
		each display by it's seperate coordinates. For example:
		
		disp.setPixel(0, 5, 10, 1);
		
		 This sets display 0 (which you defined above)
		 Coordinate x = 5
		 Coordinate Y = 10
		 Turn the LEDs on = 1 (0 for off)
	
		Alternatively you may wish to use the ToolBox. The toolbox assumes that each of your displays 
		are set out horizontally. Display 0 is the first through n. Using the toolbox you can access the display as 
		if it were on big virtual display. For example:
		
		toolbox.setPixel(5, 46, 1);
		
		Coordinate x = 5
		Coordinate y = 46 (notice that's a virtual coordinate, larger than 23)
		Turn the Leds on = 1 (0 for off)
		
		Once you have set the pixels you'd like on. You need to sync the displays (basically write the buffer to the device).
		
		disp.syncDisplays();
		
		Alternatively there's a few tricks you can use. If you sync the displays, the ENTIRE buffer is written out - that is hugely slow (comparatively). 
		It may more efficient to write each pixel as you go. For example:
		
		Just as shown above but you can add a "paint" argument. If you set this to true the library will write the pixel straight to the display.
		You wont need to use disp.syncDisplays(); if you don't want to.
		toolbox.setPixel(5, 46, true);
	*/

	// Write directly to the display
	for(int y=0; y < Y_MAX; ++y)
	{
		for(int x = 0; x< X_MAX; ++x)
		{
			toolbox.setPixel(x, y, 1, true); // Lets write straight to the display. 
		}
	}
	
	delay(2000); // Wait two seconds
	
	// Okay lets clear the buffer
	disp.clear();
	// ...and write the result to the displays
	disp.syncDisplays();
	
	// Lets use syncDisplays now
		// Write directly to the display
	for(int y=0; y < Y_MAX; ++y)
	{
		for(int x = 0; x< X_MAX; ++x)
		{
			toolbox.setPixel(x, y, 1); // Notice we've discarded the "true". This means we're no longer writing to the display directly
		}
	}
	
	// Now we've written to the back buffer, lets write out the result to the display
	disp.syncDisplays(); 
	
	// Now we're here. Why don't we try out another simple function. SetBrightness
	for(int i=0; i<16; ++i) // The displays have 15 different brightness settings
	{
		// This will set the brightness for ALL displays
		toolbox.setBrightness(i);
		// Alternatively you could set them individually
		// disp.setBrightness(displayNumber, i);
		 delay(200); // Let's wait a bit or you'll miss it!
	}
	
	
	
	// Okay lets clear the buffer
	disp.clear();
	// ...and write the result to the displays
	disp.syncDisplays();
	
	// We're all done. let's start looping
}

int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

