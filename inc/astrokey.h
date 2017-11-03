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

// Switch configuration
#define NUM_SWTICHES 5
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

#define MACRO_MAX_SIZE 64
#define MACRO_MAX_KEYS 6

#endif /* INC_ASTROKEY_H_ */
