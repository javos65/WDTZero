
#include "Arduino.h"
#include "WDTZero.h"

int WDTZeroCounter;  // global counter for extended WDT via EW (early Warning) ISR

WDTZero::WDTZero(char wdtcycle)
{
      _wdtcycle=(wdtcycle&0x0f);    // only lower 4 bits are valid
}

void WDTZero::setup(int ewtcounter) {
  // One-time initialization of watchdog timer.
  // set software counter for ISR handling
  _ewtcounter=ewtcounter;

if (!_ewtcounter){                   // code 0x00 stops the watchdog
    NVIC_DisableIRQ(WDT_IRQn);       // disable IRQ
    NVIC_ClearPendingIRQ(WDT_IRQn);
    WDT->CTRL.bit.ENABLE = 0;        // Stop watchdog now!
    while(WDT->STATUS.bit.SYNCBUSY);
    }
else {
   WDTZeroCounter = _ewtcounter;          // SET Software EWT counter
   if (_wdtcycle<0x08) _wdtcycle=0x08;    // check WDT PER value  0x8 - 0xB
   if (_wdtcycle>0x0B) _wdtcycle=0x0B;    // check WDT PER value 0x8 - 0xB
  // Generic clock generator 2, divisor = 32 (2^(DIV+1)) 
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(4);
  // Enable clock generator 2 using low-power 32.768kHz oscillator.
  // With /32 divisor above, this yields 1024Hz clock.
  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) |
                      GCLK_GENCTRL_GENEN |
                      GCLK_GENCTRL_SRC_OSCULP32K |
                      GCLK_GENCTRL_DIVSEL;
  while(GCLK->STATUS.bit.SYNCBUSY);
  // WDT clock = clock gen 2
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT |
                      GCLK_CLKCTRL_CLKEN |
                      GCLK_CLKCTRL_GEN_GCLK2;

  // Enable WDT early-warning interrupt
  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);
  NVIC_SetPriority(WDT_IRQn, 0); // Top priority
  NVIC_EnableIRQ(WDT_IRQn);

  WDT->INTENSET.bit.EW     = 1;          // Enable early warning interrupt - enable the multi cycle mechanism via WDT_Handler()
  WDT->EWCTRL.bit.EWOFFSET = 0x06;       // Early Warning Interrupt Time Offset 0x6 - 512 clockcycles = 0.5 seconds => trigger ISR
  WDT->CONFIG.bit.PER      = _wdtcycle;  // Set period before hard WDT overflow <0x8 - 0xb>
  WDT->CTRL.bit.WEN        = 0;          // Disable window mode
  while(WDT->STATUS.bit.SYNCBUSY);       // Sync CTRL write
  WDT->CTRL.bit.ENABLE     = 1;          // Start watchdog now!
  while(WDT->STATUS.bit.SYNCBUSY);
}  
}

void WDTZero::clear() {
  WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;      // Clear WTD bit
  while(WDT->STATUS.bit.SYNCBUSY);
  WDTZeroCounter = _ewtcounter;         // Reset the early warning downcounter value
}

void WDT_Handler(void) {  // ISR for watchdog early warning, DO NOT RENAME!, need to clear
  if (WDTZeroCounter==0) {                   // Software EWT counter run out of time : Reset
         WDT->CLEAR.reg = 0xFF;              // value different than WDT_CLEAR_CLEAR_KEY causes reset
         while(true);
         }
 else {
        WDT->INTFLAG.bit.EW = 1;              // Clear INT EW Flag
        WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY; // Clear WTD bit
        while(WDT->STATUS.bit.SYNCBUSY); 
        WDTZeroCounter--;                     // EWT down counter, makes multi cycle WDT possible
       }
}