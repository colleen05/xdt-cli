#!/bin/bash

./build_libxdt_linux.sh

# Make sure CXX environment variable is set.
if [ -z "$CXX" ]
then
    echo "error: CXX environment variable is not set. Please set this to your preferred C++ compiler."
    exit
fi

# Introduction
echo "Building xdt-cli [linux]..."

# Setting up directories
echo "Creating directories..."
mkdir -p bin/linux

# Compiling
echo "Compiling..."
${CXX} -O3 -o bin/linux/xdt src/Main.cpp -Iinclude -Llib/linux -lxdt --std=c++17