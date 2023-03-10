#!/bin/bash

# Make sure CXX environment variable is set.
if [ -z "$CXX" ]
then
    echo "error: CXX environment variable is not set. Please set this to your preferred C++ compiler."
    exit
fi

# Introduction
echo "Building libxdt [linux]..."

# Setting up directories
echo "Creating directories..."
mkdir -p tmp
mkdir -p lib/linux

# Compiling
echo "Compiling object files..."
${CXX} -c -O3 -o tmp/Item.o src/libxdt/Item.cpp -Iinclude -std=c++17
${CXX} -c -O3 -o tmp/Table.o src/libxdt/Table.cpp -Iinclude -std=c++17
${CXX} -c -O3 -o tmp/Compression.o src/libxdt/Compression.cpp -Iinclude -std=c++17

echo "Building static library..."
rm lib/linux/libxdt.a
ar rcs lib/linux/libxdt.a tmp/Item.o tmp/Table.o tmp/Compression.o

# Cleaning up
echo "Removing temporary directories..."
rm -r tmp