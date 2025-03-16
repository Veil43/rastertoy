#!/bin/bash

if [ -z "$1" ]; then
    echo "Please provide a mode: \"debug\" or \"release\"."
    exit 1
fi

MODE=$1

INCLUDE_DIR="./include"
SRC_DIR="./src"
LIB_DIR="./lib"

COMMON_FLAGS="-I$INCLUDE_DIR"

if [ $MODE = "debug" ]; then
    echo "Building in Debug mode..."
    BUILD_FLAGS="-g"
    LIBS="-lSDL2"
    TARGET_DIR="debug"
elif  [ $MODE = "release" ];
then
    echo "Building in Release mode..."
    BUILD_FLAGS="-O2"
    LIBS="-lSDL2"
    TARGET_DIR="release"
else
    echo "Invalid mode! Use \"debug\" or \"release\"."
    exit 1
fi

# Create the target directory if it doesn't exist
mkdir -p $TARGET_DIR

# Compile the program using gcc
g++ --std=c++11 $COMMON_FLAGS $BUILD_FLAGS $SRC_DIR/sdl2_rastertoy.cpp $SRC_DIR/rastertoy.cpp -o $TARGET_DIR/sdl2_rastertoy $LIBS
