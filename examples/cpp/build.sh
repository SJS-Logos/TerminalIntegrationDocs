#!/bin/bash
# Build script for Logos Payment Service C++ Example

echo "========================================"
echo " Building Logos Payment Service"
echo "========================================"
echo ""

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found. Please install CMake:"
    echo "  - macOS: brew install cmake"
    echo "  - Ubuntu/Debian: sudo apt install cmake"
    echo "  - Fedora/RHEL: sudo yum install cmake"
    exit 1
fi

# Check if a C++ compiler is available
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "ERROR: No C++ compiler found. Please install one:"
    echo "  - macOS: xcode-select --install"
    echo "  - Ubuntu/Debian: sudo apt install build-essential"
    echo "  - Fedora/RHEL: sudo yum groupinstall 'Development Tools'"
    exit 1
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Navigate to build directory
cd build

# Generate build files
echo ""
echo "Generating build files..."
cmake .. -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    cd ..
    exit 1
fi

# Build the project
echo ""
echo "Building project..."
cmake --build . --config Release
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    cd ..
    exit 1
fi

echo ""
echo "========================================"
echo " Build completed successfully!"
echo "========================================"
echo ""
echo "Executable location: build/payment_cli"
echo ""
echo "Try running:"
echo "  cd build"
echo "  ./payment_cli help"
echo ""

cd ..
