//-----------------------------------------------------------------------------
// astrokey.c
//-----------------------------------------------------------------------------
// Copyright 2018 AstroKey
// https://github.com/AstroKey/astrokey_firmware/blob/master/LICENSE
//
// File Description:
//
// Implementation of AstroKey input polling and other device-specific functionality.
//

#include "astrokey.h"
#include "InitDevice.h"
#include "efm8_usb.h"
#include "descriptors.h"
#include "idle.h"
#include "delay.h"
#include "EFM8UB1_FlashPrimitives.h"
#include "EFM8UB1_FlashUtils.h"

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

// The UUID String descriptor
UTF16LE_PACKED_STRING_DESC(serDesc[SER_STR_LEN + USB_STRING_DESCRIPTOR_NAME], SER_STR_LEN);

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
      else if ((getMillis() - delayStartTime) > ((uint32_t)value * 10))
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

  if (actionType == 0x00 || actionType == WORKFLOW_ACTION_UNPROGRAMMED ||
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

void astrokeyInit()
{
  uint8_t i;
  // Read chip UUID and write hex string to serial string descriptor
  for (i = 0; i < UUID_LEN; i++)
  {
    serDesc[USB_STRING_DESCRIPTOR_NAME + 2 * i + 0] =
      NIBBLE_TO_ASCII((UUID[i] >> 8) & 0x0F);
    serDesc[USB_STRING_DESCRIPTOR_NAME + 2 * i + 1] =
      NIBBLE_TO_ASCII((UUID[i] >> 0) & 0x0F);
  }
  // Enter default device configuration
  enter_DefaultMode_from_RESET();
}

void astrokeyPoll()
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
