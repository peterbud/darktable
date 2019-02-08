#!/usr/bin/env bash
#
# Environment variables:
#
# SOURCE_DIR: Set to the directory of the darktable source (optional)
#     If not set, it will be derived relative to this script.

set -e

SOURCE_DIR=${SOURCE_DIR:-$( cd "$( dirname "${BASH_SOURCE[0]}" )" && dirname $( pwd ) )}
BUILD_DIR=$(pwd)
CC=${CC:-cc}

indent() { sed "s/^/    /"; }

echo "Source directory: ${SOURCE_DIR}"
echo "Build directory:  ${BUILD_DIR}"
echo ""
echo "Operating system version:"
uname -a 2>&1 | indent
echo "CMake version:"
cmake --version 2>&1 | indent
echo "Compiler version:"
$CC --version 2>&1 | indent
echo ""

echo "##############################################################################"
echo "## Configuring build environment"
echo "##############################################################################"

mkdir build && cd build
echo $PATH
which cmake
#cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release ${SOURCE_DIR}

echo ""
echo "##############################################################################"
echo "## Building darktable"
echo "##############################################################################"

cmake --build .
