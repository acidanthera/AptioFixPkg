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

#ifndef AIK_TRANSLATE_H
#define AIK_TRANSLATE_H

#include <IndustryStandard/AppleHid.h>
#include <Protocol/AmiKeycode.h>

#define AIK_MAX_PS2_NUM    128
#define AIK_MAX_EFIKEY_NUM 128

typedef struct {
  UINT8         UsbCode;
  CONST CHAR8   *KeyName;
  CONST CHAR8   *ShiftKeyName;
} AIK_PS2_TO_USB_MAP;

extern AIK_PS2_TO_USB_MAP    gAikPs2ToUsbMap[AIK_MAX_PS2_NUM];
extern CONST CHAR8 *         gAikEfiKeyToNameMap[AIK_MAX_EFIKEY_NUM];

enum {
  AIK_RIGHT_SHIFT,
  AIK_LEFT_SHIFT,
  AIK_RIGHT_CONTROL,
  AIK_LEFT_CONTROL,
  AIK_RIGHT_ALT,
  AIK_LEFT_ALT,
  AIK_RIGHT_GUI,
  AIK_LEFT_GUI,
  AIK_MODIFIER_MAX
};

VOID
AIKTranslateConfigure (
  VOID
  );

VOID
AIKTranslate (
  IN  AMI_EFI_KEY_DATA    *KeyData,
  OUT APPLE_MODIFIER_MAP  *Modifiers,
  OUT APPLE_KEY_CODE      *Key
  );

#endif
