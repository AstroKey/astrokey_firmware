//-----------------------------------------------------------------------------
// main.c
//-----------------------------------------------------------------------------
// Copyri999ght 2014 Silicon Laboratories, Inc.
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
#include "delay.h"
#include "EFM8UB1_FlashPrimitives.h"
#include "EFM8UB1_FlashUtils.h"

#include <stdint.h>

// ----------------------------------------------------------------------------
// Variables
// ----------------------------------------------------------------------------
// Number of keys currently being pressed by the workflow
uint8_t keysPressed = 0;
// The current report to send the
volatile KeyReport_TypeDef keyReport =
{
  0,
  0,
  {0, 0, 0, 0, 0, 0}
};

volatile bool keyReportSent = false;
volatile int8_t workflowUpdated = -1;
Action_TypeDef SI_SEG_XDATA tmpWorkflow[WORKFLOW_MAX_SIZE];

// The data of the current workflow
Action_TypeDef SI_SEG_XDATA workflow[WORKFLOW_MAX_SIZE];

// Index of current workflow running (i.e. 0 for 1st key, etc.)
uint8_t workflowIndex = NO_WORKFLOW;

// Index of current action in current workflow running;
uint8_t actionIndices[NUM_SWITCHES] = {0};

// Checks if a key is currently pressed by the workflow
// Returns the index of the key in array of keys currently pressed,
// -1 if the key is not currently being pressed
int8_t keyIsPressed(uint8_t key)
{
  uint8_t i;
  for (i = 0; i < WORKFLOW_MAX_KEYS; i++)
  {
    if (keyReport.keys[i] == key)
      return i;
  }
  return -1;
}

// Presses down a key
void pressKey(uint8_t key)
{
  if (keyIsPressed(key) == -1 && keysPressed < WORKFLOW_MAX_KEYS)
  {
    keyReport.keys[keysPressed] = key;
    keysPressed++;
  }
  // Switch case seems to use 1 more byte of code than cascaded if else ifs with many conditions,
  // but runs faster
  switch (key)
  {
    case USAGE_LEFTCTRL:
      keyReport.modifiers |= MODIFIER_LEFTCTRL;
      break;
    case USAGE_LEFTSHIFT:
      keyReport.modifiers |= MODIFIER_LEFTSHIFT;
      break;
    case USAGE_LEFTALT:
      keyReport.modifiers |= MODIFIER_LEFTALT;
      break;
    case USAGE_LEFTGUI:
      keyReport.modifiers |= MODIFIER_LEFTGUI;
      break;
  }
}

// Releases a key currently being pressed
void releaseKey(uint8_t key)
{
  uint8_t keyIndex;
  if ((keyIndex = keyIsPressed(key)) != -1)
  {
    // Switch last key pressed to position of key being released
    keyReport.keys[keyIndex] = keyReport.keys[keysPressed - 1];
    // Release last key
    keyReport.keys[keysPressed - 1] = 0;
    keysPressed--;
  }
  switch (key)
  {
    case USAGE_LEFTCTRL:
      keyReport.modifiers &= ~MODIFIER_LEFTCTRL;
      break;
    case USAGE_LEFTSHIFT:
      keyReport.modifiers &= ~MODIFIER_LEFTSHIFT;
      break;
    case USAGE_LEFTALT:
      keyReport.modifiers &= ~MODIFIER_LEFTALT;
      break;
    case USAGE_LEFTGUI:
      keyReport.modifiers &= ~MODIFIER_LEFTGUI;
      break;
  }
}

bool curPressDown = false;
bool delayStarted = false;
uint32_t delayStartTime;

// Advances the workflow one action forward, ending it if the end is reached
void stepWorkflow()
{
  uint8_t actionType = workflow[actionIndices[workflowIndex]].actionType;
  uint8_t value = workflow[actionIndices[workflowIndex]].value;
  switch (actionType)
  {
    case WORKFLOW_ACTION_DOWN:
      pressKey(value);
      actionIndices[workflowIndex]++;
      break;
    case WORKFLOW_ACTION_UP:
      releaseKey(value);
      actionIndices[workflowIndex]++;
      break;
    case WORKFLOW_ACTION_PRESS:
      if (curPressDown)
      {
        releaseKey(value);
        curPressDown = false;
        actionIndices[workflowIndex]++;
      }
      else
      {
        pressKey(value);
        curPressDown = true;
      }
      break;
    case WORKFLOW_ACTION_DELAY:
      if (!delayStarted)
      {
        delayStarted = true;
        delayStartTime = getMillis();
      }
      else if ((getMillis() - delayStartTime) > ((uint32_t)value * 100))
      {
        delayStarted = false;
        actionIndices[workflowIndex]++;
      }
      break;
    default:
      actionIndices[workflowIndex]++;
      break;
  }

  keyReportSent = false;

  if (actionType == 0x00 ||
      actionIndices[workflowIndex] == WORKFLOW_MAX_SIZE)
  {
    workflowIndex = NO_WORKFLOW;
  }
  else if (actionType == WORKFLOW_ACTION_PAUSE)
  {
    actionIndices[workflowIndex]++;
    workflowIndex = NO_WORKFLOW;
  }
}

void saveWorkflow(Action_TypeDef* workflowData, uint8_t saveIndex)
{
  uint8_t i;
  FLADDR flashAddr = WORKFLOW_FLASH_ADDR + (saveIndex * WORKFLOW_BYTES);
  for (i = 0; i < WORKFLOW_PAGES; i++)
    FLASH_PageErase(flashAddr + (USER_PAGE_SIZE * i));
  FLASH_Write(flashAddr, (uint8_t*) workflowData, WORKFLOW_BYTES);
}

void loadWorkflow(Action_TypeDef* workflowData, uint8_t loadIndex)
{
  FLADDR flashAddr = WORKFLOW_FLASH_ADDR + (loadIndex * WORKFLOW_BYTES);
  FLASH_Read((uint8_t *)workflowData, flashAddr, WORKFLOW_BYTES);
}

// Starts running a workflow
void startWorkflow(uint8_t index)
{
  workflowIndex = index;
  actionIndices[workflowIndex] = 0;

  loadWorkflow(workflow, index);
  stepWorkflow();
}

void resumeWorkflow(uint8_t index)
{
  workflowIndex = index;

  loadWorkflow(workflow, index);
  stepWorkflow();
}

uint8_t wasPressed = 0x00;

uint8_t checkKeyPressed(uint8_t bitMask, uint8_t pressed)
{
  uint8_t retVal = 0;

  if (pressed)
  {
    if (0 == (wasPressed & bitMask))
      retVal = 1;
    wasPressed |= bitMask;
  }

  return retVal;
}

uint8_t checkKeyReleased(uint8_t bitMask, uint8_t pressed)
{
  uint8_t retVal = 0;

  if (!pressed)
  {
    if (wasPressed & bitMask)
      retVal = 1;
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
  /*if (PRESSED(S0))
  {
    *((uint8_t SI_SEG_DATA *) 0x00) = 0xA5;
    RSTSRC = RSTSRC_SWRSF__SET | RSTSRC_PORSF__SET;
  }*/

  while (1)
  {
    if (PRESSED(S0) && PRESSED(S4))
    {
      *((uint8_t SI_SEG_DATA *) 0x00) = 0xA5;
      RSTSRC = RSTSRC_SWRSF__SET | RSTSRC_PORSF__SET;
    }

    // Workflow currently running
    if (workflowIndex != NO_WORKFLOW)
    {
      if (keyReportSent)
        stepWorkflow();
    }
    // No workflow running, scan switches
    else
    {
      if (workflowUpdated != -1)
      {
        saveWorkflow(tmpWorkflow, workflowUpdated);
        workflowUpdated = -1;
      }

      if (checkKeyPressed(1 << 0, PRESSED(S0)))
        startWorkflow(0);
      else if (checkKeyReleased(1 << 0, PRESSED(S0)))
        resumeWorkflow(0);

      else if (checkKeyPressed(1 << 1, PRESSED(S1)))
        startWorkflow(1);
      else if (checkKeyReleased(1 << 1, PRESSED(S1)))
        resumeWorkflow(1);

      else if (checkKeyPressed(1 << 2, PRESSED(S2)))
        startWorkflow(2);
      else if (checkKeyReleased(1 << 2, PRESSED(S2)))
        resumeWorkflow(2);

      else if (checkKeyPressed(1 << 3, PRESSED(S3)))
        startWorkflow(3);
      else if (checkKeyReleased(1 << 3, PRESSED(S3)))
        resumeWorkflow(3);

      else if (checkKeyPressed(1 << 4, PRESSED(S4)))
        startWorkflow(4);
      else if (checkKeyReleased(1 << 4, PRESSED(S4)))
        resumeWorkflow(4);
    }
  }
}
