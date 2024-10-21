#!/bin/bash

# Compiler
CXX=g++

# Compiler flags
CXXFLAGS="-Wall -Wextra -O2 -Wno-maybe-uninitialized"

# Target executable
TARGET="test"

# Source files
SRCS="test.cpp Beastdevices_INA3221.cpp"

# Include directories
INCLUDES="-I."

# Libraries
LIBS="-lwiringPi"

# Clean up object files
clean() {
    rm -f $OBJS $TARGET
}

# Always clean before build
clean

# Compile source files into object files
for SRC in $SRCS; do
    OBJ="${SRC%.cpp}.o"
    $CXX $CXXFLAGS $INCLUDES -c $SRC -o $OBJ
    OBJS="$OBJS $OBJ"
done

# Link object files to create the executable
$CXX $CXXFLAGS -o $TARGET $OBJS $LIBS



# Check for clean argument
if [ "$1" == "clean" ]; then
    clean
fi