/**************************************************************************//**
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#include "SI_EFM8UB1_Register_Enums.h"
#include "efm8_usb.h"
#include "descriptors.h"
#include "idle.h"
#include "webusb.h"
#include "astrokey.h"
#include "delay.h"

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Variables
// ----------------------------------------------------------------------------
uint8_t tmpBuffer;
volatile int8_t workflowTransfer = -1;

uint32_t tmp32;

// ----------------------------------------------------------------------------
// Functions
// ----------------------------------------------------------------------------
#if SLAB_USB_HANDLER_CB
void USBD_EnterHandler(void)
{

}

void USBD_ExitHandler(void)
{

}
#endif

#if SLAB_USB_RESET_CB
void USBD_ResetCb(void)
{

}
#endif // SLAB_USB_RESET_CB

#if SLAB_USB_SOF_CB
void USBD_SofCb(uint16_t sofNr)
{
  int8_t status;
  UNREFERENCED_ARGUMENT(sofNr);

  idleTimerTick();

  // Check if the device should send a report
  // if (isIdleTimerExpired() == true || !keyReportSent)
  if (!keyReportSent)
  {
    status = USBD_Write(KEYBOARD_IN_EP_ADDR,
                        (SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_GENERIC))&keyReport,
                        sizeof(KeyReport_TypeDef),
                        false);
    if (status == USB_STATUS_OK)
      keyReportSent = true;
  }



}
#endif // SLAB_USB_SOF_CB

#if SLAB_USB_STATE_CHANGE_CB
void USBD_DeviceStateChangeCb(USBD_State_TypeDef oldState,
                              USBD_State_TypeDef newState)
{
  // If not configured or in suspend, disable the LED
  if (newState < USBD_STATE_SUSPENDED)
  {
    // Disable the LED
  }
  // Entering suspend mode, power internal and external blocks down
  else if (newState == USBD_STATE_SUSPENDED)
  {
    // Disable the LED's

    // Abort any pending transfer
    USBD_AbortTransfer(KEYBOARD_IN_EP_ADDR);
  }
  else if (newState == USBD_STATE_CONFIGURED)
  {
    idleSetDuration(POLL_RATE_MS);
  }

  // Exiting suspend mode, power internal and external blocks up
  if (oldState == USBD_STATE_SUSPENDED)
  {
    // Restore the LED's to their previous values
  }
}
#endif // SLAB_USB_STATE_CHANGE_CB

#if SLAB_USB_IS_SELF_POWERED_CB
bool USBD_IsSelfPoweredCb(void)
{
  return false;
}
#endif // SLAB_USB_IS_SELF_POWERED_CB

#if SLAB_USB_SETUP_CMD_CB
USB_Status_TypeDef USBD_SetupCmdCb(SI_VARIABLE_SEGMENT_POINTER(
                                     setup,
                                     USB_Setup_TypeDef,
                                     MEM_MODEL_SEG))
{
  USB_Status_TypeDef retVal = USB_STATUS_REQ_UNHANDLED;

  // Setup Command: Standard request to device in direction IN
  if ((setup->bmRequestType.Type == USB_SETUP_TYPE_STANDARD)
      && (setup->bmRequestType.Direction == USB_SETUP_DIR_IN)
      && (setup->bmRequestType.Recipient == USB_SETUP_RECIPIENT_DEVICE))
  {
    uint8_t index;
    switch (setup->bRequest)
    {
      case GET_DESCRIPTOR: // Get Device Descriptor
	index = setup->wValue & 0xFF;
        // Binary object store descriptor
        // This must include the capability descriptors as sub-descriptors, which
        // cannot be directly accessed with a GetDescriptor command.
        if ((setup->wValue >> 8) == USB_BOS_DESCRIPTOR_TYPE)
        {
          switch (setup->wIndex)
          {
            case 0: // Language 0
              USBD_Write(EP0,
			 (SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_GENERIC))&bosDesc,
			 EFM8_MIN(sizeof(bosDesc), setup->wLength),
			 false);
              retVal = USB_STATUS_OK;
              break;

            default: // Unhandled Language
              break;
          }
        }
        break;
      default:
        break;
    }
  }
  // Setup Command: Vendor request to device in direction IN
  else if ((setup->bmRequestType.Type == USB_SETUP_TYPE_VENDOR)
      && (setup->bmRequestType.Direction == USB_SETUP_DIR_IN)
      && (setup->bmRequestType.Recipient == USB_SETUP_RECIPIENT_DEVICE))
  {
    // Check request platform
    switch (setup->bRequest)
    {

      // WebUSB platform request
      case WEBUSB_BREQUEST:
        // Check request type
        switch (setup->wIndex)
        {
          // Get URL request
          case WEBUSB_REQUEST_GET_URL:
            // Check to make sure the URL index requested is valid
            if (setup->wValue <= numUrls && setup->wValue != 0)
            {
              USBD_Write(EP0,
                         (SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_GENERIC))(myURLs[setup->wValue - 1]),
                         EFM8_MIN(myURLs[setup->wValue - 1][0], setup->wLength),
                         false);
              retVal = USB_STATUS_OK;
            }
            break;
        }
        break;
      // MS OS 2.0 platform request
      case MS_OS_20_BREQEUST:
        // Check request type
        switch (setup->wIndex)
        {
          // Get descriptor type
          case MS_OS_20_REQUEST_DESCRIPTOR:
            USBD_Write(EP0,
        	       (SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_GENERIC))&msDesc,
        	       EFM8_MIN(MS_DS_S, setup->wLength),
        	       false);
            retVal = USB_STATUS_OK;
            break;
        }
        break;
      case ASTROKEY_BREQUEST:
        switch (setup->wIndex)
        {
          // Read workflow off device
          case ASTROKEY_GET_WORKFLOW:
            loadWorkflow(tmpWorkflow, setup->wValue);

            USBD_Write(EP0,
                       (SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_GENERIC))tmpWorkflow,
                       EFM8_MIN(WORKFLOW_BYTES, setup->wLength),
                       false);

            retVal = USB_STATUS_OK;
            break;
          case 0xF0:

            tmp32 = getMillis();

            USBD_Write(EP0,
                       (SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_GENERIC))&tmp32,
                       EFM8_MIN(sizeof(tmp32), setup->wLength),
                       false);

            retVal = USB_STATUS_OK;

            break;
        }
        break;
      default:
	break;
    }
  }
  // Setup Command: Vendor request to device in direction OUT
  else if ((setup->bmRequestType.Type == USB_SETUP_TYPE_VENDOR)
      && (setup->bmRequestType.Direction == USB_SETUP_DIR_OUT)
      && (setup->bmRequestType.Recipient == USB_SETUP_RECIPIENT_DEVICE))
  {
    if (setup->bRequest == ASTROKEY_BREQUEST)
    {
      switch (setup->wIndex) // Request type
      {
        case ASTROKEY_SET_WORKFLOW:
          memset((void*) tmpWorkflow, 0, WORKFLOW_BYTES);
          USBD_Read(EP0,
                    (SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_GENERIC))tmpWorkflow,
                    EFM8_MIN(WORKFLOW_BYTES, setup->wLength),
                    true);

          workflowTransfer = setup->wValue;

          retVal = USB_STATUS_OK;
          break;
      }
    }
  }
  // Setup command: Standard request to interface in direction IN
  else if ((setup->bmRequestType.Type == USB_SETUP_TYPE_STANDARD)
      && (setup->bmRequestType.Direction == USB_SETUP_DIR_IN)
      && (setup->bmRequestType.Recipient == USB_SETUP_RECIPIENT_INTERFACE))
  {
    // A HID device must extend the standard GET_DESCRIPTOR command
    // with support for HID descriptors.
    switch (setup->bRequest)
    {
      case GET_DESCRIPTOR:
        if ((setup->wValue >> 8) == USB_HID_REPORT_DESCRIPTOR)
        {
          switch (setup->wIndex)
          {
            case HID_KEYBOARD_IFC: // HID Interface

              USBD_Write(EP0,
                         (SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_GENERIC))ReportDescriptor0,
                         EFM8_MIN(sizeof(ReportDescriptor0), setup->wLength),
                         false);
              retVal = USB_STATUS_OK;
              break;

            default: // Unhandled Interface
              break;
          }
        }
        else if ((setup->wValue >> 8) == USB_HID_DESCRIPTOR)
        {
          switch (setup->wIndex)
          {
            case HID_KEYBOARD_IFC: // HID Interface
              USBD_Write(EP0,
                         (SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_GENERIC))(&configDesc[18]),
                         EFM8_MIN(USB_HID_DESCSIZE, setup->wLength),
                         false);
              retVal = USB_STATUS_OK;
              break;

            default: // Unhandled Interface
              break;
          }
        }
        break;
    }
  }
  // Setup command: HID Class request to interface, wIndex of keyboard
  else if ((setup->bmRequestType.Type == USB_SETUP_TYPE_CLASS)
           && (setup->bmRequestType.Recipient == USB_SETUP_RECIPIENT_INTERFACE)
           && (setup->wIndex == HID_KEYBOARD_IFC))
  {
    // Implement the necessary HID class specific commands.
    switch (setup->bRequest)
    {
      /*case USB_HID_SET_REPORT:
        if (((setup->wValue >> 8) == 2)               // Output report
            && ((setup->wValue & 0xFF) == 0)          // Report ID
            && (setup->wLength == 1)                  // Report length
            && (setup->bmRequestType.Direction != USB_SETUP_DIR_IN))
        {
          USBD_Read(EP0, &tmpBuffer, 1, true);
          // LEDs
          retVal = USB_STATUS_OK;
        }
        break;*/

      case USB_HID_GET_REPORT:
        if (((setup->wValue >> 8) == 1)               // Input report
            && ((setup->wValue & 0xFF) == 0)          // Report ID
            && (setup->wLength == 8)                  // Report length
            && (setup->bmRequestType.Direction == USB_SETUP_DIR_IN))
        {
          USBD_Write(KEYBOARD_IN_EP_ADDR,
                     (SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_GENERIC))&keyReport,
                     sizeof(KeyReport_TypeDef),
                     false);
          keyReportSent = true;

          retVal = USB_STATUS_OK;
        }
        break;

      case USB_HID_SET_IDLE:
        if (((setup->wValue & 0xFF) == 0)             // Report ID
            && (setup->wLength == 0)
            && (setup->bmRequestType.Direction != USB_SETUP_DIR_IN))
        {
          idleSetDuration(setup->wValue >> 8);
          retVal = USB_STATUS_OK;
        }
        break;

      case USB_HID_GET_IDLE:
        if ((setup->wValue == 0)                      // Report ID
            && (setup->wLength == 1)
            && (setup->bmRequestType.Direction == USB_SETUP_DIR_IN))
        {
          tmpBuffer = idleGetDuration();
          USBD_Write(EP0, &tmpBuffer, 1, false);
          retVal = USB_STATUS_OK;
        }
        break;
    }
  }

  return retVal;
}
#endif // SLAB_USB_SETUP_CMD_CB

#if SLAB_USB_SUPPORT_ALT_INTERFACES
USB_Status_TypeDef USBD_SetInterfaceCb(uint8_t interface, uint8_t altSetting)
{
  USB_Status_TypeDef retVal = USB_STATUS_REQ_ERR;

  UNREFERENCED_ARGUMENT(interface);
  UNREFERENCED_ARGUMENT(altSetting);

  return retVal;
}
#endif // SLAB_USB_SUPPORT_ALT_INTERFACES

#if SLAB_USB_REMOTE_WAKEUP_ENABLED
bool USBD_RemoteWakeupCb(void)
{
  // Return true if a remote wakeup event was the cause of the device
  // exiting suspend mode.
  // Otherwise return false
  return false;
}

void USBD_RemoteWakeupDelay(void)
{
  // Delay 10 - 15 ms here

}
#endif

uint16_t USBD_XferCompleteCb(uint8_t epAddr,
                             USB_Status_TypeDef status,
                             uint16_t xferred,
                             uint16_t remaining)
{
  UNREFERENCED_ARGUMENT(epAddr);
  UNREFERENCED_ARGUMENT(xferred);
  UNREFERENCED_ARGUMENT(remaining);

  if (status == USB_STATUS_OK)
  {
    if (workflowTransfer != -1)
    {
      workflowUpdated = workflowTransfer;
      workflowTransfer = -1;
    }
  }

  return 0;
}
