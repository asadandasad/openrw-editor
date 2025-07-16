# OpenRW Level and Mission Editor

A comprehensive level and mission editor for the OpenRW game engine, inspired by Unreal Editor (UT2004). This editor provides a complete toolset for creating and editing GTA 3/Vice City/San Andreas maps and missions with modern visual scripting capabilities.

![OpenRW Editor](assets/editor_screenshot.png)

## Features

### 🎮 Core Functionality
- **3D Viewport**: Real-time 3D scene editing with OpenGL rendering
- **Entity System**: Component-based architecture for flexible object management
- **File Format Support**: Native support for DFF, TXD, IPL, IDE, and DAT files
- **Scene Management**: Hierarchical scene organization with layers and groups
- **Property Inspector**: Dynamic property editing for all entity components

### 🎯 Mission Design
- **Visual Scripting**: Node-based mission editor similar to Unreal Engine Blueprints
- **Trigger Zones**: Area-based triggers for mission events
- **Objective System**: Mission objective management and tracking
- **AI Scripting**: NPC behavior and pathfinding configuration
- **Cutscene Tools**: Camera control and scripted sequences

### 🛠️ Advanced Tools
- **Asset Browser**: Drag-and-drop asset management
- **World Outliner**: Scene hierarchy browser with search and filtering
- **Transformation Gizmos**: Precise object positioning, rotation, and scaling
- **Grid Snapping**: Precision placement tools
- **Layer System**: Organizational layers with visibility and lock controls

### 🎨 User Interface
- **Dockable Panels**: Customizable workspace layout
- **Dark Theme**: Professional dark UI theme
- **Multi-selection**: Advanced selection tools with marquee selection
- **Undo/Redo**: Complete action history system
- **Keyboard Shortcuts**: Efficient workflow shortcuts

## Screenshots

### Main Interface
![Main Interface](assets/main_interface_mockup.png)

### Mission Editor
![Mission Editor](assets/mission_editor_mockup.png)

### 3D Viewport
![3D Viewport](assets/3d_viewport_mockup.png)

## Installation

### Prerequisites

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install qtbase5-dev qttools5-dev libqt5opengl5-dev
sudo apt install libgl1-mesa-dev libglu1-mesa-dev
```

#### CentOS/RHEL/Fedora
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake git
sudo yum install qt5-qtbase-devel qt5-qttools-devel
sudo yum install mesa-libGL-devel mesa-libGLU-devel
```

#### Arch Linux
```bash
sudo pacman -S base-devel cmake git
sudo pacman -S qt5-base qt5-tools
sudo pacman -S mesa
```

#### Windows
- Install Visual Studio 2019 or later with C++ support
- Install Qt5 (5.12 or later) from [Qt website](https://www.qt.io/download)
- Install CMake from [CMake website](https://cmake.org/download/)

#### macOS
```bash
brew install cmake qt5
```

### Building from Source

1. **Clone the repository**
   ```bash
   git clone https://github.com/openrw/openrw-editor.git
   cd openrw-editor
   ```

2. **Build using the build script (Linux/macOS)**
   ```bash
   ./build.sh
   ```
   
   Or with options:
   ```bash
   ./build.sh --debug --clean --install
   ```

3. **Manual build**
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)
   ```

4. **Windows build**
   ```cmd
   mkdir build && cd build
   cmake .. -G "Visual Studio 16 2019"
   cmake --build . --config Release
   ```

### Running the Editor

After building, run the editor:
```bash
cd build
./openrw_editor
```

## Usage

### Getting Started

1. **Create a New Scene**
   - File → New Scene
   - Set up your project directory structure

2. **Import GTA Assets**
   - File → Import GTA Map
   - Select IPL and IDE files from your GTA installation
   - Browse imported assets in the Asset Browser

3. **Place Objects**
   - Drag assets from Asset Browser to 3D Viewport
   - Use transformation gizmos to position objects
   - Organize objects using layers in World Outliner

4. **Create Missions**
   - Open Mission Editor tab
   - Drag nodes from Node Palette to create mission logic
   - Connect nodes to define mission flow
   - Test missions using the Test Mission button

### Keyboard Shortcuts

| Action | Shortcut |
|--------|----------|
| New Scene | Ctrl+N |
| Open Scene | Ctrl+O |
| Save Scene | Ctrl+S |
| Undo | Ctrl+Z |
| Redo | Ctrl+Y |
| Delete | Delete |
| Duplicate | Ctrl+D |
| Focus Selection | F |
| Frame All | A |
| Transform Mode | G (Move), R (Rotate), S (Scale) |
| Snap to Grid | Ctrl+G |

### File Formats

#### Supported Input Formats
- **DFF**: RenderWare model files
- **TXD**: RenderWare texture dictionaries
- **IPL**: Item placement lists
- **IDE**: Item definition files
- **DAT**: Data files (paths, handling, water)

#### Project Format
- **ORWE**: OpenRW Editor project files (JSON-based)
- **ORWM**: OpenRW Mission files (JSON-based)

## Architecture

### Core Components

```
OpenRW Editor
├── Core Framework
│   ├── Entity-Component System
│   ├── Scene Manager
│   ├── File Format Parsers
│   └── Math Utilities
├── 3D Viewport
│   ├── OpenGL Renderer
│   ├── Camera Controller
│   ├── Selection System
│   └── Transformation Gizmos
├── UI System
│   ├── Property Inspector
│   ├── Asset Browser
│   ├── World Outliner
│   └── Main Window
└── Mission Editor
    ├── Visual Scripting Nodes
    ├── Mission Validation
    ├── Runtime Integration
    └── Export System
```

### Entity-Component System

The editor uses a modern ECS architecture:

- **Entities**: Unique objects in the scene
- **Components**: Data containers (Transform, Mesh, Light, Script)
- **Systems**: Logic processors (Rendering, Physics, AI)

### File Format Support

Comprehensive parsers for GTA file formats:

- **DFF Parser**: Handles RenderWare geometry and materials
- **TXD Parser**: Processes compressed textures and mipmaps
- **IPL Parser**: Reads object placement data
- **IDE Parser**: Interprets object definitions and properties
- **DAT Parser**: Handles various data files (paths, handling, water)

## Development

### Project Structure

```
openrw-editor/
├── src/
│   ├── common/          # Shared utilities and types
│   ├── file_formats/    # File format parsers
│   ├── viewport/        # 3D viewport and rendering
│   ├── ui/             # User interface components
│   ├── mission/        # Mission editor and scripting
│   └── main.cpp        # Application entry point
├── assets/             # Editor assets and resources
├── docs/              # Documentation
├── tests/             # Unit tests
├── CMakeLists.txt     # CMake build configuration
├── build.sh           # Build script
└── README.md          # This file
```

### Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Coding Standards

- **C++17** standard
- **Qt5** for UI and cross-platform support
- **OpenGL 3.3** for rendering
- **CMake** for build system
- **Google C++ Style Guide** for code formatting

### Testing

Run tests with:
```bash
cd build
ctest --verbose
```

## Performance Considerations

### System Requirements

#### Minimum Requirements
- **OS**: Windows 10, Ubuntu 18.04, macOS 10.14
- **CPU**: Intel Core i3 or AMD equivalent
- **RAM**: 4 GB
- **GPU**: OpenGL 3.3 compatible
- **Storage**: 500 MB free space

#### Recommended Requirements
- **OS**: Windows 11, Ubuntu 20.04, macOS 11.0
- **CPU**: Intel Core i5 or AMD Ryzen 5
- **RAM**: 8 GB
- **GPU**: Dedicated graphics card with 2GB VRAM
- **Storage**: 2 GB free space (for assets)

### Optimization Features

- **Lazy Loading**: Assets loaded on-demand
- **Level-of-Detail**: Automatic LOD for large scenes
- **Frustum Culling**: Only render visible objects
- **Spatial Indexing**: Efficient spatial queries
- **Memory Management**: Smart pointers and RAII

## Troubleshooting

### Common Issues

#### Build Errors

**Qt5 not found**
```
CMake Error: Could not find Qt5
```
Solution: Install Qt5 development packages or set `CMAKE_PREFIX_PATH`

**OpenGL errors**
```
OpenGL context creation failed
```
Solution: Update graphics drivers or install Mesa development packages

#### Runtime Issues

**Slow performance**
- Check graphics drivers are up to date
- Reduce viewport quality settings
- Close other graphics-intensive applications

**Crashes on startup**
- Check OpenGL version compatibility
- Verify all dependencies are installed
- Run with `--debug` flag for detailed logs

### Getting Help

- **Documentation**: [docs/](docs/)
- **Issues**: [GitHub Issues](https://github.com/openrw/openrw-editor/issues)
- **Discord**: [OpenRW Community](https://discord.gg/openrw)
- **Forum**: [OpenRW Forum](https://forum.openrw.org)

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **OpenRW Team**: For the amazing open-source GTA engine
- **RenderWare**: For the original file formats and engine architecture
- **Qt Framework**: For the excellent cross-platform UI toolkit
- **Unreal Engine**: For inspiration on editor design and visual scripting
- **GTA Modding Community**: For years of research and documentation

## Roadmap

### Version 1.0 (Current)
- ✅ Core editor functionality
- ✅ Basic file format support
- ✅ 3D viewport with basic rendering
- ✅ Property inspector and asset browser
- ✅ Mission editor with visual scripting

### Version 1.1 (Planned)
- 🔄 Advanced rendering features (shadows, lighting)
- 🔄 Terrain editing tools
- 🔄 Animation timeline editor
- 🔄 Plugin system for extensions
- 🔄 Improved performance optimization

### Version 1.2 (Future)
- 📋 Multiplayer mission support
- 📋 Advanced AI scripting
- 📋 Particle system editor
- 📋 Sound editing tools
- 📋 VR preview mode

---

**Made with ❤️ by Asad, with the help of Manus**
Thanks to Manus and Monica (also known as; Butterfly Effect AI)

