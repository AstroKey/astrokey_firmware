/**************************************************************************//**
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#ifndef __SILICON_LABS_DESCRIPTORS_H__
#define __SILICON_LABS_DESCRIPTORS_H__

#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include <webusb.h>
#include "si_toolchain.h"
#include "efm8_usb.h"

#ifdef __cplusplus
extern "C"
{
#endif

// -------------------- USB Identification ------------------------------------
//
// **********
// NOTE: YOU MUST PROVIDE YOUR OWN USB VID/PID (below)
// **********
//
// Following are the definition of the USB VID and PID.  These are example
// values and are assigned to Silicon Labs.  You may not use the Silicon
// Labs VID/PID values in your product.  You must provide your own assigned
// VID and PID values.
///
// $[Vendor ID]
#define USB_VENDOR_ID                      htole16(0x10c4)
// [Vendor ID]$

// $[Product ID]
#define USB_PRODUCT_ID                     htole16(0xfe02)
// [Product ID]$

// Endpoint address of the HID keyboard IN endpoint
#define KEYBOARD_IN_EP_ADDR   EP1IN

// Interface number of the HID keyboard
#define HID_KEYBOARD_IFC                  0

// Keyboard Report
  typedef struct
  {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
  } KeyReport_TypeDef;

extern KeyReport_TypeDef keyReport;
extern volatile bool keyReportSent;

// bRequest number for WebUSB requests
#define WEBUSB_BREQUEST                   1

// bRequest number for MS_OS_20 requests
#define MS_OS_20_BREQEUST                  2

// bRequest number for Astrokey requests
#define ASTROKEY_BREQUEST                 3

// BOS Descriptor + Platform Capability Descriptors
typedef struct
{
  USB_BOSDescriptor_TypeDef bos;
  USB20_ExtCapability_TypeDef ext;
  WebUSB_DevCapability_TypeDef webCap;
  MS_OS_20_DevCapability_TypeDef msCap;
} USB_BOS_TypeDef;

// Dumb Microsoft OS 2.0 Descriptor Set
typedef struct
{
  MS_OS_20_DescSetHeader_TypeDef descSet;
  MS_OS_20_ConfSubsetHeader_TypeDef confSubset;
  MS_OS_20_FuncSubsetHeader_TypeDef funcSubset;
  MS_OS_20_CompatibleID_Descriptor_TypeDef compatibleID;
  MS_OS_20_DeviceIntGUID_TypeDef devIntGUID;
} MS_OS_20_DescriptorSet_TypeDef;

// Sizes of MS OS 2.0 Descriptor Subsets
#define MS_DSH_S htole16(sizeof(MS_OS_20_DescSetHeader_TypeDef))
#define MS_CSH_S htole16(sizeof(MS_OS_20_ConfSubsetHeader_TypeDef))
#define MS_FSH_S htole16(sizeof(MS_OS_20_FuncSubsetHeader_TypeDef))
#define MS_CID_S htole16(sizeof(MS_OS_20_CompatibleID_Descriptor_TypeDef))
#define MS_DIG_S htole16(sizeof(MS_OS_20_DeviceIntGUID_TypeDef))

// Size of entire MS OS 2.0 Descriptor
#define MS_DS_S htole16(sizeof(MS_OS_20_DescriptorSet_TypeDef))

  extern SI_SEGMENT_VARIABLE(ReportDescriptor0[69], const uint8_t, SI_SEG_CODE);
  extern SI_SEGMENT_VARIABLE(deviceDesc[], const USB_DeviceDescriptor_TypeDef, SI_SEG_CODE);
  extern SI_SEGMENT_VARIABLE(configDesc[], const uint8_t, SI_SEG_CODE);
  extern SI_SEGMENT_VARIABLE(initstruct, const USBD_Init_TypeDef, SI_SEG_CODE);

  SI_SEGMENT_VARIABLE(wcidDesc[], static const USB_StringDescriptor_TypeDef, SI_SEG_CODE) =
  {
    0x12,                   // Length (18 Bytes)
    0x03,// Descriptor Type (3 = String)
    0x4D, 0x00, 0x53, 0x00,// MSFT100, Unicode LE
    0x46, 0x00, 0x54, 0x00,//
    0x31, 0x00, 0x30, 0x00,//
    0x30, 0x00,//
    0x20,// Vendor Code
    0x00// Padding
  };

  extern SI_SEGMENT_VARIABLE_SEGMENT_POINTER(myURLs[], const USB_URLDescriptor_TypeDef, SI_SEG_GENERIC, const SI_SEG_CODE);
  extern uint16_t numUrls;

extern SI_SEGMENT_VARIABLE(bosDesc, const USB_BOS_TypeDef, SI_SEG_CODE);
extern SI_SEGMENT_VARIABLE(msDesc, const MS_OS_20_DescriptorSet_TypeDef, SI_SEG_CODE);

#ifdef __cplusplus
}
#endif

#endif  // #define __SILICON_LABS_DESCRIPTORS_H__
// $[HID Report Descriptors]
extern SI_SEGMENT_VARIABLE(ReportDescriptor0[69], const uint8_t, SI_SEG_CODE);
// [HID Report Descriptors]$

