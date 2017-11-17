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
#define NUM_SWITCHES 2
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

// Macro action struct
typedef struct {
  uint8_t actionType;
  uint8_t value;
} Macro_TypeDef;

#define MACRO_MAX_SIZE 10
#define MACRO_MAX_KEYS 6
#define MACRO_BYTES (MACRO_MAX_SIZE * sizeof(Macro_TypeDef))

#define BOOTLOADER_START_ADDR 0x1A00
#define MACRO_FLASH_ADDR (BOOTLOADER_START_ADDR - (NUM_SWITCHES * MACRO_BYTES))

void saveMacro(Macro_TypeDef* macroData, uint8_t saveIndex);
void loadMacro(Macro_TypeDef* macroData, uint8_t loadIndex);

extern Macro_TypeDef SI_SEG_XDATA macro[MACRO_MAX_SIZE];
extern uint8_t macroNumActions;

extern Macro_TypeDef SI_SEG_XDATA tmpMacro[MACRO_MAX_SIZE];
extern volatile int8_t macroUpdated;

#endif /* INC_ASTROKEY_H_ */
