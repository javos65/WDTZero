# WDTZero V1.3.0
Allows to use the WatchDog functionality for Arduino Zero, MKRZero and MKR1000 only


 WDTZero.h - Library for watchdog on the SamD Zero series
 Created by John V. - 2019 V1.2.0
 
See ATSAMD2x datasheet, #18, page205 onwards
Defines Watchdog Class 'WDTZero' to Setup Watchdog timer in Normal mode with EarlyWarning interrupt.
  1. Sets Hardware WDT timer
     - for stalled processor situations, generates a reset 
     - 62ms upto 16 seconds response
  2. Sets Software WDT using EarlyWarning timer (EWT) and software counter (Global Variable)
     - for stalled software, generates a reset if software loop does not clear WDT on time
     - Total soft-WDT intervals are 8,16,32 Seconds ,1,2,4,8.16 Minutes
 
* V1.3.0 - added shutdown function for soft-watchdog : now possible to do an extra job before shutting down like saving your key data *thnks to Juraj

Released into the public domain.

