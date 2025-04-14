# Ad-Hoc Network Routing Simulation Project

This repository contains our implementation and comparison of DSDV, DSR, and GPSR routing protocols for ad-hoc networks using NS-3. This README explains how to set up the project and work with NS-3 on any operating system.

## 🚀 Quick Start

### Step 1: Clone the Repository

```bash
# Clone the main repo
git clone https://github.com/your-username/adhoc-routing-comparison.git
cd adhoc-routing-comparison

# Initialize and download the NS-3 submodule
git submodule update --init --recursive
```

### Step 2: Build NS-3 (Only Needed Once)

This is OS-specific - see instructions for your system below.

### Step 3: Build & Run Our Simulation

```bash
# After NS-3 is built
cd build
./adhoc-routing-comparison
```

## 📋 Detailed Setup Instructions

### Linux

```bash
# Install prerequisites
sudo apt update
sudo apt install build-essential gcc g++ python3 python3-dev pkg-config sqlite3 cmake

# Build NS-3
cd external/ns-3
./ns3 configure --enable-examples --enable-tests
./ns3 build

# Build our project
cd ../..
mkdir -p build && cd build
cmake ..
make
```

### macOS

```bash
# Install prerequisites with Homebrew
brew install gcc cmake python3 boost pkg-config

# Build NS-3
cd external/ns-3
./ns3 configure --enable-examples --enable-tests
./ns3 build

# Build our project
cd ../..
mkdir -p build && cd build
cmake ..
make
```

### Windows

#### Option 1: Windows Subsystem for Linux (Recommended)

1. Install WSL2 with Ubuntu from Microsoft Store
2. Open Ubuntu terminal and follow the Linux instructions above

#### Option 2: Native Windows

```powershell
# Install prerequisites
# - Visual Studio with C++ development tools
# - Python 3
# - CMake

# Build NS-3 (from Command Prompt with Admin rights)
cd external\ns-3
python ns3 configure --enable-examples --enable-tests
python ns3 build

# Build our project
cd ..\..
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## 🛠️ Common Issues & Solutions

### "NS-3 build fails"
- Make sure you have all prerequisites installed for your platform
- Try building with fewer modules: `./ns3 configure --enable-modules=core,network,internet,mobility`

### "Git submodule update fails"
- Check your network connection
- If behind a firewall, configure git to use HTTPS: `git config --global url."https://".insteadOf git://`

### "CMake can't find NS-3"
- Make sure you built NS-3 first
- Try running `cmake -DRT_LIBRARY=/path/to/librt.so ..` on Linux with the correct path

## 🏗️ Project Structure

```
/
├── CMakeLists.txt           # Main build configuration
├── external/
│   └── ns-3/                # NS-3 submodule
├── include/                 # Header files
│   ├── DSDV.hpp
│   ├── DSR.hpp
│   └── GPSR.hpp
└── src/                     # Implementation files
    ├── main.cpp             # Simulation entry point
    ├── DSDV.cpp
    ├── DSR.cpp
    └── GPSR.cpp
```

## 🧠 How to Add Your Contributions

1. Create a branch: `git checkout -b feature/your-feature-name`
2. Make changes to the code
3. Test locally: `cd build && make && ./adhoc-routing-comparison`
4. Commit your changes: `git commit -m "Add feature X"`
5. Push to your branch: `git push origin feature/your-feature-name`
6. Create a Pull Request

## 🔄 Updating NS-3

If we need to update the NS-3 submodule:

```bash
cd external/ns-3
git checkout master
git pull
cd ../..
git add external/ns-3
git commit -m "Update NS-3 submodule"
```

Then everyone else needs to run:

```bash
git pull
git submodule update --recursive
```

## 📊 Running Simulations

```bash
# Basic simulation
./adhoc-routing-comparison

# With custom parameters
./adhoc-routing-comparison --numNodes=20 --protocol=dsdv --mobility=randomWaypoint

# See all available options
./adhoc-routing-comparison --help
```

## 📚 Documentation Resources

- [NS-3 Documentation](https://www.nsnam.org/documentation/)
- [NS-3 Tutorial](https://www.nsnam.org/docs/tutorial/html/)
- [Ad-Hoc Network Basics](https://www.researchgate.net/publication/221454408_Wireless_Ad-Hoc_Networks_An_Overview)

## 🤝 Need Help?

If you have any issues:
1. Check the NS-3 documentation
2. Look through this README again
3. Ask in our team chat
4. Contact the project lead

Remember to pull the latest changes before starting work each day!