//-----------------------------------------------------------------------------
// main.c
//-----------------------------------------------------------------------------
// Copyright 2018 AstroKey
// https://github.com/AstroKey/astrokey_firmware/blob/master/LICENSE
//
// Program Description:
//
// AstroKey is an open-source platform for automating keyboard workflows.
// This program is the firmware running on the microcontroller found in AstroKey.
// It enumerates as a USB device with 2 interfaces, 1 to implement WebUSB and the other a HID.
// The WebUSB Interface allows the AstroKey website to save workflows created by the user to the device.
// The HID Interface plays back macros.
//
#include "SI_EFM8UB1_Register_Enums.h"
#include "astrokey.h"

#include <stdint.h>

// ----------------------------------------------------------------------------
// main() Routine
// ---------------------------------------------------------------------------
//
// Initializes AstroKey functionality and polls the device inputs.
//
// ----------------------------------------------------------------------------
int16_t main(void)
{
  astrokeyInit();

  while (1)
  {
    astrokeyPoll();
  }
}
