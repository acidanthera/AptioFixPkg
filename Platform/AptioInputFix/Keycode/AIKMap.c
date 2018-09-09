/** @file
  Key mapping tables.

Copyright (c) 2016, CupertinoNet. All rights reserved.<BR>
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
    0x00,
    NULL,
    NULL
  }, // 0x00
  {
    0x29,
    "Esc",
    "^ Esc ^"
  }, // 0x01
  {
    0x1E,
    "1",
    "!"
  }, // 0x02
  {
    0x1F,
    "2",
    "@"
  }, // 0x03
  {
    0x20,
    "3",
    "#"
  }, // 0x04
  {
    0x21,
    "4",
    "$"
  }, // 0x05
  {
    0x22,
    "5",
    "%"
  }, // 0x06
  {
    0x23,
    "6",
    "^"
  }, // 0x07
  {
    0x24,
    "7",
    "&"
  }, // 0x08
  {
    0x25,
    "8",
    "*"
  }, // 0x09
  {
    0x26,
    "9",
    "("
  }, // 0x0A
  {
    0x27,
    "0",
    ")"
  }, // 0x0B
  {
    0x2D,
    "-",
    "_"
  }, // 0x0C
  {
    0x2E,
    "=",
    "+"
  }, // 0x0D
  {
    0x2A,
    "Backspace",
    "^ Backspace ^"
  }, // 0x0E
  {
    0x2B,
    "Tab",
    "^ Tab ^"
  }, // 0x0F
  {
    0x14,
    "q",
    "Q"
  }, // 0x10
  {
    0x1A,
    "w",
    "W"
  }, // 0x11
  {
    0x08,
    "e",
    "E"
  }, // 0x12
  {
    0x15,
    "r",
    "R"
  }, // 0x13
  {
    0x17,
    "t",
    "T"
  }, // 0x14
  {
    0x1C,
    "y",
    "Y"
  }, // 0x15
  {
    0x18,
    "u",
    "U"
  }, // 0x16
  {
    0x0C,
    "i",
    "I"
  }, // 0x17
  {
    0x12,
    "o",
    "O"
  }, // 0x18
  {
    0x13,
    "p",
    "P"
  }, // 0x19
  {
    0x2F,
    "[",
    "{"
  }, // 0x1A
  {
    0x30,
    "]",
    "}"
  }, // 0x1B
  {
    0x28,
    "Enter",
    "^ Enter ^"
  }, // 0x1C
  {
    0x00,
    NULL,
    NULL
  }, // 0x1D
  {
    0x04,
    "a",
    "A"
  }, // 0x1E
  {
    0x16,
    "s",
    "S"
  }, // 0x1F
  {
    0x07,
    "d",
    "D"
  }, // 0x20
  {
    0x09,
    "f",
    "F"
  }, // 0x21
  {
    0x0A,
    "g",
    "G"
  }, // 0x22
  {
    0x0B,
    "h",
    "H"
  }, // 0x23
  {
    0x0D,
    "j",
    "J"
  }, // 0x24
  {
    0x0E,
    "k",
    "K"
  }, // 0x25
  {
    0x0F,
    "l",
    "L"
  }, // 0x26
  {
    0x33,
    ";",
    ":"
  }, // 0x27
  {
    0x34,
    "'",
    "\""
  }, // 0x28
  {
    0x35,
    "`",
    "~"
  }, // 0x29
  {
    0x00,
    NULL,
    NULL
  }, // 0x2A
  {
    0x31,
    "\\",
    "|"
  }, // 0x2B
  {
    0x1D,
    "z",
    "Z"
  }, // 0x2C
  {
    0x1B,
    "x",
    "X"
  }, // 0x2D
  {
    0x06,
    "c",
    "C"
  }, // 0x2E
  {
    0x19,
    "v",
    "V"
  }, // 0x2F
  {
    0x05,
    "b",
    "B"
  }, // 0x30
  {
    0x11,
    "n",
    "N"
  }, // 0x31
  {
    0x10,
    "m",
    "M"
  }, // 0x32
  {
    0x36,
    ",",
    "<"
  }, // 0x33
  {
    0x37,
    ".",
    ">"
  }, // 0x34
  {
    0x38,
    "/",
    "?"
  }, // 0x35
  {
    0x00,
    NULL,
    NULL
  }, // 0x36
  {
    0x55,
    "*",
    "^ * ^"
  }, // 0x37
  {
    0x00,
    NULL,
    NULL
  }, // 0x38
  {
    0x2C,
    "Spacebar",
    "^ Spacebar ^"
  }, // 0x39
  {
    0x39,
    "CapsLock",
    "^ CapsLock ^"
  }, // 0x3A
  {
    0x3A,
    "F1",
    "^ F1 ^"
  }, // 0x3B
  {
    0x3B,
    "F2",
    "^ F2 ^"
  }, // 0x3C
  {
    0x3C,
    "F3",
    "^ F3 ^"
  }, // 0x3D
  {
    0x3D,
    "F4",
    "^ F4 ^"
  }, // 0x3E
  {
    0x3E,
    "F5",
    "^ F5 ^"
  }, // 0x3F
  {
    0x3F,
    "F6",
    "^ F6 ^"
  }, // 0x40
  {
    0x40,
    "F7",
    "^ F7 ^"
  }, // 0x41
  {
    0x41,
    "F8",
    "^ F8 ^"
  }, // 0x42
  {
    0x42,
    "F9",
    "^ F9 ^"
  }, // 0x43
  {
    0x43,
    "F10",
    "^ F10 ^"
  }, // 0x44
  {
    0x53,
    "NumLock",
    "^ NumLock ^"
  }, // 0x45
  {
    0x47,
    "Scroll Lock",
    "^ Scroll Lock ^"
  }, // 0x46
  {
    0x4A,
    "Home",
    "^ Home ^"
  }, // 0x47
  {
    0x52,
    "Up",
    "^ Up ^"
  }, // 0x48
  {
    0x4B,
    "PageUp",
    "^ PageUp ^"
  }, // 0x49
  {
    0x56,
    "-",
    "^ - ^"
  }, // 0x4A
  {
    0x50,
    "Left",
    "^ Left ^"
  }, // 0x4B
  {
    0x5D,
    "5",
    "^ 5 ^"
  }, // 0x4C
  {
    0x4F,
    "Right",
    "^ Right ^"
  }, // 0x4D
  {
    0x57,
    "+",
    "^ + ^"
  }, // 0x4E
  {
    0x4D,
    "End",
    "^ End ^"
  }, // 0x4F
  {
    0x51,
    "Down",
    "^ Down ^"
  }, // 0x50
  {
    0x4E,
    "PageDown",
    "^ PageDown ^"
  }, // 0x51
  {
    0x49,
    "Insert",
    "^ Insert ^"
  }, // 0x52
  {
    0x4C,
    "Delete",
    "^ Delete ^"
  }, // 0x53
  {
    0x00,
    NULL,
    NULL
  }, // 0x54
  {
    0x00,
    NULL,
    NULL
  }, // 0x55
  {
    0x00,
    NULL,
    NULL
  }, // 0x56
  {
    0x44,
    "F11",
    "^ F11 ^"
  }, // 0x57
  {
    0x45,
    "F12",
    "^ F12 ^"
  }, // 0x58
  {
    0x00,
    NULL,
    NULL
  }, // 0x59
  {
    0x00,
    NULL,
    NULL
  }, // 0x5A
  {
    0x00,
    NULL,
    NULL
  }, // 0x5B
  {
    0x00,
    NULL,
    NULL
  }, // 0x5C
  {
    0x00,
    NULL,
    NULL
  }, // 0x5D
  {
    0x00,
    NULL,
    NULL
  }, // 0x5E
  {
    0x00,
    NULL,
    NULL
  }, // 0x5F
  {
    0x00,
    NULL,
    NULL
  }, // 0x60
  {
    0x00,
    NULL,
    NULL
  }, // 0x61
  {
    0x00,
    NULL,
    NULL
  }, // 0x62
  {
    0x00,
    NULL,
    NULL
  }, // 0x63
  {
    0x00,
    NULL,
    NULL
  }, // 0x64
  {
    0x00,
    NULL,
    NULL
  }, // 0x65
  {
    0x00,
    NULL,
    NULL
  }, // 0x66
  {
    0x00,
    NULL,
    NULL
  }, // 0x67
  {
    0x00,
    NULL,
    NULL
  }, // 0x68
  {
    0x00,
    NULL,
    NULL
  }, // 0x69
  {
    0x00,
    NULL,
    NULL
  }, // 0x6A
  {
    0x00,
    NULL,
    NULL
  }, // 0x6B
  {
    0x00,
    NULL,
    NULL
  }, // 0x6C
  {
    0x00,
    NULL,
    NULL
  }, // 0x6D
  {
    0x00,
    NULL,
    NULL
  }, // 0x6E
  {
    0x00,
    NULL,
    NULL
  }, // 0x6F
  {
    0x00,
    NULL,
    NULL
  }, // 0x70
  {
    0x00,
    NULL,
    NULL
  }, // 0x71
  {
    0x00,
    NULL,
    NULL
  }, // 0x72
  {
    0x00,
    NULL,
    NULL
  }, // 0x73
  {
    0x00,
    NULL,
    NULL
  }, // 0x74
  {
    0x00,
    NULL,
    NULL
  }, // 0x75
  {
    0x00,
    NULL,
    NULL
  }, // 0x76
  {
    0x00,
    NULL,
    NULL
  }, // 0x77
  {
    0x00,
    NULL,
    NULL
  }, // 0x78
  {
    0x00,
    NULL,
    NULL
  }, // 0x79
  {
    0x00,
    NULL,
    NULL
  }, // 0x7A
  {
    0x00,
    NULL,
    NULL
  }, // 0x7B
  {
    0x00,
    NULL,
    NULL
  }, // 0x7C
  {
    0x00,
    NULL,
    NULL
  }, // 0x7D
  {
    0x00,
    NULL,
    NULL
  }, // 0x7E
  {
    0x00,
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
