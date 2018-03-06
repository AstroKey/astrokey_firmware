//-----------------------------------------------------------------------------
// delay.c
//-----------------------------------------------------------------------------
// Copyright 2018 AstroKey
// https://github.com/AstroKey/astrokey_firmware/blob/master/LICENSE
//
// File Description:
//
// Declarations for the millisecond counter.
//

#ifndef INC_DELAY_H_
#define INC_DELAY_H_

#include <stdint.h>

uint32_t getMillis();
void resetMillis();

#endif /* INC_DELAY_H_ */
