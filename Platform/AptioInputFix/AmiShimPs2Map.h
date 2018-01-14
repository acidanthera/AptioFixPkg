/** @file
  Header file for PS/2 code to USB code mapping.

Copyright (c) 2016, CupertinoNet. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#ifndef _AMI_SHIM_PS2_H_
#define _AMI_SHIM_PS2_H_

#define MAX_PS2_NUM 128

typedef struct {
  UINT8         UsbCode;
  CONST CHAR8   *KeyName;
  CONST CHAR8   *ShiftKeyName;
} PS2_TO_USB_MAP;

extern PS2_TO_USB_MAP gPs2ToUsbMap[MAX_PS2_NUM];

#endif