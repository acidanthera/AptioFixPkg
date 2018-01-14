/** @file
  AmiEfiKeycode to KeyMapDb translator.

Copyright (c) 2016, CupertinoNet. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "AmiShim.h"
#include "AmiKeycode.h"
#include "AmiShimPs2Map.h"
#include "AmiShimEfiMap.h"
#include "AmiShimKeycode.h"

//#include <MiscBase.h>

STATIC AMI_SHIM_KEYCODE mAmiShimKeycode;

VOID
AmiShimKeycodeInitKeysBuffer (
  IN OUT AMI_SHIM_KEYCODE_INSTANCE    *Keycode
  )
{
  Keycode->PendingKeysBufferHead = Keycode->PendingKeysBufferTail = Keycode->PendingKeysBuffer;
  Keycode->PendingKeysBufferSize = 0;
}

BOOLEAN
AmiShimKeycodeReadKeysBuffer (
  IN OUT AMI_SHIM_KEYCODE_INSTANCE    *Keycode,
  OUT AMI_EFI_KEY_DATA                *KeyData
  )
{
  if (Keycode->PendingKeysBufferSize == 0) {
    return FALSE;
  }

  *KeyData = *Keycode->PendingKeysBufferTail;

  Keycode->PendingKeysBufferSize--;
  Keycode->PendingKeysBufferTail++;
  if (Keycode->PendingKeysBufferTail == &Keycode->PendingKeysBuffer[MAX_KEY_BUFFER_SIZE]) {
    Keycode->PendingKeysBufferTail = Keycode->PendingKeysBuffer;
  }

  return TRUE;
}

VOID
AmiShimKeycodeWriteKeysBuffer (
  IN OUT AMI_SHIM_KEYCODE_INSTANCE    *Keycode,
  IN AMI_EFI_KEY_DATA                 *KeyData
  )
{
  // Eat the first key
  if (Keycode->PendingKeysBufferSize == MAX_KEY_BUFFER_SIZE) {
    Keycode->PendingKeysBufferSize--;
    Keycode->PendingKeysBufferTail++;
    if (Keycode->PendingKeysBufferTail == &Keycode->PendingKeysBuffer[MAX_KEY_BUFFER_SIZE]) {
      Keycode->PendingKeysBufferTail = Keycode->PendingKeysBuffer;
    }
  }
  
  Keycode->PendingKeysBufferSize++;
  *Keycode->PendingKeysBufferHead = *KeyData;
  Keycode->PendingKeysBufferHead++;
  if (Keycode->PendingKeysBufferHead == &Keycode->PendingKeysBuffer[MAX_KEY_BUFFER_SIZE]) {
    Keycode->PendingKeysBufferHead = Keycode->PendingKeysBuffer;
  }
}

EFI_STATUS
AmiShimKeycodeSendData (
  IN AMI_SHIM_KEYCODE_INSTANCE         *Keycode,
  IN BOOLEAN                           ResetModifiers,
  IN BOOLEAN                           PassModifiers
  )
{
  EFI_STATUS  Status;
  UINTN       Index;

  if (Keycode->CurrentNumberOfKeys > 0 || Keycode->CurrentDbHasData || PassModifiers) {
    //DEBUG ((EFI_D_ERROR, "AmiShim sent %d keys: [%X] %X %X %X %X %X %X\n", mCurrentNumberOfKeys, mCurrentModifiers,
    //                     mCurrentKeys[0], mCurrentKeys[1], mCurrentKeys[2],
    //                     mCurrentKeys[3], mCurrentKeys[4], mCurrentKeys[5]));
    Status = mAmiShimKeycode.KeyMapDb->SetKeyStrokeBufferKeys (
                              mAmiShimKeycode.KeyMapDb,
                              Keycode->KeyMapDbIndex,
                              Keycode->CurrentModifiers,
                              Keycode->CurrentNumberOfKeys,
                              Keycode->CurrentKeys
                              );

    // Make sure it is called soon to reset the contents
    Keycode->CurrentDbHasData = Keycode->CurrentNumberOfKeys > 0 || PassModifiers;

    for (Index = 0; Index < MAX_KEY_NUM; Index++) {
      Keycode->CurrentKeys[Index] = 0;
    }

    Keycode->CurrentNumberOfKeys = 0;
    if (ResetModifiers) {
      Keycode->CurrentModifiers = 0;
    }

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "SetKeyStrokeBufferKeys failed %d\n", Status));
      return Status;
    }
  }

  return EFI_SUCCESS;
}

VOID
AmiShimKeycodeInsertKey (
  IN AMI_SHIM_KEYCODE_INSTANCE *Keycode,
  IN UINT8                     UsbKey,
  IN CONST CHAR8               *Name
  )
{
  UINTN  Index;
  APPLE_KEY_CODE Key;

  // Flush the keys if we have no room for more
  // Unfortunately passing the real buffer size here means broken password input on quick input.
  // Furthermore multiple keys are not reported together, so this is meaningless to support anyway.
  if (Keycode->CurrentNumberOfKeys == 1 /* MAX_KEY_NUM */) {
    AmiShimKeycodeSendData(Keycode, FALSE, FALSE);
  }

  Key = APPLE_HID_USB_KB_KP_USAGE (UsbKey);

  // Flush the keys if we have the same key repeated
  for (Index = 0; Index < Keycode->CurrentNumberOfKeys; Index++) {
    if (Keycode->CurrentKeys[Index] == Key) {
      AmiShimKeycodeSendData (Keycode, FALSE, FALSE);
      break;
    }
  }
  
  //DEBUG ((EFI_D_ERROR, "AmiShim received key [%d] [%X] %a\n", Keycode->PerformingManualPoll, Keycode->CurrentModifiers, Name));
  
  Keycode->CurrentKeys[Keycode->CurrentNumberOfKeys++] = Key;
}

VOID
AmiShimKeycodeTranslateModifiers (
  IN AMI_SHIM_KEYCODE_INSTANCE *Keycode,
  IN UINT32                    KeyShiftState,
  IN BOOLEAN                   PassModifiers
  )
{
  APPLE_MODIFIER_MAP ModifierMap;
  
  ModifierMap = 0;

  if (KeyShiftState & EFI_SHIFT_STATE_VALID) {
    if (KeyShiftState & EFI_RIGHT_SHIFT_PRESSED) {
      ModifierMap |= gModifierRemap[AmiShimRightShift];
    }
    if (KeyShiftState & EFI_LEFT_SHIFT_PRESSED) {
      ModifierMap |= gModifierRemap[AmiShimRightShift];
    }
    if (KeyShiftState & EFI_RIGHT_CONTROL_PRESSED) {
      ModifierMap |= gModifierRemap[AmiShimRightControl];
    }
    if (KeyShiftState & EFI_LEFT_CONTROL_PRESSED) {
      ModifierMap |= gModifierRemap[AmiShimLeftControl];
    }
    if (KeyShiftState & EFI_RIGHT_ALT_PRESSED) {
      ModifierMap |= gModifierRemap[AmiShimRightAlt];
    }
    if (KeyShiftState & EFI_LEFT_ALT_PRESSED) {
      ModifierMap |= gModifierRemap[AmiShimLeftAlt];
    }
    if (KeyShiftState & EFI_RIGHT_LOGO_PRESSED) {
      ModifierMap |= gModifierRemap[AmiShimRightGui];
    }
    if (KeyShiftState & EFI_LEFT_LOGO_PRESSED) {
      ModifierMap |= gModifierRemap[AmiShimLeftGui];
    }
  }

  if (ModifierMap != Keycode->CurrentModifiers) {
    if (PassModifiers) {
      Keycode->CurrentModifiers = ModifierMap;  
    }    
    AmiShimKeycodeSendData (Keycode, TRUE, PassModifiers);
    Keycode->CurrentModifiers = ModifierMap;
  }
}

void AmiShimUpdateNumpad (
  IN OUT UINT8 *UsbKey,
  IN EFI_KEY   EfiKey
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

EFI_STATUS
AmiShimKeycodeAppendData (
  IN AMI_SHIM_KEYCODE_INSTANCE *Keycode,
  IN AMI_EFI_KEY_DATA          *KeyData
  )
{
  if (KeyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (KeyData->PS2ScanCodeIsValid == 1 && KeyData->PS2ScanCode < MAX_PS2_NUM) {
    PS2_TO_USB_MAP Key = gPs2ToUsbMap[KeyData->PS2ScanCode];
    if (Key.UsbCode != 0) {
      AmiShimKeycodeTranslateModifiers (Keycode, KeyData->KeyState.KeyShiftState, FALSE);
      AmiShimUpdateNumpad(&Key.UsbCode, KeyData->EfiKey);
      AmiShimKeycodeInsertKey (Keycode, Key.UsbCode, ((Keycode->CurrentModifiers & USB_HID_KB_KP_MODIFIERS_SHIFT) ? Key.ShiftKeyName : Key.KeyName));
      //DEBUG ((EFI_D_ERROR, "AmiShim received usb %d ps2 %d key %a efik %a code %X uni %X\n", Key.UsbCode, KeyData->PS2ScanCode, Key.KeyName, 
      //  KeyData->EfiKey < gEfiKeyToNameMapNum ? gEfiKeyToNameMap[KeyData->EfiKey] : "<err>", 
      //  KeyData->Key.ScanCode, KeyData->Key.UnicodeChar));
      return EFI_SUCCESS;
    }
    //DEBUG ((EFI_D_ERROR, "Received untranslatable PS/2 scancode %d [%X]\n", KeyData->PS2ScanCode, KeyData->KeyState.KeyShiftState));
  } else {
    //DEBUG ((EFI_D_ERROR, "Received invalid PS/2 scancode %d [%X]\n", KeyData->PS2ScanCode, KeyData->KeyState.KeyShiftState));
    AmiShimKeycodeTranslateModifiers (Keycode, KeyData->KeyState.KeyShiftState, TRUE);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
AmiShimKeycodeReadEfikey (
  IN  AMI_EFIKEYCODE_PROTOCOL *This,
  OUT AMI_EFI_KEY_DATA        *KeyData
  )
{
  EFI_STATUS                Status;
  UINTN                     Index;
  AMI_SHIM_KEYCODE_INSTANCE *Keycode;

  for (Index = 0; Index < MAX_KEYCODES; Index++) {
    Keycode = &mAmiShimKeycode.KeycodeMap[Index];
    if (This != NULL && Keycode->Protocol == This) {
      break;
    }
  }

  if (Index == MAX_KEYCODES) {
    return EFI_INVALID_PARAMETER;
  }

  if (!Keycode->PerformingManualPoll) {
    // Returning missed keycode
    if (AmiShimKeycodeReadKeysBuffer (Keycode, KeyData)) {
      return EFI_SUCCESS;
    }
  }

  Status = Keycode->OriginalReadEfikey (This, KeyData);

  if (Status == EFI_SUCCESS) {
    AmiShimKeycodeAppendData (Keycode, KeyData);
    if (Keycode->PerformingManualPoll) {
      // Saving keycode
      AmiShimKeycodeWriteKeysBuffer (Keycode, KeyData);
    }
  } else if (Status == EFI_NOT_READY) {
    AmiShimKeycodeSendData (Keycode, TRUE, FALSE);
  } else {
    DEBUG ((EFI_D_ERROR, "AmiShimReadEfikey called with unexpected %d\n", Status));
  }

  return Status;
}

VOID
EFIAPI
AmiShimKeycodeReadEfikeyHandler (
  IN EFI_EVENT  Event, 
  IN VOID       *Context
  )
{
  AMI_EFI_KEY_DATA          KeyData;
  UINTN                     Index;
  AMI_SHIM_KEYCODE_INSTANCE *Keycode;

  for (Index = 0; Index < MAX_KEYCODES; Index++) {
    Keycode = &mAmiShimKeycode.KeycodeMap[Index];
    Keycode->PerformingManualPoll = TRUE;
    AmiShimKeycodeReadEfikey (Keycode->Protocol, &KeyData);
    Keycode->PerformingManualPoll = FALSE;
  }
}

EFI_STATUS
EFIAPI
AmiShimProtocolArriveInstall (
  VOID
  )
{
  EFI_STATUS    Status;
  EFI_STATUS    Failed;
  VOID          *Registration;

  Failed = EFI_SUCCESS;

  if (mAmiShimKeycode.KeycodeArriveEvent == NULL) {
    Status = gBS->CreateEvent (EVT_NOTIFY_SIGNAL, TPL_NOTIFY, AmiShimProtocolArriveHandler, NULL, &mAmiShimKeycode.KeycodeArriveEvent);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "AmiShimProtocolArriveHandler KeycodeArriveEvent creation failed %d\n", Status));
      Failed = Status;
    } else {
      Status = gBS->RegisterProtocolNotify (&gAmiEfiKeycodeProtocolGuid, mAmiShimKeycode.KeycodeArriveEvent, &Registration);

      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "AmiShimProtocolArriveHandler KeycodeArriveEvent registration failed %d\n", Status));
        gBS->CloseEvent (mAmiShimKeycode.KeycodeArriveEvent);
        Failed = Status;
      }
    }
  }

  if (mAmiShimKeycode.KeyMapDbArriveEvent == NULL) {
    Status = gBS->CreateEvent (EVT_NOTIFY_SIGNAL, TPL_NOTIFY, AmiShimProtocolArriveHandler, NULL, &mAmiShimKeycode.KeyMapDbArriveEvent);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "AmiShimProtocolArriveHandler KeyMapDbArriveEvent creation failed %d\n", Status));
      Failed = Status;
    } else {
      Status = gBS->RegisterProtocolNotify (&gAppleKeyMapDatabaseProtocolGuid, mAmiShimKeycode.KeyMapDbArriveEvent, &Registration);

      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "AmiShimProtocolArriveHandler KeyMapDbArriveEvent registration failed %d\n", Status));
        gBS->CloseEvent (mAmiShimKeycode.KeyMapDbArriveEvent);
        Failed = Status;
      }
    }  
  }

  return Failed;
}

EFI_STATUS
EFIAPI
AmiShimProtocolArriveUninstall (
  VOID
  )
{
  EFI_STATUS Status;
  EFI_STATUS Failed;

  Failed = EFI_SUCCESS;

  if (mAmiShimKeycode.KeycodeArriveEvent != NULL) {
    Status = gBS->CloseEvent (mAmiShimKeycode.KeycodeArriveEvent);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "AmiShimProtocolArriveHandler KeycodeArriveEvent closing failed %d\n", Status));
      Failed = Status;
    } else {
      mAmiShimKeycode.KeycodeArriveEvent = NULL;
    }
  }

  if (mAmiShimKeycode.KeyMapDbArriveEvent != NULL) {
    Status = gBS->CloseEvent (mAmiShimKeycode.KeyMapDbArriveEvent);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "AmiShimProtocolArriveHandler KeyMapDbArriveEvent closing failed %d\n", Status));
      Failed = Status;
    } else {
      mAmiShimKeycode.KeyMapDbArriveEvent = NULL;
    }
  }

  return Failed;
}

EFI_STATUS
EFIAPI
AmiShimKeycodeInstallOnHandle (
  IN EFI_HANDLE                   DeviceHandle,
  IN AMI_EFIKEYCODE_PROTOCOL      *EfiKeycode
  )
{
  UINTN                      Index;
  AMI_SHIM_KEYCODE_INSTANCE  *Keycode;
  AMI_SHIM_KEYCODE_INSTANCE  *FreeKeycode;
  EFI_STATUS                 Status;
  
  FreeKeycode = NULL;

  for (Index = 0; Index < MAX_KEYCODES; Index++) {
    Keycode = &mAmiShimKeycode.KeycodeMap[Index];
    if (Keycode->DeviceHandle == NULL && FreeKeycode == NULL) {
      FreeKeycode = Keycode;
    } else if (Keycode->DeviceHandle == DeviceHandle) {
      return EFI_ALREADY_STARTED;
    }
  }

  if (FreeKeycode == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = mAmiShimKeycode.KeyMapDb->CreateKeyStrokesBuffer (mAmiShimKeycode.KeyMapDb, MAX_KEY_NUM, &FreeKeycode->KeyMapDbIndex);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "CreateKeyStrokesBuffer failed %d\n", Status));
    return Status;
  }

  DEBUG ((EFI_D_ERROR, "Installed onto %X\n", DeviceHandle));
  FreeKeycode->DeviceHandle = DeviceHandle;
  FreeKeycode->Protocol = EfiKeycode;
  if (FreeKeycode->Protocol->ReadEfikey == AmiShimKeycodeReadEfikey) {
    FreeKeycode->OriginalReadEfikey = NULL;
    DEBUG ((EFI_D_ERROR, "Function is already hooked\n"));
  } else {
    FreeKeycode->OriginalReadEfikey = FreeKeycode->Protocol->ReadEfikey;
    FreeKeycode->Protocol->ReadEfikey = AmiShimKeycodeReadEfikey;
  }

  AmiShimKeycodeInitKeysBuffer (FreeKeycode);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
AmiShimKeycodeInstall (
  VOID
  )
{
  EFI_STATUS                Status;
  UINTN                     NoHandles;
  EFI_HANDLE                *Handles;
  UINTN                     Index;
  EFI_USB_IO_PROTOCOL       *UsbIo;
  AMI_EFIKEYCODE_PROTOCOL   *EfiKeycode;
  BOOLEAN                   Installed;

  Status = gBS->LocateProtocol (&gAppleKeyMapDatabaseProtocolGuid, NULL, (VOID **)&mAmiShimKeycode.KeyMapDb);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "gAppleKeyMapDatabaseProtocolGuid is unavailable %d\n", Status));
    return EFI_NOT_FOUND;
  }

  Status = gBS->LocateHandleBuffer (ByProtocol, &gAmiEfiKeycodeProtocolGuid, NULL, &NoHandles, &Handles);
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }

  DEBUG ((EFI_D_ERROR, "Found %d keycode handles\n", NoHandles));

  for (Index = 0; Index < NoHandles; Index++) {
    Status = gBS->HandleProtocol (Handles[Index], &gAmiEfiKeycodeProtocolGuid, (VOID **)&EfiKeycode);
    if (EFI_ERROR(Status)) {
      DEBUG ((EFI_D_ERROR, "Handle %d has no EfiKeycode (wtf?) %d\n", Index, Status));
      continue;
    }

    Status = gBS->HandleProtocol (Handles[Index], &gEfiUsbIoProtocolGuid, (VOID **)&UsbIo);
    if (!EFI_ERROR(Status)) {
      DEBUG ((EFI_D_ERROR, "Only splitter and PS/2 should be polled, skipping\n"));
      continue;
    }

    Status = AmiShimKeycodeInstallOnHandle (Handles[Index], EfiKeycode);
    if (!EFI_ERROR(Status)) {
      Installed = TRUE;
    }
  }

  gBS->FreePool(Handles);

  if (mAmiShimKeycode.ReadEfiKeyEvent == NULL && Installed) {
    Status = gBS->CreateEvent (EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_NOTIFY, AmiShimKeycodeReadEfikeyHandler, NULL, &mAmiShimKeycode.ReadEfiKeyEvent);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "AmiShimReadEfikeyHandler event creation failed %d\n", Status));
      return Status;
    }

    Status = gBS->SetTimer (mAmiShimKeycode.ReadEfiKeyEvent, TimerPeriodic, READ_EFIKEY_TIMER_INTERVAL);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "AmiShimReadEfikeyHandler timer setting failed %d\n", Status));
      gBS->CloseEvent (mAmiShimKeycode.ReadEfiKeyEvent);
      return Status;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
AmiShimKeycodeUninstall (
  VOID
  )
{
  EFI_STATUS                  Status;
  UINTN                       Index;
  AMI_SHIM_KEYCODE_INSTANCE   *Keycode;

  AmiShimProtocolArriveUninstall();

  if (mAmiShimKeycode.ReadEfiKeyEvent) {
    Status = gBS->SetTimer (mAmiShimKeycode.ReadEfiKeyEvent, TimerCancel, 0);

    if (!EFI_ERROR (Status)) {
      Status = gBS->CloseEvent (mAmiShimKeycode.ReadEfiKeyEvent);

      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "AmiShimReadEfikeyHandler event closing failed %d\n", Status));
      } else {
        mAmiShimKeycode.ReadEfiKeyEvent = NULL;
      }
    } else {
      DEBUG ((EFI_D_ERROR, "AmiShimReadEfikeyHandler timer unsetting failed %d\n", Status));
    }
  }

  for (Index = 0; Index < MAX_KEYCODES; Index++) {
    Keycode = &mAmiShimKeycode.KeycodeMap[Index];
    if (Keycode->Protocol == NULL) {
      continue;
    }

    mAmiShimKeycode.KeyMapDb->RemoveKeyStrokesBuffer (mAmiShimKeycode.KeyMapDb, Keycode->KeyMapDbIndex);

    if (Keycode->OriginalReadEfikey) {
      Keycode->Protocol->ReadEfikey = Keycode->OriginalReadEfikey;
      Keycode->OriginalReadEfikey = NULL; 
    }

    gBS->DisconnectController(Keycode->DeviceHandle, NULL, NULL);

    Keycode->Protocol = NULL;
  }

  return EFI_SUCCESS;
}

VOID
EFIAPI
AmiShimProtocolArriveHandler (
  IN EFI_EVENT  Event, 
  IN VOID       *Context
  )
{
  EFI_STATUS Status;

  Status = AmiShimKeycodeInstall();

  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "AmiShimKeycodeInstall failed %d\n", Status));
  }
}

EFI_STATUS
EFIAPI
AmiShimKeycodeInit (
  VOID
  )
{
  EFI_STATUS                Status;

  Status = AmiShimKeycodeInstall();
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "AmiShim installation failed %d\n", Status));
  }

  Status = AmiShimProtocolArriveInstall();
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "AmiShim is NOT waiting for protocols %d\n", Status));
  }
  
  return Status;
}

EFI_STATUS
EFIAPI
AmiShimKeycodeExit (
  VOID
  )
{
  return AmiShimKeycodeUninstall();
}
