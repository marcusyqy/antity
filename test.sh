#!/bin/sh

# Change to the directory of the script
pushd .
cd "$(dirname "$0")" || exit 1

set -eu

CCFLAGS="-Wall -O2 -Wextra -pedantic -Werror "
CPPFLAGS="$CCFLAGS -std=c++17"

BUILD_NAME=test
BUILD_SRCS="../test/test.cpp ../antity/build.cpp"


mkdir -p .build
pushd .build
cc $CPPFLAGS -o $BUILD_NAME $BUILD_SRCS && ./$BUILD_NAME
popd
