//-----------------------------------------------------------------------------
// main.c
//-----------------------------------------------------------------------------
// Copyright 2014 Silicon Laboratories, Inc.
// http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
//
// Program Description:
//
// This program enumerates as a USB keyboard. Each time a button is pressed
// a character is sent to the host. A series of presses will spell out
// "HID Keyboard ". The status of the Caps Lock and Num Lock on the host will
// be indicated by the color of the LED.
//
// Resources:
// SYSCLK - 48 MHz HFOSC1 / 1
// USB0   - Full speed
// P0.2 - push button
// P0.3 - push button
// P2.3 - Display enable
//
//-----------------------------------------------------------------------------
// How To Test: EFM8UB1 STK
//-----------------------------------------------------------------------------
// 1) Place the switch in "AEM" mode.
// 2) Connect the EFM8UB1 STK board to a PC using a mini USB cable.
// 3) Compile and download code to the EFM8UB1 STK board.
//    In Simplicity Studio IDE, select Run -> Debug from the menu bar,
//    click the Debug button in the quick menu, or press F11.
// 4) Run the code.
//    In Simplicity Studio IDE, select Run -> Resume from the menu bar,
//    click the Resume button in the quick menu, or press F8.
// 5) The HID keyboard demo should start.
// 6) Connect a micro USB cable from the PC to the STK.
// 7) The device should enumerate on the PC as a HID keyboard.
// 8) Press either push-button (PB0 or PB1) to send one character in the
//    string "HID Keyboard ".
// 9) Pressing Caps Lock or Num Lock on the host keyboard will change the color
//    of the LED.
//
// Target:         EFM8UB1
// Tool chain:     Generic
//
// Release 0.1 (JM)
//    - Initial Revision
//    - 26 JAN 2015
//
#include "SI_EFM8UB1_Register_Enums.h"
#include "efm8_usb.h"
#include "descriptors.h"
#include "idle.h"
#include "InitDevice.h"
#include "astrokey.h"
#include "EFM8UB1_FlashUtils.h"

#include <stdint.h>

// ----------------------------------------------------------------------------
// Variables
// ----------------------------------------------------------------------------
// Number of keys currently being pressed by the macro
uint8_t keysPressed = 0;
// The current report to send the
KeyReport_TypeDef keyReport =
{
  0,
  0,
  {0, 0, 0, 0, 0, 0}
};

volatile bool keyReportSent = false;

// The data of the current macro
Macro_TypeDef SI_SEG_XDATA macro[MACRO_MAX_SIZE];
// Number of actions in current macro
uint8_t macroNumActions = 0;

// Index of current macro running (i.e. 0 for 1st key, etc.)
uint8_t macroIndex = NO_MACRO;

// Index of current action in current macro running;
uint8_t actionIndex = 0;

// Checks if a key is currently pressed by the macro
// Returns the index of the key in array of keys currently pressed,
// -1 if the key is not currently being pressed
int8_t keyIsPressed(uint8_t key)
{
  uint8_t i;
  for (i = 0; i < MACRO_MAX_KEYS; i++)
  {
    if (keyReport.keys[i] == key)
      return i;
  }
  return -1;
}

// Presses down a key
void pressKey(uint8_t key)
{
  if (keyIsPressed(key) == -1 && keysPressed < MACRO_MAX_KEYS)
  {
    keyReport.keys[keysPressed] = key;
    keysPressed++;
  }
}

// Releases a key currently being pressed
void releaseKey(uint8_t key)
{
  uint8_t keyIndex;
  if ((keyIndex = keyIsPressed(key)) != -1)
  {
    if (keysPressed == MACRO_MAX_KEYS)
    {
      keyReport.keys[MACRO_MAX_KEYS - 1] = 0;
    }
    else
    {
      keyReport.keys[keyIndex] = keyReport.keys[keysPressed - 1];
    }
    keysPressed--;
  }
}

// Advances the macro one action forward, ending it if the end is reached
void stepMacro()
{
  uint8_t actionType = macro[actionIndex].actionType;
  uint8_t value = macro[actionIndex].value;
  switch (actionType)
  {
    case MACRO_ACTION_DOWN:
      keyReport.keys[0] = value;
      //pressKey(value);
      break;
    case MACRO_ACTION_UP:
      keyReport.keys[0] = 0x00;
      //releaseKey(value);
      break;
  }

  keyReportSent = false;

  actionIndex++;
  if (actionIndex == macroNumActions)
  {
    macroIndex = NO_MACRO;
  }
}

void saveMacro(Macro_TypeDef* macroData, uint8_t saveIndex)
{
  FLADDR flashAddr = MACRO_FLASH_ADDR + (macroIndex * MACRO_BYTES);
  FLASH_Write(flashAddr, (uint8_t *) macroData, MACRO_BYTES);
}

void loadMacro()
{
  uint8_t i;

  macro[0].actionType = 1;
  macro[0].value = 0x04;
  macro[1].actionType = 2;
  macro[1].value = 0x04;
  macro[2].actionType = 1;
  macro[2].value = 0x16;
  macro[3].actionType = 2;
  macro[3].value = 0x16;
  macro[4].actionType = 1;
  macro[4].value = 0x17;
  macro[5].actionType = 2;
  macro[5].value = 0x17;
  macro[6].actionType = 1;
  macro[6].value = 0x15;
  macro[7].actionType = 2;
  macro[7].value = 0x15;
  macro[8].actionType = 1;
  macro[8].value = 0x12;
  macro[9].actionType = 2;
  macro[9].value = 0x12;
  macro[10].actionType = 1;
  macro[10].value = 0x0E;
  macro[11].actionType = 2;
  macro[11].value = 0x0E;
  macro[12].actionType = 1;
  macro[12].value = 0x08;
  macro[13].actionType = 2;
  macro[13].value = 0x08;
  macro[14].actionType = 1;
  macro[14].value = 0x1C;
  macro[15].actionType = 2;
  macro[15].value = 0x1C;

  macroNumActions = 16;

  //FLADDR flashAddr = MACRO_FLASH_ADDR + (macroIndex * MACRO_BYTES);
  //FLASH_Read((uint8_t *)macro, flashAddr, MACRO_BYTES);

  //macroNumActions = MACRO_MAX_SIZE;

  //for (i = 0; i < MACRO_MAX_SIZE; i++)
  //{
  //  if (macro[i].actionType == 0)
  //  {
  //    macroNumActions = i;
  //    break;
  //  }
  //}
}

// Starts running a macro
void startMacro(uint8_t index)
{
  macroIndex = index;
  actionIndex = 0;

  loadMacro();
  stepMacro();
}

uint8_t wasPressed = 0x00;

uint8_t checkKey(uint8_t bitMask, uint8_t pressed)
{
  uint8_t retVal = 0;

  if (pressed)
  {
    if (0 == wasPressed & bitMask)
      retVal = 1;
    wasPressed |= bitMask;
  }
  else
  {
    wasPressed &= ~bitMask;
  }

  return retVal;
}

// ----------------------------------------------------------------------------
// main() Routine
// ---------------------------------------------------------------------------
//
// Note: The software watchdog timer is not disabled by default in this
// example, so a long-running program will reset periodically unless
// the timer is disabled or your program periodically writes to it.
//
// Review the "Watchdog Timer" section under the part family's datasheet
// for details.
//
// ----------------------------------------------------------------------------
int16_t main(void)
{
  enter_DefaultMode_from_RESET();
  if (PRESSED(S0))
  {
    *((uint8_t SI_SEG_DATA *) 0x00) = 0xA5;
    RSTSRC = RSTSRC_SWRSF__SET | RSTSRC_PORSF__SET;
  }

  while (1)
  {
    if (PRESSED(S0) && PRESSED(S4))
    {
      *((uint8_t SI_SEG_DATA *) 0x00) = 0xA5;
      RSTSRC = RSTSRC_SWRSF__SET | RSTSRC_PORSF__SET;
    }

    // Macro currently running
    if (macroIndex != NO_MACRO)
    {
      if (keyReportSent)
        stepMacro();
    }
    // No macro running, scan switches
    else
    {
      if (checkKey(1 << 0, PRESSED(S0)))
        startMacro(0);
      else if (checkKey(1 << 1, PRESSED(S1)))
        startMacro(1);
      else if (checkKey(1 << 2, PRESSED(S2)))
        startMacro(2);
      else if (checkKey(1 << 3, PRESSED(S3)))
        startMacro(3);
      else if (checkKey(1 << 4, PRESSED(S4)))
        startMacro(4);
    }
  }
}
