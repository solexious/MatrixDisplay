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
MatrixDisplay disp(4,11,10, false);
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

  // Setup diagnostic LED  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  // Prepare displays
  disp.setMaster(0,4);
  disp.setSlave(1,5);
  disp.setSlave(2,6);
  disp.setSlave(3,7);
}

// Response codes
#define RSP_READY 1
#define RSP_CONF 2
#define RSP_UNK 3
#define RSP_NOTRDY 4

// Commands we understand
#define CMD_DRAWLINE 1
#define CMD_HELLO 2
#define CMD_CLEAR 3
#define CMD_SHIFTLEFT 4
#define CMD_SHIFTRIGHT 5
#define CMD_GETWIDTH 6
#define CMD_GETHEIGHT 7

// An error flag
bool dataError = false;


// Blinken lights! Good for diagnostics
void blink()
{
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW); 
}


void loop ()
{
  // Check we've got data
  if(Serial.available())
  {
    // First peice should be the command
    byte cmd = Serial.read();      

    // What does the host want us to do?
    switch(cmd)
    {
    case CMD_HELLO:
      Serial.write(RSP_CONF); // Return Understood
      blink();
      break;
    case CMD_DRAWLINE:
      {
        int uBound = Y_MAX-getData(0)-1; // Get line top (aka 16)
        int lBound = Y_MAX-getData(0); // Get line low (aka 0)
        int yHeight = Y_MAX-getData(0); // Get line height (aka 5)
        int x = getData(0); // Get line height (aka 24)

        if(dataError)
        { 
          dataError = false;
          return;
        }

        // Draw each pixel from the bottom of the display to the top
        // Light up a pixel when yHeight is hit
        int val = 0;
        // Hard coded to 15 and 0 while debugging (should be y=uBound; y>lBound
        for(int y=15; y>0; y--)
        {

          // Check which pixel to light
          if(yHeight < y){ 
            val = 1;
          }
          else{
            val =0; 
          }
          
          // Set a pixel in the back buffer
          toolbox.setPixel(x, y, val);
        }

       // Render our changes (don't bother rending if we're at X_MAX as we're only going to shift anyway
       if(x+1<X_MAX) disp.syncDisplays();

        Serial.write(RSP_CONF); // Return Understood
      }
      break;
    case  CMD_SHIFTLEFT:
      {
        disp.shiftLeft();
        disp.syncDisplays(); // Render our changes
        Serial.write(RSP_CONF);  // Return Understood
      }
      break;
    case  CMD_CLEAR:
      {
        disp.clear(); // Nuke the display
        Serial.write(RSP_CONF); // Return understood
        disp.syncDisplays(); 
      }
    case CMD_GETWIDTH:
      {
        // How wide is our display?
        Serial.write(X_MAX);
      }
    case CMD_GETHEIGHT:
      {
        // How high is our display
        Serial.write(Y_MAX); 
      }
    default:
      
      // Say sorry we dont understand
      Serial.write(RSP_UNK);
    }




  }
}


// Fetch data with delay
byte getData(int timeout)
{
  for(int i=0;i<(timeout<=0) ? 50 : timeout; i++)
  {
    while(!Serial.available())
    { 
      delay(1); 
    } // Wait for data 

    if(!Serial.available()) dataError = true;

    return Serial.read();
  }
}











