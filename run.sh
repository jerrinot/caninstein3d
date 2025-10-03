#!/bin/bash
# Run the raycaster engine from project root

cd "$(dirname "$0")"

# Check for executable in various build directories
if [ -f "./cmake-build-debug/raycaster" ]; then
    ./cmake-build-debug/raycaster "$@"
elif [ -f "./build/raycaster" ]; then
    ./build/raycaster "$@"
elif [ -f "./raycaster" ]; then
    ./raycaster "$@"
else
    echo "Error: raycaster executable not found!"
    echo "Please build the project first:"
    echo "  cmake -S . -B cmake-build-debug && cmake --build cmake-build-debug"
    echo "  or: make"
    exit 1
fi
