#!/bin/bash

BUILDDIR=$(dirname "$0")
pushd "$BUILDDIR" >/dev/null
BUILDDIR=$(pwd)
popd >/dev/null

NASMVER="2.13.02"

cd "$BUILDDIR"

prompt() {
  echo "$1"
  read -p "Enter [Y]es to continue: " v
  if [ "$v" != "Y" ] && [ "$v" != "y" ]; then
    exit 1
  fi
}

updaterepo() {
  if [ ! -d "$2" ]; then
    git clone "$1" "$2" || exit 1
  fi
  pushd "$2" >/dev/null
  git pull
  if [ "$3" != "" ]; then
    git checkout "$3"
  fi
  popd >/dev/null
}

if [ "$BUILDDIR" != "$(printf "%s\n" $BUILDDIR)" ]; then
  echo "EDK2 build system may still fail to support directories with spaces!"
  exit 1
fi

if [ "$(which clang)" = "" ] || [ "$(which git)" = "" ] || [ "$(clang -v 2>&1 | grep "no developer")" != "" ] || [ "$(git -v 2>&1 | grep "no developer")" != "" ]; then
  echo "Missing Xcode tools, please install them!"
  exit 1
fi

if [ "$(nasm -v)" = "" ] || [ "$(nasm -v | grep Apple)" != "" ]; then
  echo "Missing or incompatible nasm!"
  echo "Download the latest nasm from http://www.nasm.us/pub/nasm/releasebuilds/"
  prompt "Last tested with nasm $NASMVER. Install it automatically?"
  pushd /tmp >/dev/null
  rm -rf "nasm-${NASMVER}-macosx.zip" "nasm-${NASMVER}"
  curl -O "http://www.nasm.us/pub/nasm/releasebuilds/${NASMVER}/macosx/nasm-${NASMVER}-macosx.zip" || exit 1
  unzip -q "nasm-${NASMVER}-macosx.zip" "nasm-${NASMVER}/nasm" "nasm-${NASMVER}/ndisasm" || exit 1
  sudo mkdir -p /usr/local/bin || exit 1
  sudo mv "nasm-${NASMVER}/nasm" /usr/local/bin/ || exit 1
  sudo mv "nasm-${NASMVER}/ndisasm" /usr/local/bin/ || exit 1
  rm -rf "nasm-${NASMVER}-macosx.zip" "nasm-${NASMVER}"
  popd >/dev/null
fi

if [ "$(which mtoc.NEW)" == "" ] || [ "$(which mtoc)" == "" ]; then
  echo "Missing mtoc or mtoc.NEW!"
  echo "To build mtoc follow: https://github.com/tianocore/tianocore.github.io/wiki/Xcode#mac-os-x-xcode"
  prompt "Install prebuilt mtoc and mtoc.NEW automatically?"
  rm -f mtoc mtoc.NEW
  unzip -q external/mtoc-mac64.zip mtoc mtoc.NEW || exit 1
  sudo mkdir -p /usr/local/bin || exit 1
  sudo mv mtoc /usr/local/bin/ || exit 1
  sudo mv mtoc.NEW /usr/local/bin/ || exit 1
fi

if [ ! -d "binaries" ]; then
  mkdir binaries || exit 1
  cd binaries || exit 1
  ln -s ../edk2/Build/AptioFixPkg/RELEASE_XCODE5/X64 RELEASE || exit 1
  ln -s ../edk2/Build/AptioFixPkg/DEBUG_XCODE5/X64 DEBUG || exit 1
  cd .. || exit 1
fi

if [ "$1" != "" ]; then
  MODE="$1"
  shift
fi

if [ ! -f edk2/edk2.ready ]; then
  rm -rf edk2
fi

updaterepo "https://github.com/tianocore/edk2" edk2 || exit 1
cd edk2
updaterepo "https://github.com/CupertinoNet/CupertinoModulePkg" CupertinoModulePkg || exit 1
updaterepo "https://github.com/CupertinoNet/EfiMiscPkg" EfiMiscPkg || exit 1
updaterepo "https://github.com/CupertinoNet/EfiPkg" EfiPkg || exit 1

if [ ! -d AptioFixPkg ]; then
  ln -s .. AptioFixPkg || exit 1
fi

source edksetup.sh || exit 1
make -C BaseTools || exit 1
touch edk2.ready

if [ "$MODE" = "" ] || [ "$MODE" = "DEBUG" ]; then
  build -a X64 -b DEBUG -t XCODE5 -p AptioFixPkg/AptioFixPkg.dsc || exit 1
fi

if [ "$MODE" = "" ] || [ "$MODE" = "RELEASE" ]; then
  build -a X64 -b RELEASE -t XCODE5 -p AptioFixPkg/AptioFixPkg.dsc || exit 1
fi
