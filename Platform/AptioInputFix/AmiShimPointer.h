/** @file
  Header file for AmiEfiPointer to EfiPointer translator.

Copyright (c) 2016, CupertinoNet. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#ifndef _AMI_SHIM_POINTER_H_
#define _AMI_SHIM_POINTER_H_

// Taken from AMI
#define MAX_POINTERS 6
#define POSITION_POLL_TIMER_INTERVAL 50000
#define AMI_SHIM_TIMER_PERIOD 50000
#define SCALE_FACTOR 1 // 1~4

typedef struct {
  EFI_HANDLE                    DeviceHandle;
  AMI_EFIPOINTER_PROTOCOL       *EfiPointer;
  EFI_SIMPLE_POINTER_PROTOCOL   *SimplePointer;
  EFI_SIMPLE_POINTER_GET_STATE  OriginalGetState;
  BOOLEAN                       PositionChanged;
  INT32                         PositionX;
  INT32                         PositionY;
  INT32                         PositionZ;
} AMI_SHIM_POINTER_INSTANCE;

typedef struct {
  BOOLEAN                       TimersInitialised;
  BOOLEAN                       UsageStarted;
  EFI_EVENT                     ProtocolArriveEvent;
  UINTN                         OriginalTimerPeriod;
  EFI_TIMER_ARCH_PROTOCOL       *TimerProtocol;
  EFI_EVENT                     PositionEvent;
  AMI_SHIM_POINTER_INSTANCE     PointerMap[MAX_POINTERS];
} AMI_SHIM_POINTER;

#endif
