#!/bin/bash

# OpenRW Level Editor Build Script
# This script builds the OpenRW Level Editor using CMake

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE="Release"
BUILD_DIR="build"
INSTALL_DIR="install"
PARALLEL_JOBS=$(nproc)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --install)
            INSTALL=true
            shift
            ;;
        --jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug     Build in Debug mode (default: Release)"
            echo "  --clean     Clean build directory before building"
            echo "  --install   Install after building"
            echo "  --jobs N    Use N parallel jobs (default: $(nproc))"
            echo "  --help      Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo -e "${BLUE}OpenRW Level Editor Build Script${NC}"
echo -e "${BLUE}================================${NC}"
echo ""

# Check dependencies
echo -e "${YELLOW}Checking dependencies...${NC}"

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake is not installed${NC}"
    exit 1
fi

# Check for Qt5
if ! pkg-config --exists Qt5Core Qt5Widgets Qt5OpenGL; then
    echo -e "${RED}Error: Qt5 development packages are not installed${NC}"
    echo "Please install Qt5 development packages:"
    echo "  Ubuntu/Debian: sudo apt install qtbase5-dev qttools5-dev libqt5opengl5-dev"
    echo "  CentOS/RHEL: sudo yum install qt5-qtbase-devel qt5-qttools-devel"
    echo "  Arch: sudo pacman -S qt5-base qt5-tools"
    exit 1
fi

# Check for OpenGL
if ! pkg-config --exists gl; then
    echo -e "${YELLOW}Warning: OpenGL development packages may not be installed${NC}"
    echo "If build fails, install OpenGL development packages:"
    echo "  Ubuntu/Debian: sudo apt install libgl1-mesa-dev"
    echo "  CentOS/RHEL: sudo yum install mesa-libGL-devel"
fi

echo -e "${GREEN}Dependencies check passed${NC}"
echo ""

# Clean build directory if requested
if [[ "$CLEAN_BUILD" == true ]]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
    rm -rf "$INSTALL_DIR"
fi

# Create build directory
echo -e "${YELLOW}Creating build directory...${NC}"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="../$INSTALL_DIR" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DQT_VERSION=5

if [[ $? -ne 0 ]]; then
    echo -e "${RED}CMake configuration failed${NC}"
    exit 1
fi

echo -e "${GREEN}CMake configuration successful${NC}"
echo ""

# Build
echo -e "${YELLOW}Building OpenRW Level Editor ($BUILD_TYPE mode, $PARALLEL_JOBS jobs)...${NC}"
make -j"$PARALLEL_JOBS"

if [[ $? -ne 0 ]]; then
    echo -e "${RED}Build failed${NC}"
    exit 1
fi

echo -e "${GREEN}Build successful${NC}"
echo ""

# Install if requested
if [[ "$INSTALL" == true ]]; then
    echo -e "${YELLOW}Installing...${NC}"
    make install
    
    if [[ $? -ne 0 ]]; then
        echo -e "${RED}Installation failed${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}Installation successful${NC}"
    echo -e "${BLUE}Installed to: $(pwd)/../$INSTALL_DIR${NC}"
    echo ""
fi

# Show build information
echo -e "${BLUE}Build Information:${NC}"
echo -e "  Build Type: $BUILD_TYPE"
echo -e "  Build Directory: $(pwd)"
echo -e "  Executable: $(pwd)/openrw_editor"

if [[ "$INSTALL" == true ]]; then
    echo -e "  Install Directory: $(pwd)/../$INSTALL_DIR"
fi

echo ""
echo -e "${GREEN}Build completed successfully!${NC}"

# Show how to run
echo -e "${BLUE}To run the editor:${NC}"
if [[ "$INSTALL" == true ]]; then
    echo -e "  cd ../$(pwd)/../$INSTALL_DIR/bin && ./openrw_editor"
else
    echo -e "  cd $(pwd) && ./openrw_editor"
fi

echo ""
echo -e "${BLUE}For development:${NC}"
echo -e "  - Compile commands: $(pwd)/compile_commands.json"
echo -e "  - Debug with GDB: gdb ./openrw_editor"
echo -e "  - Profile with Valgrind: valgrind --tool=memcheck ./openrw_editor"

