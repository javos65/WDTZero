# WDTZero V1.2.0
Allows to use the WatchDog functionality for Arduino Zero, MKRZero and MKR1000 only


 WDTZero.h - Library for watchdog on the SamD Zero series
 Created by John V. - 2019 V1.0.0
 
See ATSAMD2x datasheet, #18, page205 onwards
Defines Watchdog Class 'WDTZero' to Setup Watchdog timer in Normal mode with EarlyWarning interrupt.
  1. Sets Hardware WDT timer
     - for stalled processor situations, generates a reset 
     - 2, 4,8 or 16 seconds response
  2. Sets Software WDT using EarlyWarning timer (EWT) and software counter (Global Variable)
     - for stalled software, generates a reset if software loop does not clear WDT on time
     - EWT runs on 0.5 Sec interval ISR
     - Total soft-WDT intervals are 4,8,6,32 Seconds , 1,2,4,10 Minutes
 
Released into the public domain.

