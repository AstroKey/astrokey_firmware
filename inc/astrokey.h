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
#define ASTROKEY_SET_MACRO 0x01
#define ASTROKEY_GET_MACRO 0x02

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
#define NO_MACRO 0xFF

// Macro action types
#define MACRO_ACTION_DOWN  1
#define MACRO_ACTION_UP    2
#define MACRO_ACTION_PRESS 3
#define MACRO_DELAY        16

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
} Macro_TypeDef;

// User data flash
#define USER_PAGE_SIZE  64
#define USER_START_ADDR 0xF800

// Number of pages per macro
#define MACRO_PAGES 2
// Number of bytes per macro
#define MACRO_BYTES (MACRO_PAGES * USER_PAGE_SIZE)
// Maximum number of actions in a macro
#define MACRO_MAX_SIZE ((MACRO_BYTES) / sizeof(Macro_TypeDef))
// Max number of keys simultaenously held by macro
#define MACRO_MAX_KEYS 6

#define MACRO_FLASH_ADDR USER_START_ADDR

void saveMacro(Macro_TypeDef* macroData, uint8_t saveIndex);
void loadMacro(Macro_TypeDef* macroData, uint8_t loadIndex);

extern Macro_TypeDef SI_SEG_XDATA macro[MACRO_MAX_SIZE];
extern uint8_t macroNumActions;

extern Macro_TypeDef SI_SEG_XDATA tmpMacro[MACRO_MAX_SIZE];
extern volatile int8_t macroUpdated;

#endif /* INC_ASTROKEY_H_ */
