/*
 * WDTZero.h - Library for watchdog on the SamD Zero series
 * Created by John V. - 2019 V 1.3.0
 * 
 * See ATSAMD2x datasheet, #18, page205 onwards
 * Defines Watchdog Class to Setup Watchdog timer in Normal mode with EarlyWarning interrupt.
 *  1. Sets Hardware WDT timer
 *     -> for stalled processor situations, generates a reset 
 *     -> 62ms upto 16s response
 *  2. Sets Software WDT using EarlyWarning timer (EWT) and software counter (Global Variable)
 *     -> for stalled software, generates a reset if software loop does not clear WDT on time
 *     -> Total soft-WDT intervals are 8,16,32 Seconds , 1,2,4,8,16 Minutes
 * 
 * V1.3.0 - added shutdown function for soft-watchdog : now possible to do an extra job before shutting down like saving your key data *thnks to Juraj
 * 
 * Released into the public domain.
*/
#ifndef WDTZERO_H
#define WDTZERO_H
//#define DBGON

#include "Arduino.h"

//        ATSAMD2x datasheet, #18, page205 onwards for Register details
//        Software Configured is the following variable 'ewtcounter', passing 16 bits datas, paired in 4x4
//        w=CNT[3]EWen[1]  x=DIV[4]  y=PER[4]  z=EW[4]   
//        WARNING 1 : > PER[4] MUST BE LARGER THAN EW[4] ! -> otherise HARD RESET IS QUICKER THAN THE EW-ISR ROUTINE
//        WARNING 2 : > Changing DIV[] CAN SCREW UP YOUR RTC !!!, best left at 0x4
//
//                     HeX[0wxyz]     
# define WDT_OFF           0x0000    // WDT off

# define WDT_HARDCYCLE62m  0x0430    // WDT HARD only : 64 clockcycles @ 1024hz = 62.5ms
# define WDT_HARDCYCLE250m 0x0450    // WDT HARD only : 256 clockcycles @ 1024hz = 250ms
# define WDT_HARDCYCLE1S   0x0470    // WDT HARD only : 1024 clockcycles @ 1024hz = 1 seconds
# define WDT_HARDCYCLE2S   0x0480    // WDT HARD only : 2048 clockcycles @ 1024hz = 2 seconds
# define WDT_HARDCYCLE4S   0x0490    // WDT HARD cycle 4 Seconds
# define WDT_HARDCYCLE8S   0x04A0    // WDT HARD cycle 8 Seconds
# define WDT_HARDCYCLE16S  0x04B0    // WDT HARD cycle 16 Seconds

# define WDT_SOFTCYCLE8S   0x14BA    // EW on, @8192cycles/1024 x  2^0 = 8S (HW WDT runs on 16s)
# define WDT_SOFTCYCLE16S  0x54B9    // EW on, @4096cycles/1024 x  2^2 = 16S      "   
# define WDT_SOFTCYCLE32S  0x54BA    // EW on, @8192cycles/1024 x  2^2  = 32S     "      010 1 
# define WDT_SOFTCYCLE1M   0x74BA    // EW on, @8192cycles/1024 x  2^3  = 64S     "      011 1
# define WDT_SOFTCYCLE2M   0x94BA    // EW on, @8192cycles/1024 x  2^4 = 128S     "      100 1 -> 2.1 minutes to be precise ...
# define WDT_SOFTCYCLE4M   0xB4BA    // EW on, @8192cycles/1024 x  2^5 = 256S     "      101 1 -> 4.2 minutes to be precise ...
# define WDT_SOFTCYCLE8M   0xD4BA    // EW on, @8192cycles/1024 x  2^6 = 512S     "      110 1 -> 8.5 minutes to be precise ...
# define WDT_SOFTCYCLE16M  0xF4BA    // EW on, @8192cycles/1024 x  2^7 = 1024S    "      111 1 -> 17  minutes to be precise ...


void WDT_Handler(void);         // ISR HANDLER FOR WDT EW INTERRUPT
extern int WDTZeroCounter;      // SOFT COUNTER FOR EXTENDING WDT TIME VIA EW INTERRUPT

class WDTZero
{
  public:
    WDTZero();
    void clear();
    void setup(unsigned int wdtzerosetup);  
    void attachShutdown(voidFuncPtr callback);
    void detachShutdown();
  private:                      // wdtsetup varaible is split out in Hex[wxyx] as follows :
     unsigned int _ewtcounter;  // w->  bit3:1 soft counter for early warning timer Cyclese = 2^[3:1] -> 1x,2x, ... 128x
     char _w;                   // w -> bit0 : EWen = early warning interrupt enable : activates ISR for EW handling
     char _x;                   // x -> 32Khz clock divisor DIV[3:0], 4 bits 0x1 - 0X4 -> Divisor = (2^(x+1)) i.e. x=4 -> DIV=32 CLK = 32Khz/32=1024Hz
     char _y;                   // y -> hard counter PER[3:0], 4 bits, value 0x1 - 0XB -> clock cycle count
     char _z;                   // z -> EW offset EW[3:0], 4 bits, valaue 0x1 - 0xB -> clock cycle count
};

#endif
