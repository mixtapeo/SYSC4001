#!/bin/bash
mkdir -p bin
g++ -std=c++17 -Wall -o bin/interrupts interrupts.cpp
echo "✅ Build complete — executable created at bin/interrupts"
