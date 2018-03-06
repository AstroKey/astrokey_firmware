//-----------------------------------------------------------------------------
// delay.c
//-----------------------------------------------------------------------------
// Copyright 2018 AstroKey
// https://github.com/AstroKey/astrokey_firmware/blob/master/LICENSE
//
// File Description:
//
// Implementation of the millisecond counter.
//

#include "SI_EFM8UB1_Register_Enums.h"
#include "delay.h"

static volatile uint32_t millis = 0;

uint32_t getMillis()
{
  return millis;
}

void resetMillis()
{
  millis = 0;
}

SI_INTERRUPT(timer2ISR, TIMER2_IRQn)
{
  // Increment millisecond counter
  millis++;
  // Clear interrupt flag
  TMR2CN0 &= ~(TMR2CN0_TF2H__SET | TMR2CN0_TF2L__SET);
}
