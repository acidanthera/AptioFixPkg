/** @file
  Header file for AmiEfiKeycode to KeyMapDb translator.

Copyright (c) 2016, CupertinoNet. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#ifndef _AMI_SHIM_KEYCODE_H_
#define _AMI_SHIM_KEYCODE_H_

#define MAX_KEYCODES        12
#define MAX_KEY_NUM         6
#define MAX_KEY_BUFFER_SIZE 12

typedef struct {
  EFI_HANDLE                       DeviceHandle;
  AMI_EFIKEYCODE_PROTOCOL          *Protocol;
  AMI_READ_EFI_KEY                 OriginalReadEfikey;
  UINTN                            KeyMapDbIndex;
  APPLE_MODIFIER_MAP               CurrentModifiers;
  APPLE_KEY_CODE                   CurrentKeys[MAX_KEY_NUM];
  UINTN                            CurrentNumberOfKeys;
  BOOLEAN                          CurrentDbHasData;
  BOOLEAN                          PerformingManualPoll;
  UINT8                            PendingKeysBufferSize;
  AMI_EFI_KEY_DATA                 PendingKeysBuffer[MAX_KEY_BUFFER_SIZE];
  AMI_EFI_KEY_DATA                 *PendingKeysBufferHead;
  AMI_EFI_KEY_DATA                 *PendingKeysBufferTail;
} AMI_SHIM_KEYCODE_INSTANCE;


typedef struct {
  AMI_SHIM_KEYCODE_INSTANCE        KeycodeMap[MAX_KEYCODES];
  APPLE_KEY_MAP_DATABASE_PROTOCOL  *KeyMapDb;
  EFI_EVENT                        KeyMapDbArriveEvent;
  EFI_EVENT                        KeycodeArriveEvent;
  EFI_EVENT                        ReadEfiKeyEvent;
} AMI_SHIM_KEYCODE;

VOID
EFIAPI
AmiShimProtocolArriveHandler (
  IN EFI_EVENT  Event, 
  IN VOID       *Context
  );

#define MAX_KEY_NUM 6
#define READ_EFIKEY_TIMER_INTERVAL EFI_TIMER_PERIOD_MILLISECONDS(10)

#endif
