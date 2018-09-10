/** @file
  Key translator

Copyright (c) 2018, vit9696. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "AIKTranslate.h"

#include <Library/DebugLib.h>

STATIC
APPLE_MODIFIER_MAP    mModifierRemap[AIK_MODIFIER_MAX];

STATIC
VOID
AIKTranslateModifiers (
  IN  AMI_EFI_KEY_DATA     *KeyData,
  OUT APPLE_MODIFIER_MAP   *Modifiers
  )
{
  UINT32  KeyShiftState;

  KeyShiftState = KeyData->KeyState.KeyShiftState;

  *Modifiers = 0;

  if (KeyShiftState & EFI_SHIFT_STATE_VALID) {
    if (KeyShiftState & EFI_RIGHT_SHIFT_PRESSED) {
      *Modifiers |= mModifierRemap[AIK_RIGHT_SHIFT];
    }
    if (KeyShiftState & EFI_LEFT_SHIFT_PRESSED) {
      *Modifiers |= mModifierRemap[AIK_LEFT_SHIFT];
    }
    if (KeyShiftState & EFI_RIGHT_CONTROL_PRESSED) {
      *Modifiers |= mModifierRemap[AIK_RIGHT_CONTROL];
    }
    if (KeyShiftState & EFI_LEFT_CONTROL_PRESSED) {
      *Modifiers |= mModifierRemap[AIK_LEFT_CONTROL];
    }
    if (KeyShiftState & EFI_RIGHT_ALT_PRESSED) {
      *Modifiers |= mModifierRemap[AIK_RIGHT_ALT];
    }
    if (KeyShiftState & EFI_LEFT_ALT_PRESSED) {
      *Modifiers |= mModifierRemap[AIK_LEFT_ALT];
    }
    if (KeyShiftState & EFI_RIGHT_LOGO_PRESSED) {
      *Modifiers |= mModifierRemap[AIK_RIGHT_GUI];
    }
    if (KeyShiftState & EFI_LEFT_LOGO_PRESSED) {
      *Modifiers |= mModifierRemap[AIK_LEFT_GUI];
    }
  } else {
    //TODO: handle legacy EFI_SIMPLE_TEXT_INPUT_PROTOCOL
  }
}

STATIC
VOID
AIKTranslateNumpad (
  IN OUT UINT8    *UsbKey,
  IN     EFI_KEY  EfiKey
  )
{
  switch (EfiKey) {
    case EfiKeyZero:
      *UsbKey = UsbHidUsageIdKbKpKeyZero;
      break;
    case EfiKeyOne:
      *UsbKey = UsbHidUsageIdKbKpKeyOne;
      break;
    case EfiKeyTwo:
      *UsbKey = UsbHidUsageIdKbKpKeyTwo;
      break;
    case EfiKeyThree:
      *UsbKey = UsbHidUsageIdKbKpKeyThree;
      break;
    case EfiKeyFour:
      *UsbKey = UsbHidUsageIdKbKpKeyFour;
      break;
    case EfiKeyFive:
      *UsbKey = UsbHidUsageIdKbKpKeyFive;
      break;
    case EfiKeySix:
      *UsbKey = UsbHidUsageIdKbKpKeySix;
      break;
    case EfiKeySeven:
      *UsbKey = UsbHidUsageIdKbKpKeySeven;
      break;
    case EfiKeyEight:
      *UsbKey = UsbHidUsageIdKbKpKeyEight;
      break;
    case EfiKeyNine:
      *UsbKey = UsbHidUsageIdKbKpKeyNine;
      break;
    default:
      break;
  }
}

VOID
AIKTranslateConfigure (
  VOID
  )
{
  UINTN                     Index; 
  CONST APPLE_MODIFIER_MAP  DefaultModifierMap[AIK_MODIFIER_MAX] = {
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

  // You can swap Alt with Gui, to get Apple layout on a PC keyboard
  CONST UINTN DefaultModifierConfig[AIK_MODIFIER_MAX/2] = {
    AIK_RIGHT_SHIFT,
    AIK_RIGHT_CONTROL,
    AIK_RIGHT_ALT,
    AIK_RIGHT_GUI
  };

  for (Index = 0; Index < AIK_MODIFIER_MAX/2; Index++) {
    mModifierRemap[Index*2]   = DefaultModifierMap[DefaultModifierConfig[Index]];
    mModifierRemap[Index*2+1] = DefaultModifierMap[DefaultModifierConfig[Index]+1];
  }
}

VOID
AIKTranslate (
  IN  AMI_EFI_KEY_DATA    *KeyData,
  OUT APPLE_MODIFIER_MAP  *Modifiers,
  OUT APPLE_KEY_CODE      *Key
  )
{
  AIK_PS2_TO_USB_MAP  Ps2Key;

  AIKTranslateModifiers (KeyData, Modifiers);

  *Key = UsbHidUndefined;

  //
  // This is APTIO protocol, which reported a PS/2 key to us. Best!
  //
  if (KeyData->PS2ScanCodeIsValid == 1 && KeyData->PS2ScanCode < AIK_MAX_PS2_NUM) {
    Ps2Key = gAikPs2ToUsbMap[KeyData->PS2ScanCode];
    if (Ps2Key.UsbCode != 0) {
      //
      // We need to process numpad keys separately.
      //
      AIKTranslateNumpad (&Ps2Key.UsbCode, KeyData->EfiKey);
      *Key = APPLE_HID_USB_KB_KP_USAGE (Ps2Key.UsbCode);
    }

    DEBUG ((EFI_D_ERROR, "AIKTranslate USB 0x%X PS2 0x%X KeyName %a EfiKey %a Scan 0x%X Uni 0x%X\n",
      Ps2Key.UsbCode, KeyData->PS2ScanCode, Ps2Key.KeyName, 
      KeyData->EfiKey < AIK_MAX_EFIKEY_NUM ? gAikEfiKeyToNameMap[KeyData->EfiKey] : "<err>", 
      KeyData->Key.ScanCode, KeyData->Key.UnicodeChar));
  } else {
    //TODO: Handle KeyData->Key for EFI_SIMPLE_TEXT_INPUT_PROTOCOL.
  }
}
