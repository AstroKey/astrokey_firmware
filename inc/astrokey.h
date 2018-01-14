/*
 * astrokey.h
 *
 *  Created on: Nov 3, 2017
 *      Author: Aaron
 */

#ifndef INC_ASTROKEY_H_
#define INC_ASTROKEY_H_

#include <SI_EFM8UB1_Defs.h>
#include <stdint.h>

// Astrokey USB protocol

// wIndex values
#define ASTROKEY_SET_WORKFLOW 0x01
#define ASTROKEY_GET_WORKFLOW 0x02

// Switch configuration
#define NUM_SWITCHES 5
#define S0 P0_B0
#define S1 P0_B1
#define S2 P0_B2
#define S3 P0_B3
#define S4 P0_B4

// Switch pressed
#define PRESSED(x) (!x)

// No macro running
#define NO_WORKFLOW 0xFF

// Macro action types
#define WORKFLOW_ACTION_DOWN  1
#define WORKFLOW_ACTION_UP    2
#define WORKFLOW_ACTION_PRESS 3
#define WORKFLOW_ACTION_DELAY 16
#define WORKFLOW_ACTION_PAUSE 128 // Pauses a macro until key release

#define USAGE_LEFTCTRL  224
#define USAGE_LEFTSHIFT 225
#define USAGE_LEFTALT   226
#define USAGE_LEFTGUI   227

#define MODIFIER_LEFTCTRL  0x01
#define MODIFIER_LEFTSHIFT 0x02
#define MODIFIER_LEFTALT   0x04
#define MODIFIER_LEFTGUI   0x08

// Macro action struct
typedef struct {
  uint8_t actionType;
  uint8_t value;
} Action_TypeDef;

// User data flash
#define USER_PAGE_SIZE  64
#define USER_START_ADDR 0xF800

// Number of pages per macro
#define WORKFLOW_PAGES 2
// Number of bytes per macro
#define WORKFLOW_BYTES (WORKFLOW_PAGES * USER_PAGE_SIZE)
// Maximum number of actions in a macro
#define WORKFLOW_MAX_SIZE ((WORKFLOW_BYTES) / sizeof(Action_TypeDef))
// Max number of keys simultaenously held by macro
#define WORKFLOW_MAX_KEYS 6

#define WORKFLOW_FLASH_ADDR USER_START_ADDR

void saveWorkflow(Action_TypeDef* workflowData, uint8_t saveIndex);
void loadWorkflow(Action_TypeDef* workflowData, uint8_t loadIndex);

extern Action_TypeDef SI_SEG_XDATA workflow[WORKFLOW_MAX_SIZE];
extern uint8_t workflowNumActions;

extern Action_TypeDef SI_SEG_XDATA tmpWorkflow[WORKFLOW_MAX_SIZE];
extern volatile int8_t workflowUpdated;

#endif /* INC_ASTROKEY_H_ */
