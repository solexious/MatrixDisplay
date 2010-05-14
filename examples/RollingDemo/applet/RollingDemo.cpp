#include "MatrixDisplay.h"
#include "DisplayToolbox.h"
#include "font.h"

#define DEMOTIME 30000  // 30 seconds max on each demo is enough.
#define DISPDELAY 100    // Each "display" lasts this long
#define LONGDELAY 1000  // This delay BETWEEN demos

// Macro to make it the initDisplay function a little easier to understand
#define setMaster(dispNum, CSPin) initDisplay(dispNum,CSPin,true)
#define setSlave(dispNum, CSPin) initDisplay(dispNum,CSPin,false)

// 4 = Number of displays
// Data = 10/
// WR == 11
// True. Do you want a shadow buffer? (A scratch pad)

// Init Matrix
#include "WProgram.h"
void setup();
void loop();
void demo_life ();
void demo_bouncyline ();
void demoText();
void demoBouncyCircle();
void drawChar(uint8_t x, uint8_t y, char c);
void drawString(uint8_t x, uint8_t y, char* c);
MatrixDisplay disp(1,11,10, true);
// Pass a copy of the display into the toolbox
DisplayToolbox toolbox(&disp);

// Prepare boundaries
uint8_t X_MAX = 0;
uint8_t Y_MAX = 0;

void setup() {
  // Fetch bounds (dynamically work out how large this display is)
  X_MAX = disp.getDisplayCount() * (disp.getDisplayWidth()-1)+1;
  Y_MAX = disp.getDisplayHeight();

  // Prepare displays
  disp.setMaster(0,4);
 // disp.setSlave(1,5);
 // disp.setSlave(2,6);
 // disp.setSlave(3,7);
}



void loop() {
   demoText(); // Bouncy hello

  demoBouncyCircle(); 

  demo_bouncyline(); // Bouncy line
  
  demo_life(); // Basic life demo
}


/*
 * demo_life
 * Run the "life" game for a while, demonstrating the
 * ability of the AVR to update every pixle of the display
 * after having done some computation to figure out the new
 * value.  Also demonstrates the use of the snapshot ram.
 */
void demo_life ()
{
  byte x,y, neighbors, newval;

  toolbox.setPixel(10,3,1);  // Plant an "acorn"; a simple pattern that
  toolbox.setPixel(12,4,1); //  grows for quite a while..
  toolbox.setPixel(9,5,1);
  toolbox.setPixel(10,5,1);
  toolbox.setPixel(13,5,1);
  toolbox.setPixel(14,5,1);
  toolbox.setPixel(15,5,1);

  delay(LONGDELAY);   // Play life
  disp.copyBuffer(); // Copy the back buffer into the shadow buffer (basically create a backup of the CURRENT display)

  for (int i=0; i < (DEMOTIME/DISPDELAY)/4; i++) {
    for (x=1; x < X_MAX; x++) {
      for (y=1; y < Y_MAX; y++) {
        neighbors = toolbox.getPixel(x, y+1, true) +
          toolbox.getPixel(x, y-1, true) +
          toolbox.getPixel(x+1, y, true) +
          toolbox.getPixel(x+1, y+1, true) +
          toolbox.getPixel(x+1, y-1, true) +
          toolbox.getPixel(x-1, y, true) +
          toolbox.getPixel(x-1, y+1, true) +
          toolbox.getPixel(x-1, y-1, true);

        switch (neighbors) {
        case 0:
        case 1:
          newval = 0;   // death by loneliness
          break;
        case 2:
          newval = toolbox.getPixel(x,y, true); // Fetch pixel from the SHADOW buffer
          break;  // remains the same
        case 3:
          newval = 1;
          break;
        default:
          newval = 0;  // death by overcrowding
          break;
        }

        toolbox.setPixel(x,y, newval);
      }
    }
    // Write out display
    disp.syncDisplays(); 

    // Copy buffer
    disp.copyBuffer(); // Copy the back buffer into the shadow buffer (basically create a backup of the CURRENT display)

    delay(DISPDELAY);
  }
}




/*
 * demo_bouncyline
 * Do the classic "bouncing line" demo, where the endpoints of a line
 * move independently and bounce off the edges of the display.
 * This should demonstrate (more or less) the performance limits of
 * the line drawing function.
 */
void demo_bouncyline ()
{
  char x1,y1, x2,y2, dx1, dy1, dx2, dy2;

  disp.clear();
  x1 = random(0,X_MAX);
  x2 = random(0,X_MAX);
  y1 = random(0,Y_MAX);
  y2 = random(0,Y_MAX);
  dx1 = random(1,4);
  dx2 = random(1,4);
  dy1 = random(1,4);
  dy2 = random(1,4);
  for (int i=0; i < DEMOTIME/DISPDELAY; i++) {
    toolbox.drawLine(x1,y1, x2,y2, 1);
    disp.syncDisplays(); 
    delay(DISPDELAY);
    toolbox.drawLine(x1,y1, x2,y2, 0);

    x1 += dx1;
    if (x1 > X_MAX) {
      x1 = X_MAX;
      dx1 = -random(1,4);
    } 
    else if (x1 < 0) {
      x1 = 0;
      dx1 = random(1,4);
    }

    x2 += dx2;
    if (x2 > X_MAX) {
      x2 = X_MAX;
      dx2 = -random(1,4);
    } 
    else if (x2 < 0) {
      x2 = 0;
      dx2 = random(1,4);
    }

    y1 += dy1;
    if (y1 > Y_MAX) {
      y1 = Y_MAX;
      dy1 = -random(1,3);
    } 
    else if (y1 < 0) {
      y1 = 0;
      dy1 = random(1,3);
    }

    y2 += dy2;
    if (y2 > Y_MAX) {
      y2 = Y_MAX;
      dy2 = -random(1,3);
    } 
    else if (y2 < 0) {
      y2 = 0;
      dy2 = random(1,3);
    }
  }
}

// Text bouncing around
void demoText()
{
  int y=Y_MAX-7;
  int x=X_MAX;
  boolean textDir = false;
  boolean textRight = false;
  for (int i=0; i < (DEMOTIME/DISPDELAY)/4; i++) 
  {
    if(y<=0) textDir = true;
    else if(y>=(Y_MAX-7)) textDir = false; 


    if(x>=X_MAX) textRight = false;
    else if(x<=0) textRight = true;

    if(textDir) y++;
    else y--;

    if(textRight) x++;
    else x--;

    drawString(x,y,"Hello");
    disp.syncDisplays(); 

    delay(100);
    disp.clear(); 
  } 
}




// Small circle bouncing around the bounds
void demoBouncyCircle()
{
  int radius = 3; 
  int y=Y_MAX-(radius*2);
  int x=X_MAX;

  boolean textDir = false;
  boolean textRight = false;
  for (int i=0; i < (DEMOTIME/DISPDELAY)/4; i++) 
  {
    if(y<=radius) textDir = true;
    else if(y>=(Y_MAX-radius)) textDir = false; 


    if(x>=X_MAX) textRight = false;
    else if(x-24<=0) textRight = true;

    if(textDir) y++;
    else y--;

    if(textRight) x++;
    else x--;

    toolbox.drawCircle(x, y, radius);
    disp.syncDisplays(); 

    delay(100);
    disp.clear(); 
  }  

}



/*
 * Copy a character glyph from the myfont data structure to
 * display memory, with its upper left at the given coordinate
 * This is unoptimized and simply uses setPixel() to draw each dot.
 */
void drawChar(uint8_t x, uint8_t y, char c)
{
  uint8_t dots;
  if (c >= 'A' && c <= 'Z' ||
    (c >= 'a' && c <= 'z') ) {
    c &= 0x1F;   // A-Z maps to 1-26
  } 
  else if (c >= '0' && c <= '9') {
    c = (c - '0') + 27;
  } 
  else if (c == ' ') {
    c = 0; // space
  }
  for (char col=0; col< 5; col++) {
    dots = pgm_read_byte_near(&myfont[c][col]);
    for (char row=0; row < 7; row++) {
      if (dots & (64>>row))   	     // only 7 rows.
        toolbox.setPixel(x+col, y+row, 1);
      else 
        toolbox.setPixel(x+col, y+row, 0);
    }
  }
}


// Write out an entire string (Null terminated)
void drawString(uint8_t x, uint8_t y, char* c)
{
	for(char i=0; i< strlen(c); i++)
	{
		drawChar(x, y, c[i]);
		x+=6; // Width of each glyph
	}
}

int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

