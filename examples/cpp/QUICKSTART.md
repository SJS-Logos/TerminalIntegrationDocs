# C++ Payment Service - Quick Start

## Build Instructions

### Windows (Visual Studio)

1. Open PowerShell and navigate to the C++ example directory:
```powershell
cd examples/cpp
```

2. Create and enter the build directory:
```powershell
mkdir build
cd build
```

3. Generate Visual Studio project files:
```powershell
cmake ..
```

4. Build the project:
```powershell
cmake --build . --config Release
```

5. The executable will be created at: `Release\payment_cli.exe`

### Linux/macOS

1. Navigate to the C++ example directory:
```bash
cd examples/cpp
```

2. Create and enter the build directory:
```bash
mkdir build
cd build
```

3. Generate Makefiles:
```bash
cmake ..
```

4. Build the project:
```bash
make
```

5. The executable will be created at: `payment_cli`

## Running the Application

### Windows
```powershell
cd build
.\Release\payment_cli.exe help
```

### Linux/macOS
```bash
cd build
./payment_cli help
```

## Example Commands

### 1. Show Help
```bash
payment_cli help
```

### 2. Authorize a Valid Payment
```bash
payment_cli authorize --amount 100.00 --currency USD --merchant MERCH-001
```

**Expected Output:**
```
=== Payment Authorization Result ===
Payment ID:     PAY-000001
Authorized:     YES
Status:         Authorized
Amount:         100.00 USD
====================================
```

### 3. Test Precise Decimal Handling (No Floating-Point Errors)
```bash
payment_cli authorize --amount 0.10 --currency USD --merchant MERCH-001
```

**Note**: The Money class uses integer cents internally to avoid floating-point precision issues. This means `$0.10` is stored as exactly `10 cents`, preventing errors like `0.1 + 0.2 ? 0.3` that occur with floating-point arithmetic.

### 4. Test Fraud Detection (Amount > $5000)
```bash
payment_cli authorize --amount 6000.00 --currency USD --merchant MERCH-001
```

**Expected Output:**
```
=== Payment Authorization Result ===
Payment ID:     PAY-000002
Authorized:     NO
Status:         Declined
Amount:         6000.00 USD
Decline Reason: Suspected fraud
====================================
```

### 5. Test Negative Amount Validation
```bash
payment_cli authorize --amount -50.00 --currency USD --merchant MERCH-001
```

**Expected Output:**
```
=== Payment Authorization Result ===
Payment ID:     PAY-000003
Authorized:     NO
Status:         Declined
Amount:         -50.00 USD
Decline Reason: Amount must be positive
====================================
```

### 6. Get Command Help
```bash
payment_cli authorize --help
```

## Business Rules Tested

The application demonstrates several business rules:

1. **Positive Amount Rule**: Payments must have a positive amount
2. **Fraud Detection**: Amounts over $5,000.00 are flagged as suspicious
3. **Maximum Limit**: Payments cannot exceed $10,000.00 (checked after fraud detection)
4. **Currency Validation**: Money operations validate matching currencies
5. **Precise Calculations**: Uses integer cents internally to avoid floating-point errors

## Architecture Overview

The compiled executable demonstrates:

- **Core Unit** (`mypaymentservice_core.lib`): Domain, SharedKernel, Capabilities, and Application - pure business logic with no Host dependencies
- **Infrastructure Unit** (`mypaymentservice_infrastructure.lib`): Capability implementations (in-memory storage, simple fraud detection)
- **CLI Host Unit** (`payment_cli.exe`): Command-line interface entry point

## Troubleshooting

### CMake Not Found
Install CMake from https://cmake.org/download/ or via package manager:
- Windows: `choco install cmake` or download installer
- macOS: `brew install cmake`
- Linux: `sudo apt install cmake` or `sudo yum install cmake`

### Compiler Not Found
Ensure you have a C++17 compatible compiler:
- Windows: Install Visual Studio 2019 or later with C++ workload
- macOS: Install Xcode Command Line Tools: `xcode-select --install`
- Linux: Install g++: `sudo apt install build-essential`

### Build Errors
If you encounter build errors:
1. Clean the build directory: `rm -rf build` (or `Remove-Item -Recurse build` on Windows)
2. Regenerate and rebuild: 
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

## Next Steps

- Read the [full README](README.md) for architecture details
- Explore the source code in the various library directories
- Review the [AP-002 Implementation Guide](../../docs/examples/AP-002-Implementation-Cpp.md)
- Check out the [CLI Application Example](../../docs/examples/AP-003-CLI-Cpp.md)
