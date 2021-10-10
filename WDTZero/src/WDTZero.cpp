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

#include "Arduino.h"
#include "WDTZero.h"

voidFuncPtr WDT_Shutdown = NULL;
int WDTZeroCounter;  // global counter for extended WDT via EW (early Warning) ISR

WDTZero::WDTZero()
{
}

void WDTZero::setup(unsigned int wdtzerosetup) {
  // One-time initialization of watchdog timer.

  if (!wdtzerosetup){                            // code 0x00 stops the watchdog
    NVIC_DisableIRQ(WDT_IRQn); // disable IRQ
    NVIC_ClearPendingIRQ(WDT_IRQn);

#if defined(__SAMD51__)
    WDT->CTRLA.bit.ENABLE = 0; // Stop watchdog now!
    while (WDT->SYNCBUSY.reg)
      ;
#else
    WDT->CTRL.bit.ENABLE = 0; // Stop watchdog now!
    while (WDT->STATUS.bit.SYNCBUSY)
      ;
#endif
  }
  else
  {
    //Split out the bits wdtzerosetup =>  _ewtcounter=CNT[3] _w=EWen[1]  _x=DIV[4]  _y=PER[4]  _z=EW[4]
#ifdef DBGON
    Serial.print(wdtzerosetup,HEX); Serial.print("= w/");
    Serial.print(wdtzerosetup, HEX);
    Serial.print("= w/");
#endif

    _z = (char)(wdtzerosetup & 0x000f);                    // lower quad bits = EWoffset
    _y = (char)((wdtzerosetup >> 4) & 0x000f);             // second quad bits = PER
    _x = (char)((wdtzerosetup >> 8) & 0x000f);             // third quad bits = DIV -> WARNING: Changing DIV CAN SCREW UP YOUR RTC !!!, best left at 0x4
    _w = (char)((wdtzerosetup >> 12) & 0x0001);            // upper quad bits - bit[0] = EW enable
    _ewtcounter = 0x01 << ((wdtzerosetup >> 13) & 0x0007); // make new software EWT counter value : upper quad :  3 bits -> 2^ bit[3:1]
    WDTZeroCounter = _ewtcounter;                          // SET Software EWT counter - used in ISR and Clear functions

#ifdef DBGON
    Serial.print(_w, HEX);
    Serial.print(" x/");
    Serial.print(_x, HEX);
    Serial.print(" y/");
    Serial.print(_y, HEX);
    Serial.print(" z/");
    Serial.print(_z, HEX);
    Serial.print("\nNew Ewtcounter:");
    Serial.print(_ewtcounter, HEX);
#endif

#if defined(__SAMD51__)
    // SAMD51 WDT uses OSCULP32k as input clock now
    // section: 20.5.3
    OSC32KCTRL->OSCULP32K.bit.EN1K = 1;  // Enable out 1K (for WDT)
    OSC32KCTRL->OSCULP32K.bit.EN32K = 0; // Disable out 32K

    // Enable WDT early-warning interrupt
    NVIC_DisableIRQ(WDT_IRQn);
    NVIC_ClearPendingIRQ(WDT_IRQn);
    NVIC_SetPriority(WDT_IRQn, 0); // Top priority
    NVIC_EnableIRQ(WDT_IRQn);

    while (WDT->SYNCBUSY.reg)
      ;

    USB->DEVICE.CTRLA.bit.ENABLE = 0; // Disable the USB peripheral
    while (USB->DEVICE.SYNCBUSY.bit.ENABLE)
      ;                                 // Wait for synchronization
    USB->DEVICE.CTRLA.bit.RUNSTDBY = 0; // Deactivate run on standby
    USB->DEVICE.CTRLA.bit.ENABLE = 1;   // Enable the USB peripheral
    while (USB->DEVICE.SYNCBUSY.bit.ENABLE)
      ; // Wait for synchronization

    WDT->INTFLAG.bit.EW = 1;
    WDT->INTENSET.bit.EW = _w; // Enable early warning interrupt - enable the multi cycle mechanism via WDT_Handler()

    WDT->EWCTRL.bit.EWOFFSET = _z; // Early Warning Interrupt Time Offset 0x6 - 512 clockcycles = 0.5 seconds => trigger ISR
    WDT->CONFIG.bit.PER = _y;      // Set period before hard WDT overflow <0x8 - 0xb>
    WDT->CTRLA.bit.WEN = 0;        // Disable window mode
    while (WDT->SYNCBUSY.reg)
      ;                        // Sync CTRL write
    WDT->CTRLA.bit.ENABLE = 1; // Start watchdog now!
    while (WDT->SYNCBUSY.reg)
      ;
#else
    // Generic clock generator 2, divisor = 32 (2^(DIV+1))  = _x
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(_x);
    // Enable clock generator 2 using low-power 32.768kHz oscillator.
    // With /32 divisor above, this yields 1024Hz clock.
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) |
                        GCLK_GENCTRL_GENEN |
                        GCLK_GENCTRL_SRC_OSCULP32K |
                        GCLK_GENCTRL_DIVSEL;
    while (GCLK->STATUS.bit.SYNCBUSY)
      ;
    // WDT clock = clock gen 2
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT |
                        GCLK_CLKCTRL_CLKEN |
                        GCLK_CLKCTRL_GEN_GCLK2;

    // Enable WDT early-warning interrupt
    NVIC_DisableIRQ(WDT_IRQn);
    NVIC_ClearPendingIRQ(WDT_IRQn);
    NVIC_SetPriority(WDT_IRQn, 0); // Top priority
    NVIC_EnableIRQ(WDT_IRQn);

    WDT->INTENSET.bit.EW = _w;     // Enable early warning interrupt - enable the multi cycle mechanism via WDT_Handler()
    WDT->EWCTRL.bit.EWOFFSET = _z; // Early Warning Interrupt Time Offset 0x6 - 512 clockcycles = 0.5 seconds => trigger ISR
    WDT->CONFIG.bit.PER = _y;      // Set period before hard WDT overflow <0x8 - 0xb>
    WDT->CTRL.bit.WEN = 0;         // Disable window mode
    while (WDT->STATUS.bit.SYNCBUSY)
      ;                       // Sync CTRL write
    WDT->CTRL.bit.ENABLE = 1; // Start watchdog now!
    while (WDT->STATUS.bit.SYNCBUSY)
      ;
#endif
  }
}

void WDTZero::clear()
{
#if defined(__SAMD51__)
  while (WDT->SYNCBUSY.reg)
    ;
#else
  while (WDT->STATUS.bit.SYNCBUSY)
    ;
#endif
  WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;

  WDTZeroCounter = _ewtcounter; // Reset the early warning downcounter value
}

void WDT_Handler(void)
{                   // ISR for watchdog early warning, DO NOT RENAME!, need to clear
  WDTZeroCounter--; // EWT down counter, makes multi cycle WDT possible
  if (WDTZeroCounter <= 0)
  { // Software EWT counter run out of time : Reset
    if (WDT_Shutdown != NULL)
      WDT_Shutdown();      // run extra Shutdown functions if defined
    WDT->CLEAR.reg = 0xFF; // value different than WDT_CLEAR_CLEAR_KEY causes reset
    while (true)
      ;
  }
  else
  {
    WDT->INTFLAG.bit.EW = 1;              // Clear INT EW Flag
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY; // Clear WTD bit
#if defined(__SAMD51__)
    while (WDT->SYNCBUSY.reg)
      ;
#else
    while (WDT->STATUS.bit.SYNCBUSY)
      ;
#endif
  }
}

void WDTZero::attachShutdown(voidFuncPtr callback)
{
  WDT_Shutdown = callback;
}

void WDTZero::detachShutdown()
{
  WDT_Shutdown = NULL;
}