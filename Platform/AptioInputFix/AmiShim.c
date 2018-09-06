/** @file
  Ami input translators.

Copyright (c) 2016, CupertinoNet. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "AmiShim.h"

//#include <MiscBase.h>

APPLE_MODIFIER_MAP         gModifierRemap[AmiShimModifierMax];
STATIC EFI_EVENT           mAmiShimTranslatorExitBootServicesEvent;
STATIC BOOLEAN             mPerformedExit;


VOID
AmiShimConfigureModifierMap (
  VOID
  )
{
  UINTN                     Index; 
  CONST APPLE_MODIFIER_MAP  DefaultModifierMap[AmiShimModifierMax] = {
    USB_HID_KB_KP_MODIFIER_RIGHT_SHIFT,
    USB_HID_KB_KP_MODIFIER_LEFT_SHIFT,
    USB_HID_KB_KP_MODIFIER_RIGHT_CONTROL,
    USB_HID_KB_KP_MODIFIER_LEFT_CONTROL,
    USB_HID_KB_KP_MODIFIER_RIGHT_ALT,
    USB_HID_KB_KP_MODIFIER_LEFT_ALT,
    USB_HID_KB_KP_MODIFIER_RIGHT_GUI,
    USB_HID_KB_KP_MODIFIER_LEFT_GUI
  };

  //TODO: This should be user-configurable, perhaps via a nvram variable.

  // By default swap Alt with Gui, so that a PC keyboard gets an Apple layout
  CONST UINTN DefaultModifierConfig[AmiShimModifierMax/2] = {
    AmiShimRightShift,
    AmiShimRightControl,
    AmiShimRightGui,
    AmiShimRightAlt
  };

  for (Index = 0; Index < AmiShimModifierMax/2; Index++) {
    gModifierRemap[Index*2] = DefaultModifierMap[DefaultModifierConfig[Index]];
    gModifierRemap[Index*2+1] = DefaultModifierMap[DefaultModifierConfig[Index]+1];
  }
}

VOID
EFIAPI
AmiShimTranslatorExitBootServicesHandler (
  IN  EFI_EVENT                 Event,
  IN  VOID                      *Context
  )
{
  if (mPerformedExit) {
    return;
  }

  mPerformedExit = TRUE;
  gBS->CloseEvent(mAmiShimTranslatorExitBootServicesEvent);
  AmiShimPointerExit();
  AmiShimKeycodeExit();
}

EFI_STATUS
EFIAPI
AmiShimTranslatorEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS    Status;

  AmiShimConfigureModifierMap();
  AmiShimPointerInit();
  AmiShimKeycodeInit();

  Status = gBS->CreateEvent (EVT_SIGNAL_EXIT_BOOT_SERVICES, TPL_NOTIFY, AmiShimTranslatorExitBootServicesHandler, NULL, &mAmiShimTranslatorExitBootServicesEvent);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to create exit bs event %d", Status));
  }

  return EFI_SUCCESS;
}
