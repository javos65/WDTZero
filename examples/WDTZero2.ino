
#include <WDTZero.h>

WDTZero MyWatchDoggy; // Define WDT  

void setup() {
  int t = 20; //Initialize serial and wait for port to open, max 10 seconds
  Serial.begin(9600);
  while (!Serial) {
    delay(500);
    if ( (t--) == 0 ) break;
  }
 Serial.print("\nWDTZero-Demo : Setup Soft Watchdog at 32S interval"); 
 MyWatchDoggy.attachShutdown(myshutdown);
 MyWatchDoggy.setup(WDT_SOFTCYCLE32S);  // initialize WDT-softcounter refesh cycle on 32sec interval
}

void loop() {
  unsigned int t = 0;


  Serial.print("\nWatchdog Test1 - run 60 seconds with a WDT.clear()\n");
  for (t = 1; t < 60; ++t) {
    MyWatchDoggy.clear();  // refresh wdt - before it loops
    delay(950);
    Serial.print(t);Serial.print(".");
  }

  Serial.print("\n\nWatchdog Test2 - Free run wait for reset @32s\n");
  for (t = 1; t > 0; ++t) {
    delay(950);
    Serial.print(t);Serial.print(".");
    }
    
Serial.print("\nYou will never read this line");

}

void myshutdown()
{

Serial.print("\nWe gonna shut down ! ...");
  
}
