#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build

# Enter build directory
cd build

# Configure with CMake
cmake ..

# Build
cmake --build .

# Run the executable
./database_engine

# Print build status
echo "Build completed. Executable location: $(pwd)/database_engine"
