/** @file
  Key mapping tables.

Copyright (c) 2018, vit9696. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "AIKTranslate.h"

// Conversion table
AIK_PS2_TO_USB_MAP
gAikPs2ToUsbMap[AIK_MAX_PS2_NUM] = {
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x00
  {
    UsbHidUsageIdKbKpKeyEsc,
    "Esc",
    "^ Esc ^"
  }, // 0x01
  {
    UsbHidUsageIdKbKpKeyOne,
    "1",
    "!"
  }, // 0x02
  {
    UsbHidUsageIdKbKpKeyTwo,
    "2",
    "@"
  }, // 0x03
  {
    UsbHidUsageIdKbKpKeyThree,
    "3",
    "#"
  }, // 0x04
  {
    UsbHidUsageIdKbKpKeyFour,
    "4",
    "$"
  }, // 0x05
  {
    UsbHidUsageIdKbKpKeyFive,
    "5",
    "%"
  }, // 0x06
  {
    UsbHidUsageIdKbKpKeySix,
    "6",
    "^"
  }, // 0x07
  {
    UsbHidUsageIdKbKpKeySeven,
    "7",
    "&"
  }, // 0x08
  {
    UsbHidUsageIdKbKpKeyEight,
    "8",
    "*"
  }, // 0x09
  {
    UsbHidUsageIdKbKpKeyNine,
    "9",
    "("
  }, // 0x0A
  {
    UsbHidUsageIdKbKpKeyZero,
    "0",
    ")"
  }, // 0x0B
  {
    UsbHidUsageIdKbKpKeyMinus,
    "-",
    "_"
  }, // 0x0C
  {
    UsbHidUsageIdKbKpKeyEquals,
    "=",
    "+"
  }, // 0x0D
  {
    UsbHidUsageIdKbKpKeyBackSpace,
    "Backspace",
    "^ Backspace ^"
  }, // 0x0E
  {
    UsbHidUsageIdKbKpKeyTab,
    "Tab",
    "^ Tab ^"
  }, // 0x0F
  {
    UsbHidUsageIdKbKpKeyQ,
    "q",
    "Q"
  }, // 0x10
  {
    UsbHidUsageIdKbKpKeyW,
    "w",
    "W"
  }, // 0x11
  {
    UsbHidUsageIdKbKpKeyE,
    "e",
    "E"
  }, // 0x12
  {
    UsbHidUsageIdKbKpKeyR,
    "r",
    "R"
  }, // 0x13
  {
    UsbHidUsageIdKbKpKeyT,
    "t",
    "T"
  }, // 0x14
  {
    UsbHidUsageIdKbKpKeyY,
    "y",
    "Y"
  }, // 0x15
  {
    UsbHidUsageIdKbKpKeyU,
    "u",
    "U"
  }, // 0x16
  {
    UsbHidUsageIdKbKpKeyI,
    "i",
    "I"
  }, // 0x17
  {
    UsbHidUsageIdKbKpKeyO,
    "o",
    "O"
  }, // 0x18
  {
    UsbHidUsageIdKbKpKeyP,
    "p",
    "P"
  }, // 0x19
  {
    UsbHidUsageIdKbKpKeyLeftBracket,
    "[",
    "{"
  }, // 0x1A
  {
    UsbHidUsageIdKbKpKeyRightBracket,
    "]",
    "}"
  }, // 0x1B
  {
    UsbHidUsageIdKbKpKeyEnter,
    "Enter",
    "^ Enter ^"
  }, // 0x1C
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x1D
  {
    UsbHidUsageIdKbKpKeyA,
    "a",
    "A"
  }, // 0x1E
  {
    UsbHidUsageIdKbKpKeyS,
    "s",
    "S"
  }, // 0x1F
  {
    UsbHidUsageIdKbKpKeyD,
    "d",
    "D"
  }, // 0x20
  {
    UsbHidUsageIdKbKpKeyF,
    "f",
    "F"
  }, // 0x21
  {
    UsbHidUsageIdKbKpKeyG,
    "g",
    "G"
  }, // 0x22
  {
    UsbHidUsageIdKbKpKeyH,
    "h",
    "H"
  }, // 0x23
  {
    UsbHidUsageIdKbKpKeyJ,
    "j",
    "J"
  }, // 0x24
  {
    UsbHidUsageIdKbKpKeyK,
    "k",
    "K"
  }, // 0x25
  {
    UsbHidUsageIdKbKpKeyL,
    "l",
    "L"
  }, // 0x26
  {
    UsbHidUsageIdKbKpKeySemicolon,
    ";",
    ":"
  }, // 0x27
  {
    UsbHidUsageIdKbKpKeyQuotation,
    "'",
    "\""
  }, // 0x28
  {
    UsbHidUsageIdKbKpKeyAcute,
    "`",
    "~"
  }, // 0x29
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x2A
  {
    UsbHidUsageIdKbKpKeyBackslash,
    "\\",
    "|"
  }, // 0x2B
  {
    UsbHidUsageIdKbKpKeyZ,
    "z",
    "Z"
  }, // 0x2C
  {
    UsbHidUsageIdKbKpKeyX,
    "x",
    "X"
  }, // 0x2D
  {
    UsbHidUsageIdKbKpKeyC,
    "c",
    "C"
  }, // 0x2E
  {
    UsbHidUsageIdKbKpKeyV,
    "v",
    "V"
  }, // 0x2F
  {
    UsbHidUsageIdKbKpKeyB,
    "b",
    "B"
  }, // 0x30
  {
    UsbHidUsageIdKbKpKeyN,
    "n",
    "N"
  }, // 0x31
  {
    UsbHidUsageIdKbKpKeyM,
    "m",
    "M"
  }, // 0x32
  {
    UsbHidUsageIdKbKpKeyComma,
    ",",
    "<"
  }, // 0x33
  {
    UsbHidUsageIdKbKpKeyPeriod,
    ".",
    ">"
  }, // 0x34
  {
    UsbHidUsageIdKbKpKeySlash,
    "/",
    "?"
  }, // 0x35
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x36
  {
    UsbHidUsageIdKbKpPadKeyAsterisk,
    "*",
    "^ * ^"
  }, // 0x37
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x38
  {
    UsbHidUsageIdKbKpKeySpaceBar,
    "Spacebar",
    "^ Spacebar ^"
  }, // 0x39
  {
    UsbHidUsageIdKbKpKeyCLock,
    "CapsLock",
    "^ CapsLock ^"
  }, // 0x3A
  {
    UsbHidUsageIdKbKpKeyF1,
    "F1",
    "^ F1 ^"
  }, // 0x3B
  {
    UsbHidUsageIdKbKpKeyF2,
    "F2",
    "^ F2 ^"
  }, // 0x3C
  {
    UsbHidUsageIdKbKpKeyF3,
    "F3",
    "^ F3 ^"
  }, // 0x3D
  {
    UsbHidUsageIdKbKpKeyF4,
    "F4",
    "^ F4 ^"
  }, // 0x3E
  {
    UsbHidUsageIdKbKpKeyF5,
    "F5",
    "^ F5 ^"
  }, // 0x3F
  {
    UsbHidUsageIdKbKpKeyF6,
    "F6",
    "^ F6 ^"
  }, // 0x40
  {
    UsbHidUsageIdKbKpKeyF7,
    "F7",
    "^ F7 ^"
  }, // 0x41
  {
    UsbHidUsageIdKbKpKeyF8,
    "F8",
    "^ F8 ^"
  }, // 0x42
  {
    UsbHidUsageIdKbKpKeyF9,
    "F9",
    "^ F9 ^"
  }, // 0x43
  {
    UsbHidUsageIdKbKpKeyF10,
    "F10",
    "^ F10 ^"
  }, // 0x44
  {
    UsbHidUsageIdKbKpPadKeyNLck,
    "NumLock",
    "^ NumLock ^"
  }, // 0x45
  {
    UsbHidUsageIdKbKpKeySLock,
    "Scroll Lock",
    "^ Scroll Lock ^"
  }, // 0x46
  {
    UsbHidUsageIdKbKpKeyHome,
    "Home",
    "^ Home ^"
  }, // 0x47
  {
    UsbHidUsageIdKbKpKeyUpArrow,
    "Up",
    "^ Up ^"
  }, // 0x48
  {
    UsbHidUsageIdKbKpKeyPgUp,
    "PageUp",
    "^ PageUp ^"
  }, // 0x49
  {
    UsbHidUsageIdKbKpPadKeyMinus,
    "-",
    "^ - ^"
  }, // 0x4A
  {
    UsbHidUsageIdKbKpKeyLeftArrow,
    "Left",
    "^ Left ^"
  }, // 0x4B
  {
    UsbHidUsageIdKbKpPadKeyFive,
    "5",
    "^ 5 ^"
  }, // 0x4C
  {
    UsbHidUsageIdKbKpKeyRightArrow,
    "Right",
    "^ Right ^"
  }, // 0x4D
  {
    UsbHidUsageIdKbKpPadKeyPlus,
    "+",
    "^ + ^"
  }, // 0x4E
  {
    UsbHidUsageIdKbKpKeyEnd,
    "End",
    "^ End ^"
  }, // 0x4F
  {
    UsbHidUsageIdKbKpKeyDownArrow,
    "Down",
    "^ Down ^"
  }, // 0x50
  {
    UsbHidUsageIdKbKpKeyPgDn,
    "PageDown",
    "^ PageDown ^"
  }, // 0x51
  {
    UsbHidUsageIdKbKpKeyIns,
    "Insert",
    "^ Insert ^"
  }, // 0x52
  {
    UsbHidUsageIdKbKpKeyDel,
    "Delete",
    "^ Delete ^"
  }, // 0x53
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x54
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x55
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x56
  {
    UsbHidUsageIdKbKpKeyF11,
    "F11",
    "^ F11 ^"
  }, // 0x57
  {
    UsbHidUsageIdKbKpKeyF12,
    "F12",
    "^ F12 ^"
  }, // 0x58
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x59
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x5A
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x5B
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x5C
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x5D
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x5E
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x5F
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x60
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x61
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x62
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x63
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x64
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x65
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x66
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x67
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x68
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x69
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x6A
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x6B
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x6C
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x6D
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x6E
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x6F
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x70
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x71
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x72
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x73
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x74
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x75
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x76
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x77
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x78
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x79
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x7A
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x7B
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x7C
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x7D
  {
    UsbHidUndefined,
    NULL,
    NULL
  }, // 0x7E
  {
    UsbHidUndefined,
    NULL,
    NULL
  } // 0x7F
};

CONST CHAR8 *
gAikEfiKeyToNameMap[AIK_MAX_EFIKEY_NUM] = {    
  "EfiKeyLCtrl",
  "EfiKeyA0",
  "EfiKeyLAlt",
  "EfiKeySpaceBar",
  "EfiKeyA2",
  "EfiKeyA3",
  "EfiKeyA4",
  "EfiKeyRCtrl",
  "EfiKeyLeftArrow",
  "EfiKeyDownArrow",
  "EfiKeyRightArrow",
  "EfiKeyZero",
  "EfiKeyPeriod",
  "EfiKeyEnter",
  "EfiKeyLShift",
  "EfiKeyB0",
  "EfiKeyB1",
  "EfiKeyB2",
  "EfiKeyB3",
  "EfiKeyB4",
  "EfiKeyB5",
  "EfiKeyB6",
  "EfiKeyB7",
  "EfiKeyB8",
  "EfiKeyB9",
  "EfiKeyB10",
  "EfiKeyRshift",
  "EfiKeyUpArrow",
  "EfiKeyOne",
  "EfiKeyTwo",
  "EfiKeyThree",
  "EfiKeyCapsLock",
  "EfiKeyC1",
  "EfiKeyC2",
  "EfiKeyC3",
  "EfiKeyC4",
  "EfiKeyC5",
  "EfiKeyC6",
  "EfiKeyC7",
  "EfiKeyC8",
  "EfiKeyC9",
  "EfiKeyC10",
  "EfiKeyC11",
  "EfiKeyC12",
  "EfiKeyFour",
  "EfiKeyFive",
  "EfiKeySix",
  "EfiKeyPlus",
  "EfiKeyTab",
  "EfiKeyD1",
  "EfiKeyD2",
  "EfiKeyD3",
  "EfiKeyD4",
  "EfiKeyD5",
  "EfiKeyD6",
  "EfiKeyD7",
  "EfiKeyD8",
  "EfiKeyD9",
  "EfiKeyD10",
  "EfiKeyD11",
  "EfiKeyD12",
  "EfiKeyD13",
  "EfiKeyDel",
  "EfiKeyEnd",
  "EfiKeyPgDn",
  "EfiKeySeven",
  "EfiKeyEight",
  "EfiKeyNine",
  "EfiKeyE0",
  "EfiKeyE1",
  "EfiKeyE2",
  "EfiKeyE3",
  "EfiKeyE4",
  "EfiKeyE5",
  "EfiKeyE6",
  "EfiKeyE7",
  "EfiKeyE8",
  "EfiKeyE9",
  "EfiKeyE10",
  "EfiKeyE11",
  "EfiKeyE12",
  "EfiKeyBackSpace",
  "EfiKeyIns",
  "EfiKeyHome",
  "EfiKeyPgUp",
  "EfiKeyNLck",
  "EfiKeySlash",
  "EfiKeyAsterisk",
  "EfiKeyMinus",
  "EfiKeyEsc",
  "EfiKeyF1",
  "EfiKeyF2",
  "EfiKeyF3",
  "EfiKeyF4",
  "EfiKeyF5",
  "EfiKeyF6",
  "EfiKeyF7",
  "EfiKeyF8",
  "EfiKeyF9",
  "EfiKeyF10",
  "EfiKeyF11",
  "EfiKeyF12",
  "EfiKeyPrint",
  "EfiKeySLck",
  "EfiKeyPause",
  "Unk105",
  "Unk106",
  "Unk107",
  "Unk108",
  "Unk109",
  "Unk110",
  "Unk111",
  "Unk112",
  "Unk113",
  "Unk114",
  "Unk115",
  "Unk116",
  "Unk117",
  "Unk118",
  "Unk119",
  "Unk120",
  "Unk121",
  "Unk122",
  "Unk123",
  "Unk124",
  "Unk125",
  "Unk126",
  "Unk127"
};
