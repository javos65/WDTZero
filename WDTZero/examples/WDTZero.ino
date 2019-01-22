
#include <WDTZero.h>

byte wdtCounter, wdtCounterReset;
WDTZero MyWatchDoggy(WDT_HARDCYCLE4S); // Define WDT with hard-Watchdog @ 4 seconds - hardstop when processor is stalled. Configure your soft-counter at .Setup(). 

void setup() {
  int t = 20; //Initialize serial and wait for port to open, max 10 seconds
  Serial.begin(9600);
  while (!Serial) {
    delay(500);
    if ( (t--) == 0 ) break;
  }
 Serial.print("\nWDTZero-Demo : Setup Watchdog at 16Sec interval"); 
 MyWatchDoggy.setup(WDT_CYCLE16S);  // initialize WDT-softcounter refesh cycle on 16sec interval
}

void loop() {
  unsigned int t = 0;


  Serial.print("\nWatchdog Test1 - loop 30 seconds with a WDT.clear()\n");
  for (t = 1; t < 30; ++t) {
    MyWatchDoggy.clear();  // refresh wdt - before it loops
    delay(950);
    Serial.print(t);Serial.print(".");
  }

  Serial.print("\n\nWatchdog Test2 - Free run wait for reset\n");
  for (t = 1; t < 30; ++t) {
    delay(950);
    Serial.print(t);Serial.print(".");
    }
    
Serial.print("\nYou will never read this line");

}
