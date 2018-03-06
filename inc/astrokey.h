//-----------------------------------------------------------------------------
// astrokey.h
//-----------------------------------------------------------------------------
// Copyright 2018 AstroKey
// https://github.com/AstroKey/astrokey_firmware/blob/master/LICENSE
//
// File Description:
//
// AstroKey-specific definitions and function declarations.
//

#ifndef INC_ASTROKEY_H_
#define INC_ASTROKEY_H_

#include <SI_EFM8UB1_Defs.h>
#include <stdint.h>
#include <efm8_usb.h>

//////////////////////////
// Device Serial Number //
//////////////////////////

// Converts a nibble to a hex character
#define NIBBLE_TO_ASCII(x) ((x) >= 10? (x) - 10 + 'A' : (x) + '0')

// Struct for non-const string descriptors
#define UTF16LE_PACKED_STRING_DESC(__name, __size) \
  SI_SEGMENT_VARIABLE(__name,  USB_StringDescriptor_TypeDef, SI_SEG_XDATA) = \
    { USB_STRING_DESCRIPTOR_UTF16LE_PACKED, __size * 2, USB_STRING_DESCRIPTOR }

// Serial number string descriptor
extern SI_SEGMENT_VARIABLE(serDesc[], USB_StringDescriptor_TypeDef, SI_SEG_XDATA);

// MCU UUID Flash Address
#define UUID_ADDR 0xFFC0

// MCU UUID Length in Bytes
#define UUID_LEN  16

// Length of serial number string (2 characters per byte)
#define SER_STR_LEN (UUID_LEN * 2)

// MCU UUID
SI_VARIABLE_SEGMENT_POINTER(UUID, static const uint8_t, SI_SEG_CODE) = UUID_ADDR;

///////////////////////////
// Astrokey USB protocol //
///////////////////////////

// wIndex values
#define ASTROKEY_SET_WORKFLOW 0x01
#define ASTROKEY_GET_WORKFLOW 0x02

///////////////////////
// Device Parameters //
///////////////////////

// Switch configuration
#define NUM_SWITCHES 5
#define S0 P0_B0
#define S1 P0_B1
#define S2 P0_B2
#define S3 P0_B3
#define S4 P0_B4

// Switch pressed
#define PRESSED(x) (!x)

////////////////////////
// Workflow Constants //
////////////////////////

// No workflow running
#define NO_WORKFLOW 0xFF

// Workflow action types
#define WORKFLOW_ACTION_DOWN  1
#define WORKFLOW_ACTION_UP    2
#define WORKFLOW_ACTION_PRESS 3
#define WORKFLOW_ACTION_DELAY 16
#define WORKFLOW_ACTION_PAUSE 128 // Pauses a macro until key release
#define WORKFLOW_ACTION_UNPROGRAMMED 255 // Unprogrammed flash memory

#define USAGE_LEFTCTRL  224
#define USAGE_LEFTSHIFT 225
#define USAGE_LEFTALT   226
#define USAGE_LEFTGUI   227

#define MODIFIER_LEFTCTRL  0x01
#define MODIFIER_LEFTSHIFT 0x02
#define MODIFIER_LEFTALT   0x04
#define MODIFIER_LEFTGUI   0x08

// Workflow action struct
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

////////////////////////
// Workflow Functions //
////////////////////////

void saveWorkflow(Action_TypeDef* workflowData, uint8_t saveIndex);
void loadWorkflow(Action_TypeDef* workflowData, uint8_t loadIndex);

////////////////////////
// Workflow Variables //
////////////////////////

extern Action_TypeDef SI_SEG_XDATA workflow[WORKFLOW_MAX_SIZE];
extern uint8_t workflowNumActions;

extern Action_TypeDef SI_SEG_XDATA tmpWorkflow[WORKFLOW_MAX_SIZE];
extern volatile int8_t workflowUpdated;

////////////////////////
// Astrokey Functions //
////////////////////////

void astrokeyInit();
void astrokeyPoll();

#endif /* INC_ASTROKEY_H_ */
