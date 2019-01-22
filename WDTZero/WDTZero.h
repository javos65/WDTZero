/*
 * WDTZero.h - Library for watchdog on the SamD Zero series
 * Created by John V. - 2019
 * 
 * See ATSAMD2x datasheet, #18, page205 onwards
 * LDefines Watchdog Calss to Setup Watchdog timer in Normal mode with EarlyWarning interrupt.
 *  1. Sets Hardware WDT timer
 *     -> for stalled processor situations, generates a reset 
 *     -> 4,8 or 16 seconds response
 *  2. Sets Software WDT using EarlyWarning timer (EWT) and software counter (Global Variable)
 *     -> for stalled software, generates a reset if software loop does not clear WDT on time
 *     -> EWT runs on 0.5 Sec interval ISR
 *     -> Total soft-WDT intervals are 4,8,6,32 Seconds , 1,2,4,10 Minutes
 * 
 * Released into the public domain.
*/
#ifndef WDTZERO_H
#define WDTZERO_H

#include "Arduino.h"

# define WDT_HARDCYCLE1S  0x7   // WDT HARD cycle 2 Seconds
# define WDT_HARDCYCLE2S  0x8   // WDT HARD cycle 2 Seconds
# define WDT_HARDCYCLE4S  0x9   // WDT HARD cycle 4 Seconds
# define WDT_HARDCYCLE8S  0xA   // WDT HARD cycle 8 Seconds
# define WDT_HARDCYCLE16S 0xB   // WDT HARD cycle 16 Seconds

# define WDT_CYCLEOFF 0    // EW cycles = OFF Disable WDT
# define WDT_CYCLE1S  1    // EW cycles = 2 x 0.5 seconds ~ 1 seconds
# define WDT_CYCLE2S  3    // EW cycles = 4 x 0.5 seconds ~ 2 seconds
# define WDT_CYCLE4S  7    // EW cycles = 8 x 0.5 seconds ~ 4 seconds
# define WDT_CYCLE8S  15   // EW cycles = 16 x 0.5 seconds ~ 8 seconds
# define WDT_CYCLE16S 31   // EW cycles = 32 x 0.5 seconds ~ 16 seconds
# define WDT_CYCLE30S 59   // EW cycles = 60 x 0.5 seconds ~ 30 seconds
# define WDT_CYCLE1M 119   // EW cycles = 120 x 0.5 seconds ~ 60 seconds
# define WDT_CYCLE2M 239   // EW cycles = 240 x 0.5 seconds ~ 2 minutes
# define WDT_CYCLE4M 479   // EW cycles = 480 x 0.5 seconds ~ 4 minutes
# define WDT_CYCLE10M 1199 // EW cycles = 1200 x 0.5 seconds ~ 10 minutes

void WDT_Handler(void);
extern int WDTZeroCounter;

class WDTZero
{
  public:
    WDTZero(char wdtcycle);
    void clear();
    void setup(int ewtcounter);
  private:
     int _ewtcounter;       // soft counter integer early warning timer
     char _wdtcycle;        // hard counter one byte
};

#endif
