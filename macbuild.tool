#!/bin/bash

BUILDDIR=$(dirname "$0")
pushd "$BUILDDIR" >/dev/null
BUILDDIR=$(pwd)
popd >/dev/null

cd "$BUILDDIR"

if [ "$(nasm -v | grep Apple)" != "" ]; then
  echo "Incompatible nasm version!"
  echo "Download the latest nasm from http://www.nasm.us/pub/nasm/releasebuilds/"
  echo "Last tested with nasm 2.12.02 and 2.13.02."
  exit 1
fi

if [ "$(which mtoc.NEW)" == "" ] || [ "$(which mtoc)" == "" ]; then
  echo "Missing mtoc or mtoc.NEW!"
  echo "To build mtoc follow: https://github.com/tianocore/tianocore.github.io/wiki/Xcode#mac-os-x-xcode"
  echo "You may also use one in external directory."
  exit 1
fi


if [ "$1" != "" ]; then
  MODE="$1"
  shift
fi

if [ ! -f edk2/edk2.ready ]; then
  rm -rf edk2
  git clone https://github.com/tianocore/edk2 || exit 1
  cd edk2
  git clone https://github.com/CupertinoNet/CupertinoModulePkg || exit 1
  git clone https://github.com/CupertinoNet/EfiMiscPkg || exit 1
  git clone https://github.com/CupertinoNet/EfiPkg || exit 1
  ln -s ../AptioFixPkg AptioFixPkg
  source edksetup.sh || exit 1
  make -C BaseTools || exit 1
  touch edk2.ready
else
  cd edk2
  source edksetup.sh || exit 1
fi

if [ "$MODE" == "" ] || [ "$MODE" == "DEBUG" ]; then
  build -a X64 -b DEBUG -t XCODE5 -p AptioFixPkg/AptioFixPkg.dsc || exit 1
fi

if [ "$MODE" == "" ] || [ "$MODE" == "RELEASE" ]; then
  build -a X64 -b RELEASE -t XCODE5 -p AptioFixPkg/AptioFixPkg.dsc || exit 1
fi
