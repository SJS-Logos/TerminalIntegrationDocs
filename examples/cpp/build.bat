@echo off
REM Build script for Logos Payment Service C++ Example

echo ========================================
echo  Building Logos Payment Service
echo ========================================
echo.

REM Check if CMake is available
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found. Please install CMake from https://cmake.org/download/
    exit /b 1
)

REM Create build directory if it doesn't exist
if not exist build (
    echo Creating build directory...
    mkdir build
)

REM Navigate to build directory
cd build

REM Generate build files
echo.
echo Generating build files...
cmake .. -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed
    cd ..
    exit /b 1
)

REM Build the project
echo.
echo Building project...
cmake --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    cd ..
    exit /b 1
)

echo.
echo ========================================
echo  Build completed successfully!
echo ========================================
echo.
echo Executable location: build\Release\payment_cli.exe
echo.
echo Try running:
echo   cd build
echo   .\Release\payment_cli.exe help
echo.

cd ..
