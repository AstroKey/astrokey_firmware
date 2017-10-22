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

// Descriptor requests
#define WEBUSB_REQUEST_GET_URL  0x02

// Descriptor types
#define WEBUSB_URL_DESCRIPTOR 0x03

// String descriptor bScheme
#define WEBUSB_SCHEME_HTTP  0
#define WEBUSB_SCHEME_HTTPS 1
#define WEBUSB_SCHEME_NONE  255

typedef uint8_t USB_URLDescriptor_TypeDef; // URL Descriptor type

#define URL_DESC(__name, __numbytes, __scheme, __val) \
  SI_SEGMENT_VARIABLE(__name,  static const USB_URLDescriptor_TypeDef, SI_SEG_CODE) = \
    { __numbytes + 3, USB_STRING_DESCRIPTOR, __scheme, __val }

typedef struct
{
  uint8_t bLength;         // Size of descriptor (5)
  uint8_t bDescriptorType; // BOS Descriptor type (0x0F)
  uint16_t wTotalLength;   // Length of this descriptor and all of its sub descriptors
  uint8_t bNumDeviceCaps;  // The number of separate device capability descriptors in the BOS
} USB_BOSDescriptor_TypeDef;

#define WEBUSB_DEVCAPABILITY_UUID { \
  0x38, 0xB6, 0x08, 0x34, 0xA9, 0x09, 0xA0, 0x47, \
  0x8B, 0xFD, 0xA0, 0x76, 0x88, 0x15, 0xB6, 0x65  \
}

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

////////////////////////
// Dumb Windows stuff //
////////////////////////

#define MS_OS_20_DEVCAPABILITY_UUID { \
    0xDF, 0x60, 0xDD, 0xD8, 0x89, 0x45, 0xC7, 0x4C, \
    0x9C, 0xD2, 0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F  \
  }

#define MS_OS_20_SET_HEADER_DESCRIPTOR       0x00
#define MS_OS_20_SUBSET_HEADER_CONFIGURATION 0x01
#define MS_OS_20_SUBSET_HEADER_FUNCTION      0x02
#define MS_OS_20_FEATURE_COMPATIBLE_ID       0x03

typedef struct
{
  uint8_t bLength;                        // Size of descriptor (40)
  uint8_t bDescriptorType;                // Device Capability descriptor type (0x10)
  uint8_t bDevCapabilityType;             // Platform capability type (0x05).
  uint8_t bReserved;                      // This field is reserved and shall be set to zero.
  uint8_t PlatformCapabilityUUID[16];     // Must be set to {D8DD60DF-4589-4CC7-9CD2-659D9E648A9F}.
  uint32_t dwWindowsVersion;              // Windows version
  uint16_t wMSOSDescriptorSetTotalLength; // The length, in bytes of the MS OS 2.0 descriptor set.
  uint8_t bMS_VendorCode;                 // Vendor defined code to use to retrieve this version of the MS OS 2.0 descriptor and also to set alternate enumeration behavior on the device.
  uint8_t bAltEnumCode;                   // A non-zero value to send to the device to indicate that the device may return non-default USB descriptors for enumeration.
} MS_OS_20_DevCapability_TypeDef;

typedef struct
{
  uint16_t wLength;          // The length, in bytes, of this header. Shall be set to 10.
  uint16_t wDescriptorType;  // MS_OS_20_SET_HEADER_DESCRIPTOR
  uint32_t dwWindowsVersion; // Windows version.
  uint16_t wTotalLength;     // The size of entire MS OS 2.0 descriptor set. The value shall match the value in the descriptor set information structure.
} MS_OS_20_DescSetHeader_TypeDef;

typedef struct
{
  uint16_t wLength;            // The length, in bytes, of this subset header. Shall be set to 8.
  uint16_t wDescriptorType;    // MS_OS_20_SUBSET_HEADER_CONFIGURATION
  uint8_t bConfigurationValue; // The configuration value for the USB configuration to which this subset applies.
  uint8_t bReserved;           // Shall be set to 0.
  uint16_t wTotalLength;       // The size of entire configuration subset including this header.
} MS_OS_20_ConfSubsetHeader_TypeDef;

typedef struct
{
  uint16_t wLength;         // The length, in bytes, of this subset header. Shall be set to 8.
  uint16_t wDescriptorType; // MS_OS_20_SUBSET_HEADER_FUNCTION
  uint8_t bFirstInterface;  // The interface number for the first interface of the function to which this subset applies.
  uint8_t bReserved;        // Shall be set to 0.
  uint16_t wSubsetLength;   // The size of entire function subset including this header.
} MS_OS_20_FuncSubsetHeader_TypeDef;

typedef struct
{
  uint16_t wLength; // The length, bytes, of the compatible ID descriptor including value descriptors. Shall be set to 20.
  uint16_t wDescriptorType; // MS_OS_20_FEATURE_COMPATIBLE_ID
  uint8_t CompatibleID[8]; // Compatible ID String
  uint8_t SubCompatibleID[8]; // Sub-compatible ID String
} MS_OS_20_CompatibleID_Descriptor_TypeDef;

#endif /* INC_WEBUSB_H_ */
