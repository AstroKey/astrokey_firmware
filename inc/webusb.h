/*
 * webusb.h
 *
 *  Created on: Oct 20, 2017
 *      Author: Aaron
 */

#ifndef INC_WEBUSB_H_
#define INC_WEBUSB_H_

#define USB_BOS_DESCRIPTOR_TYPE 0x0F
#define USB_BOS_DESCRIPTOR_SIZE 5
#define WEBUSB_CAPABILITY_TYPE  0x10
#define WEBUSB_CAPABILITY_SIZE  24
#define WEBUSB_PLATFORM_TYPE    0x05
#define WEBUSB_REQUEST_GET_URL  0x02

typedef struct
{
  uint8_t bLength;         // Size of descriptor (5)
  uint8_t bDescriptorType; // BOS Descriptor type (0x0F)
  uint16_t wTotalLength;   // Length of this descriptor and all of its sub descriptors
  uint8_t bNumDeviceCaps;  // The number of separate device capability descriptors in the BOS
} USB_BOSDescriptor_TypeDef;

typedef struct
{
  uint8_t bLength;                    // Size of descriptor (24)
  uint8_t bDescriptorType;            // Device Capability descriptor type (0x10)
  uint8_t bDevCapabilityType;         // Platform capability type (0x05).
  uint8_t bReserved;                  // This field is reserved and shall be set to zero.
  uint8_t PlatformCapabilityUUID[16]; // Must be set to {3408b638-09a9-47a0-8bfd-a0768815b665}.
  uint16_t bcdVersion;                // Protocol version supported. Must be set to 0x0100.
  uint8_t bVendorCode;                // bRequest value used for issuing WebUSB requests.
  uint8_t iLandingPage;               // URL Descriptor index of the device's landing page.
} WebUSB_DevCapability_TypeDef;

#endif /* INC_WEBUSB_H_ */
