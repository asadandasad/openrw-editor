
# OpenRW Level and Mission Editor: Architecture and Design

## 1. Introduction

This document outlines the proposed architecture for a comprehensive, user-friendly, and low-end PC-friendly Level and Mission Editor for the OpenRW game engine. The editor will be visually and functionally inspired by the Unreal Editor (specifically, the version from Unreal Tournament 2004), providing a familiar and powerful toolset for creators. The primary goal is to create a tool that is both powerful enough for experienced developers and accessible to hobbyists and new modders.

This document will cover the overall architecture, a detailed module breakdown, UI/UX suggestions, and recommendations for the initial code scaffold and UI framework.



## 2. Overall Architecture and Module Breakdown

The OpenRW Level and Mission Editor will follow a modular, layered architecture to ensure maintainability, scalability, and flexibility. This approach allows for independent development and testing of components, and facilitates future extensions through plugins and scripting. The core components will be designed to be cross-platform, targeting Windows, Linux, and macOS.

### 2.1. High-Level Architecture

At a high level, the editor can be divided into the following major subsystems:

*   **Core Engine Integration Layer:** Handles communication with the OpenRW engine, including loading/saving game assets and integrating with the runtime for testing missions.
*   **Asset Management System:** Manages all game assets (models, textures, sounds, scripts) used within the editor, providing a centralized repository and tools for importing/exporting.
*   **Scene Management System:** Responsible for the 3D world representation, including entity placement, terrain data, lighting, and camera control.
*   **User Interface (UI) Framework:** Provides the visual components and interaction mechanisms for the editor, including the 3D viewport, property inspector, and various tool panels.
*   **Mission Editor Subsystem:** Dedicated module for designing and scripting missions, including visual scripting, trigger management, and objective definition.
*   **Scripting and Plugin System:** Enables extensibility through embedded scripting languages and a plugin API for custom tools and entities.

### 2.2. Detailed Module Breakdown

#### 2.2.1. Core Engine Integration Layer

This layer acts as the bridge between the editor and the OpenRW game engine. It will be responsible for:

*   **File Format Parsers:** Modules to load and parse GTA 3/VC/SA specific file formats (`.dff`, `.txd`, `.ide`, `.ipl`, `.dat`). These parsers will convert raw game data into an editor-friendly data model.
*   **OpenRW Level File I/O:** Functionality to import and export OpenRW-compatible level files. This will likely involve a custom serialization format that can be easily consumed by the OpenRW engine.
*   **Runtime Integration:** A mechanism to launch the OpenRW engine with the currently edited level and mission for in-editor testing. This could involve a shared memory interface, a network socket, or command-line arguments.

#### 2.2.2. Asset Management System

This system will provide a robust way to manage all assets within the editor.

*   **Asset Database:** A centralized database (could be file-based or in-memory for simplicity) to track all loaded assets, their types, and their paths.
*   **Asset Importer/Exporter:** Tools for importing new assets into the editor and exporting modified assets back into game-compatible formats.
*   **Asset Browser:** A UI component to browse, search, and preview available assets (NPCs, vehicles, props, lights, etc.).

#### 2.2.3. Scene Management System

This is the heart of the 3D editing experience.

*   **3D Scene Graph:** A hierarchical data structure representing all objects and their relationships within the 3D world. This will include entities, terrain, lights, and cameras.
*   **Entity System:** A component-based system where each object in the scene (NPC, vehicle, prop) is an entity composed of various components (e.g., MeshComponent, TransformComponent, ScriptComponent, PropertyComponent). This allows for flexible and extensible object definitions.
*   **Terrain Editor:** Tools for real-time terrain manipulation, including sculpting, painting textures, and adjusting heightmaps.
*   **Object Placement and Manipulation:** Functionality for drag-and-drop placement of objects, translation, rotation, scaling, and snapping.
*   **Pathfinding Node Editor:** Tools for defining and editing navigation meshes or pathfinding nodes for AI.
*   **Lighting System:** Management of scene lighting, including ambient, directional, point, and spot lights.
*   **Camera Control:** Navigation within the 3D viewport (pan, orbit, zoom).

#### 2.2.4. User Interface (UI) Framework

This layer will be responsible for rendering the editor's visual elements and handling user input.

*   **Main Window:** The primary application window containing all other UI elements.
*   **3D Viewport:** The central component for visualizing and interacting with the 3D scene. This will require a robust 3D rendering engine (e.g., OpenGL, DirectX, Vulkan, or a higher-level engine like Godot).
*   **Property Inspector:** A dynamic UI panel that displays and allows editing of properties for the currently selected entity or component. This will leverage the component-based entity system.
*   **Toolbars and Menus:** Standard UI elements for common actions and settings.
*   **Asset Browser Panel:** (As described in Asset Management System).
*   **Layer/Zone Panel:** A UI panel to manage different layers or zones within the level, enabling selective visibility and editing.
*   **Console/Log Window:** For displaying editor messages, errors, and debugging information.

#### 2.2.5. Mission Editor Subsystem

This dedicated module will provide tools for mission design.

*   **Mission Data Model:** A data structure to represent mission logic, including triggers, objectives, checkpoints, and NPC behaviors.
*   **Visual Scripting/Flowchart Editor:** A graphical interface for defining mission logic using nodes and connections (e.g., a state machine or behavior tree editor).
*   **Trigger System:** Tools for defining trigger zones (e.g., sphere, box), conditions (e.g., player enters zone, timer expires), and actions.
*   **Objective/Checkpoint Manager:** UI for defining and tracking mission objectives and checkpoints.
*   **NPC Scripting Interface:** Specific tools for defining NPC patrols, conversations, attack conditions, and other AI behaviors, potentially integrated with the visual scripting system.

#### 2.2.6. Scripting and Plugin System

For extensibility and advanced customization.

*   **Embedded Scripting Engine:** Integration of a scripting language (e.g., Lua or AngelScript) for entity scripting and custom tool development. This allows users to extend editor functionality without recompiling the core.
*   **Plugin API:** A well-defined API that allows developers to create custom plugins for new entity types, tools, or import/export formats.

This modular design ensures that different parts of the editor can be developed in parallel and maintained independently, leading to a more robust and flexible application. The choice of programming language and UI framework will significantly impact the implementation details of these modules, which will be discussed in subsequent sections.



## 3. UI Mockup Suggestions

Inspired by Unreal Editor (UT2004), the OpenRW Level and Mission Editor will feature a multi-panel interface designed for efficiency and ease of use. The layout prioritizes the 3D viewport while providing quick access to essential tools and information.

### 3.1. Main Window Layout

The main window will be organized into several key areas:

*   **Menu Bar (Top):** Standard file operations (New, Open, Save, Save As), Edit (Undo, Redo, Cut, Copy, Paste), View (Toggle panels, Wireframe/Solid/Texture view), Tools (Terrain Editor, Pathfinding Editor, Mission Editor), Build (Test Mission), Help.
*   **Toolbar (Below Menu Bar):** Quick access icons for frequently used actions like selection tools, transformation tools (move, rotate, scale), object placement, and the 'Test Mission' button.
*   **3D Viewport (Center):** The largest and most prominent area, displaying the 3D scene. This is where users will directly interact with the level, place objects, and navigate the environment. It will support various camera modes (perspective, orthographic) and rendering modes (wireframe, solid, textured, lit).
*   **Asset Browser (Left Panel, Collapsible):** A dockable panel on the left side, allowing users to browse and search for game assets (NPCs, vehicles, props, lights, prefabs). Assets can be dragged directly from this panel into the 3D viewport.
*   **Property Inspector (Right Panel, Collapsible):** A dockable panel on the right side. When an object or component is selected in the 3D viewport, its properties will be displayed here, allowing for real-time editing. The inspector will dynamically update based on the selected item's components.
*   **World Outliner/Scene Hierarchy (Top-Right, above Property Inspector):** A tree-view displaying all entities in the current scene, organized hierarchically. This allows for easy selection, grouping, and reordering of objects.
*   **Output Log/Console (Bottom Panel, Collapsible):** A dockable panel at the bottom, displaying editor messages, warnings, errors, and debugging information. This can also serve as a command console for advanced users.
*   **Layer/Zone Manager (Bottom-Left, above Asset Browser):** A small panel or tab within the Asset Browser area to manage different layers or zones within the level, allowing users to toggle visibility, lock layers, and organize mission-specific areas.

### 3.2. Panel Details and Interaction

#### 3.2.1. 3D Viewport

*   **Navigation:** Standard WASD for movement, mouse for looking around. Hotkeys for quick camera resets or snapping to specific views (top, front, side).
*   **Selection:** Single-click to select objects, Ctrl+Click for multi-selection, marquee selection.
*   **Manipulation Gizmos:** When an object is selected, transformation gizmos (move arrows, rotation circles, scale cubes) will appear, allowing for intuitive manipulation directly in the 3D space.
*   **Context Menus:** Right-clicking in the viewport will bring up context-sensitive menus for actions like 


duplicating, deleting, grouping, or adding components.

#### 3.2.2. Property Inspector

*   **Dynamic Fields:** The fields displayed will change based on the type of object or component selected. For example, a `MeshComponent` might show properties for mesh file path, material, and visibility, while a `LightComponent` would show light type, color, intensity, and radius.
*   **Component-Based:** Properties will be grouped by component, making it easy to understand and modify specific aspects of an entity.
*   **Data Types:** Support for various data types including text fields, numerical inputs (with sliders for common ranges), dropdowns for enums, color pickers, and file path selectors.
*   **Drag-and-Drop:** Ability to drag assets from the Asset Browser directly into property fields (e.g., assigning a texture to a material property).

#### 3.2.3. Mission Editor UI

*   **Flowchart/Visual Scripting Canvas:** A dedicated tab or window within the editor for the mission editor. This will feature a node-based visual scripting interface where users can drag and drop nodes representing triggers, conditions, actions, and objectives. Connections between nodes will define the mission flow.
*   **Node Palette:** A panel containing various pre-defined nodes for mission logic (e.g., 'Player Enters Zone', 'Timer Starts', 'Spawn NPC', 'Display Message', 'Objective Complete').
*   **Trigger Zone Editor:** Integrated within the 3D viewport, allowing users to visually define trigger volumes (boxes, spheres, capsules) and link them to mission nodes.
*   **Objective/Checkpoint List:** A panel to manage mission objectives and checkpoints, allowing users to define their text, status, and associated triggers.
*   **NPC Behavior Editor:** A sub-panel or specialized nodes within the visual scripting system to define NPC patrols (waypoints), conversation trees, and combat behaviors.

### 3.3. UI Framework Recommendations

Considering the requirements for a cross-platform, visually rich, and low-end PC friendly editor, several UI frameworks are suitable:

*   **Qt (C++):** A mature, powerful, and widely used cross-platform framework. It offers excellent performance, extensive UI widgets, and a robust signal-slot mechanism for event handling. Qt's QML can be used for more modern, declarative UI design. It's well-suited for complex applications like game editors and can be optimized for performance.
*   **Godot Editor (Godot Engine):** While Godot is primarily a game engine, its editor is built using its own UI system, which is highly customizable and performant. If the editor itself were built *within* Godot, it would leverage the engine's 3D capabilities directly, simplifying integration with OpenRW's runtime (assuming OpenRW could be integrated as a Godot module or plugin). This could offer the best 


integration for a "Test Mission" button. However, it might introduce a dependency on the Godot engine itself.
*   **ImGui (C++):** A popular immediate-mode GUI library. It's excellent for debugging tools, in-game editors, and performance-critical applications due to its low overhead. However, building a full-featured, complex editor UI with ImGui from scratch can be more labor-intensive than with a retained-mode GUI like Qt, as it requires more manual layout and state management. It's highly performant and suitable for low-end PCs.
*   **Electron + Three.js (JavaScript/Web Technologies):** Electron allows building desktop applications using web technologies (HTML, CSS, JavaScript). Three.js is a powerful 3D library for the web. This combination offers rapid UI development, easy extensibility with web standards, and a large developer community. However, Electron apps can be resource-intensive compared to native applications, which might be a concern for "low-end PC friendly" requirement. The 3D viewport would be rendered in a web canvas.

**Recommendation:**

For a full-featured, cross-platform, and performant editor that is also low-end PC friendly, **Qt (C++)** stands out as the most balanced choice. It provides robust UI capabilities, excellent performance, and a mature ecosystem. While Godot Editor offers deep integration if the editor is built within Godot, Qt allows for a standalone application that can integrate with OpenRW via a well-defined API. ImGui is a strong contender for performance but requires more development effort for complex UIs. Electron is great for rapid development but might struggle with the "low-end PC" requirement.

Therefore, the initial code scaffold will primarily focus on a C++ foundation, with Qt as the recommended UI framework.




## 4. Initial Code Scaffold and Example

This section provides an initial code scaffold for a C++ application using the Qt framework, demonstrating the basic structure of the editor. It will include a conceptual representation of loading GTA map files and placing objects.

### 4.1. Project Structure

The project will be structured as follows:

```
openrw_editor/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── editor_window.h
│   ├── editor_window.cpp
│   ├── gta_loader.h
│   ├── gta_loader.cpp
│   └── asset_manager.h
│   └── asset_manager.cpp
├── assets/
│   └── (placeholder for GTA assets)
└── build/
    └── (build output)
```

### 4.2. `CMakeLists.txt` (Build Configuration)

This file configures the build process using CMake, which is a cross-platform build system generator. It will find Qt libraries and compile the source code.

```cmake
cmake_minimum_required(VERSION 3.14)

project(OpenRWEditor VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Qt
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Add source files
set(SOURCE_FILES
    src/main.cpp
    src/editor_window.cpp
    src/gta_loader.cpp
    src/asset_manager.cpp
)

# Add header files
set(HEADER_FILES
    src/editor_window.h
    src/gta_loader.h
    src/asset_manager.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Optional: Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
```

### 4.3. `src/main.cpp` (Application Entry Point)

This is the main entry point of the application, responsible for initializing the Qt application and creating the main editor window.

```cpp
#include <QApplication>
#include "editor_window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    EditorWindow editorWindow;
    editorWindow.showMaximized(); // Or show() for a default size

    return app.exec();
}
```

### 4.4. `src/editor_window.h` (Main Editor Window Header)

Defines the main window class, inheriting from `QMainWindow`, and declares its UI components.

```cpp
#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QDockWidget>
#include <QListWidget>
#include <QTreeWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class EditorWindow; }
QT_END_NAMESPACE

class EditorWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit EditorWindow(QWidget *parent = nullptr);
    ~EditorWindow();

private slots:
    void openFile();
    void saveFile();
    void testMission();

private:
    Ui::EditorWindow *ui;

    // Placeholder for 3D Viewport (will be a custom QOpenGLWidget or similar)
    QWidget *viewportWidget;

    // UI Components
    QDockWidget *assetBrowserDock;
    QListWidget *assetListWidget;

    QDockWidget *propertyInspectorDock;
    QTextEdit *propertyEditor;

    QDockWidget *worldOutlinerDock;
    QTreeWidget *worldOutlinerTree;

    QTextEdit *logConsole;

    void createMenus();
    void createToolbars();
    void createDocks();
    void setupCentralWidget();
};

#endif // EDITOR_WINDOW_H
```

### 4.5. `src/editor_window.cpp` (Main Editor Window Implementation)

Implements the main window's constructor, setting up the basic UI layout with menu bars, toolbars, and dockable widgets for the asset browser, property inspector, and world outliner.

```cpp
#include "editor_window.h"
#include "gta_loader.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDebug>

// Forward declaration for the UI class generated by Qt Designer (if used)
// For this scaffold, we'll manually create UI elements.
namespace Ui { class EditorWindow {}; }

EditorWindow::EditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditorWindow)
{
    // If using Qt Designer, uncomment and use ui->setupUi(this);
    // For manual setup, we'll create widgets directly.

    setWindowTitle("OpenRW Level and Mission Editor");
    setMinimumSize(1024, 768);

    createMenus();
    createToolbars();
    createDocks();
    setupCentralWidget();

    statusBar()->showMessage("Ready");
}

EditorWindow::~EditorWindow()
{
    delete ui;
}

void EditorWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *openAction = fileMenu->addAction(tr("&Open..."), this, &EditorWindow::openFile);
    openAction->setShortcut(QKeySequence::Open);
    QAction *saveAction = fileMenu->addAction(tr("&Save"), this, &EditorWindow::saveFile);
    saveAction->setShortcut(QKeySequence::Save);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &QWidget::close);

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(tr("Test Mission"), this, &EditorWindow::testMission);

    // Add more menus as needed (Edit, View, etc.)
}

void EditorWindow::createToolbars()
{
    QToolBar *fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(QIcon::fromTheme("document-open"), tr("Open"), this, &EditorWindow::openFile);
    fileToolBar->addAction(QIcon::fromTheme("document-save"), tr("Save"), this, &EditorWindow::saveFile);

    QToolBar *editorToolBar = addToolBar(tr("Editor"));
    editorToolBar->addAction(QIcon::fromTheme("media-playback-start"), tr("Test Mission"), this, &EditorWindow::testMission);
    // Add more actions for selection, transform tools, etc.
}

void EditorWindow::createDocks()
{
    // Asset Browser Dock
    assetBrowserDock = new QDockWidget(tr("Asset Browser"), this);
    assetListWidget = new QListWidget(this);
    assetBrowserDock->setWidget(assetListWidget);
    addDockWidget(Qt::LeftDockWidgetArea, assetBrowserDock);

    // Property Inspector Dock
    propertyInspectorDock = new QDockWidget(tr("Property Inspector"), this);
    propertyEditor = new QTextEdit(this);
    propertyEditor->setReadOnly(true); // Will be dynamic later
    propertyInspectorDock->setWidget(propertyEditor);
    addDockWidget(Qt::RightDockWidgetArea, propertyInspectorDock);

    // World Outliner Dock
    worldOutlinerDock = new QDockWidget(tr("World Outliner"), this);
    worldOutlinerTree = new QTreeWidget(this);
    worldOutlinerTree->setHeaderLabels({"Object Name", "Type"});
    worldOutlinerDock->setWidget(worldOutlinerTree);
    addDockWidget(Qt::RightDockWidgetArea, worldOutlinerDock);

    // Log Console Dock
    logConsole = new QTextEdit(this);
    logConsole->setReadOnly(true);
    QDockWidget *logDock = new QDockWidget(tr("Log"), this);
    logDock->setWidget(logConsole);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);

    // Example: Add some dummy assets and objects
    assetListWidget->addItem("NPC_Pedestrian");
    assetListWidget->addItem("Vehicle_Car");
    assetListWidget->addItem("Prop_Bench");
    assetListWidget->addItem("Light_Point");

    QTreeWidgetItem *item1 = new QTreeWidgetItem(worldOutlinerTree, {"SpawnPoint_1", "SpawnPoint"});
    QTreeWidgetItem *item2 = new QTreeWidgetItem(worldOutlinerTree, {"Car_Taxi_01", "Vehicle"});
    QTreeWidgetItem *item3 = new QTreeWidgetItem(worldOutlinerTree, {"Light_Street_01", "Light"});

    // Connect selection change to property inspector update
    connect(worldOutlinerTree, &QTreeWidget::itemClicked, [this](QTreeWidgetItem *item, int column){
        propertyEditor->setText(QString("Selected: %1\nType: %2\n\n(Properties will be displayed here)").arg(item->text(0)).arg(item->text(1)));
    });
}

void EditorWindow::setupCentralWidget()
{
    // Placeholder for the 3D Viewport
    viewportWidget = new QWidget(this);
    viewportWidget->setStyleSheet("background-color: #333; border: 1px solid #555;");
    // In a real implementation, this would be a QOpenGLWidget or a custom widget
    // that integrates with a 3D rendering engine (e.g., a custom OpenGL context).
    // For now, it's just a dark grey placeholder.

    setCentralWidget(viewportWidget);
}

void EditorWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open GTA Map File"),
                                                    QString(),
                                                    tr("GTA Map Files (*.ipl *.ide *.dat);;All Files (*)"));
    if (!fileName.isEmpty()) {
        logConsole->append(QString("Attempting to load: %1").arg(fileName));
        GTALoader::loadMapFile(fileName); // Conceptual call
        QMessageBox::information(this, tr("File Loaded"),
                                 QString("Successfully initiated loading of %1. (Actual loading logic not implemented in scaffold)").arg(fileName));
    }
}

void EditorWindow::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save OpenRW Level File"),
                                                   QString(),
                                                   tr("OpenRW Level Files (*.orw);;All Files (*)"));
    if (!fileName.isEmpty()) {
        logConsole->append(QString("Attempting to save to: %1").arg(fileName));
        // Save logic here (conceptual)
        QMessageBox::information(this, tr("File Saved"),
                                 QString("Successfully initiated saving to %1. (Actual saving logic not implemented in scaffold)").arg(fileName));
    }
}

void EditorWindow::testMission()
{
    logConsole->append("""Launching OpenRW engine for mission test...
(This would involve passing current level data to OpenRW runtime)""");
    QMessageBox::information(this, tr("Test Mission"),
                             tr("Simulating OpenRW engine launch. (Integration logic not implemented in scaffold)"));
}
```

### 4.6. `src/gta_loader.h` (GTA File Loader Header - Conceptual)

Defines a conceptual class for loading GTA-specific files. The actual implementation would involve complex parsing logic.

```cpp
#ifndef GTA_LOADER_H
#define GTA_LOADER_H

#include <QString>
#include <QVector>
#include <QDebug>

// Forward declarations for conceptual data structures
struct GTAObjectPlacement;
struct GTATerrainData;

class GTALoader {
public:
    static bool loadMapFile(const QString &filePath);
    static bool loadDFF(const QString &filePath); // .dff (model)
    static bool loadTXD(const QString &filePath); // .txd (texture dictionary)
    static bool loadIDE(const QString &filePath); // .ide (item definitions)
    static bool loadIPL(const QString &filePath); // .ipl (item placements)
    static bool loadDAT(const QString &filePath); // .dat (data files, e.g., path nodes)

private:
    // Conceptual internal data structures
    static QVector<GTAObjectPlacement> s_objectPlacements;
    static GTATerrainData s_terrainData;
};

#endif // GTA_LOADER_H
```

### 4.7. `src/gta_loader.cpp` (GTA File Loader Implementation - Conceptual)

Provides a conceptual implementation for the GTA file loader. The actual parsing would be significantly more involved.

```cpp
#include "gta_loader.h"

// Conceptual data structures (simplified for scaffold)
struct GTAObjectPlacement {
    QString modelName;
    float posX, posY, posZ;
    float rotX, rotY, rotZ, rotW;
    // Add other properties as needed
};

struct GTATerrainData {
    // Placeholder for terrain vertices, indices, textures
};

QVector<GTAObjectPlacement> GTALoader::s_objectPlacements;
GTATerrainData GTALoader::s_terrainData;

bool GTALoader::loadMapFile(const QString &filePath) {
    qDebug() << "GTALoader: Attempting to load map file:" << filePath;

    // In a real implementation, this would involve:
    // 1. Determining file type (.ipl, .ide, .dat)
    // 2. Calling appropriate parser (loadIPL, loadIDE, loadDAT)
    // 3. Populating internal scene data structures

    if (filePath.endsWith(".ipl", Qt::CaseInsensitive)) {
        qDebug() << "GTALoader: Parsing IPL file...";
        // Example: Add a dummy object for demonstration
        GTAObjectPlacement dummyObject;
        dummyObject.modelName = "dummy_prop_01";
        dummyObject.posX = 10.0f;
        dummyObject.posY = 20.0f;
        dummyObject.posZ = 5.0f;
        dummyObject.rotX = 0.0f; dummyObject.rotY = 0.0f; dummyObject.rotZ = 0.0f; dummyObject.rotW = 1.0f;
        s_objectPlacements.append(dummyObject);
        qDebug() << "GTALoader: Added conceptual object:" << dummyObject.modelName;
        return true;
    } else if (filePath.endsWith(".ide", Qt::CaseInsensitive)) {
        qDebug() << "GTALoader: Parsing IDE file...";
        return true;
    } else if (filePath.endsWith(".dat", Qt::CaseInsensitive)) {
        qDebug() << "GTALoader: Parsing DAT file...";
        return true;
    } else {
        qDebug() << "GTALoader: Unsupported map file type.";
        return false;
    }
}

bool GTALoader::loadDFF(const QString &filePath) {
    qDebug() << "GTALoader: Loading DFF model:" << filePath;
    // Actual DFF parsing logic here
    return true;
}

bool GTALoader::loadTXD(const QString &filePath) {
    qDebug() << "GTALoader: Loading TXD texture dictionary:" << filePath;
    // Actual TXD parsing logic here
    return true;
}

bool GTALoader::loadIDE(const QString &filePath) {
    qDebug() << "GTALoader: Loading IDE definitions:" << filePath;
    // Actual IDE parsing logic here
    return true;
}

bool GTALoader::loadIPL(const QString &filePath) {
    qDebug() << "GTALoader: Loading IPL placements:" << filePath;
    // Actual IPL parsing logic here
    return true;
}

bool GTALoader::loadDAT(const QString &filePath) {
    qDebug() << "GTALoader: Loading DAT data:" << filePath;
    // Actual DAT parsing logic here
    return true;
}
```

### 4.8. `src/asset_manager.h` (Asset Manager Header - Conceptual)

Defines a conceptual class for managing assets.

```cpp
#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <QString>
#include <QMap>
#include <QDebug>

// Conceptual Asset class
class Asset {
public:
    enum Type { Model, Texture, Sound, Script, Other };

    Asset(const QString &name, Type type, const QString &path) :
        m_name(name), m_type(type), m_path(path) {}

    QString name() const { return m_name; }
    Type type() const { return m_type; }
    QString path() const { return m_path; }

private:
    QString m_name;
    Type m_type;
    QString m_path;
};

class AssetManager {
public:
    static AssetManager& instance();

    void loadAssetsFromDirectory(const QString &directoryPath);
    const Asset* getAsset(const QString &name) const;

private:
    AssetManager() = default;
    Q_DISABLE_COPY(AssetManager) // Singleton pattern

    QMap<QString, Asset*> m_assets;
};

#endif // ASSET_MANAGER_H
```

### 4.9. `src/asset_manager.cpp` (Asset Manager Implementation - Conceptual)

Provides a conceptual implementation for the asset manager.

```cpp
#include "asset_manager.h"
#include <QDir>
#include <QFileInfoList>

AssetManager& AssetManager::instance() {
    static AssetManager instance;
    return instance;
}

void AssetManager::loadAssetsFromDirectory(const QString &directoryPath) {
    qDebug() << "AssetManager: Loading assets from:" << directoryPath;
    QDir dir(directoryPath);
    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo &fileInfo : fileList) {
        Asset::Type type = Asset::Other;
        if (fileInfo.suffix().compare("dff", Qt::CaseInsensitive) == 0) {
            type = Asset::Model;
        } else if (fileInfo.suffix().compare("txd", Qt::CaseInsensitive) == 0) {
            type = Asset::Texture;
        }
        // Add more type detections

        Asset *newAsset = new Asset(fileInfo.baseName(), type, fileInfo.absoluteFilePath());
        m_assets.insert(newAsset->name(), newAsset);
        qDebug() << "AssetManager: Loaded asset:" << newAsset->name() << "(" << newAsset->path() << ")";
    }
}

const Asset* AssetManager::getAsset(const QString &name) const {
    return m_assets.value(name, nullptr);
}
```

### 4.10. Basic Example of Loading a GTA Map and Placing an Object

As shown in the `EditorWindow::openFile()` method, the editor will use `GTALoader::loadMapFile()` to conceptually load a GTA map file (e.g., `.ipl`). Within `GTALoader::loadMapFile()`, a dummy object is conceptually added to `s_objectPlacements` to simulate an object being loaded and placed. In a real scenario, the `GTALoader` would parse the actual binary data from `.ipl` and `.ide` files to extract object definitions and their placements (position, rotation, scale). These parsed objects would then be added to the editor's internal scene graph (e.g., `worldOutlinerTree` in the UI scaffold), and their properties would be accessible via the `Property Inspector` when selected.

For drag-and-drop placement, the `Asset Browser` (e.g., `assetListWidget`) would list available assets (models, prefabs). When a user drags an asset from the `Asset Browser` into the `3D Viewport`, the editor would create a new instance of that object in the scene at the drop location, assign it a unique ID, and add it to the `World Outliner`. Its default properties would then be editable in the `Property Inspector`.

This scaffold provides the foundational structure upon which the more complex 3D rendering, file parsing, and interaction logic would be built. The choice of Qt facilitates the creation of a rich, responsive, and cross-platform user interface, while the C++ backend ensures performance suitable for a low-end PC friendly application.



## 5. UI/UX Design and Visual Mockups

Based on the research of Unreal Editor (UT2004) and modern game development tools, the OpenRW Level and Mission Editor will feature a professional, multi-panel interface optimized for efficiency and ease of use. The design prioritizes the 3D viewport while providing intuitive access to essential tools and information.

### 5.1. Main Interface Design

The main interface mockup demonstrates a comprehensive layout inspired by classic game editors but with modern usability improvements:

**Key Design Elements:**
- **Central 3D Viewport:** The largest area displaying the game world with GTA-style urban environments, featuring buildings, streets, and props
- **Left Asset Browser Panel:** Organized categories for NPCs, vehicles, props, lights, and other game objects with thumbnail previews
- **Right Property Inspector Panel:** Dynamic component-based property editing with sliders, text fields, and dropdowns
- **World Outliner:** Hierarchical tree view showing all scene objects for easy selection and organization
- **Bottom Console Panel:** Log output and debugging information with clear, readable text
- **Professional Dark Theme:** Reduces eye strain during long editing sessions while maintaining good contrast for UI elements

### 5.2. Mission Editor Interface

The mission editor features a dedicated visual scripting interface designed for intuitive mission creation:

**Visual Scripting Canvas:**
- **Node-Based System:** Colorful, categorized nodes for triggers, conditions, actions, and objectives
- **Connection Lines:** Clear visual flow between mission logic elements
- **Node Palette:** Organized categories (Triggers, Conditions, Actions, Objectives, NPCs) for easy access
- **Mission Properties Panel:** Configuration options for mission difficulty, name, and description
- **Timeline View:** Linear representation of mission checkpoints and progression
- **Objective Management:** Clear list of mission objectives with status tracking

### 5.3. 3D Viewport Details

The 3D viewport incorporates professional-grade editing tools:

**Transformation Tools:**
- **Gizmos:** Color-coded transformation handles (red for X-axis, green for Y-axis, blue for Z-axis)
- **Grid Overlay:** Snap-to-grid functionality with visible grid lines for precise placement
- **Selection Highlighting:** Bright colored outlines for selected objects
- **Navigation Cube:** 3D orientation indicator for quick camera positioning
- **Rendering Modes:** Toggle between wireframe, solid, textured, and lit viewing modes

**Toolbar Integration:**
- **Selection Tools:** Various selection modes (single, multi, marquee)
- **Transform Modes:** Move, rotate, scale tools with keyboard shortcuts
- **Viewport Controls:** Camera navigation and rendering options
- **Snap Settings:** Grid snap, object snap, and angle snap controls

### 5.4. Color Scheme and Typography

**Color Palette:**
- **Primary Background:** Dark charcoal (#2D2D2D) for reduced eye strain
- **Panel Backgrounds:** Slightly lighter gray (#3A3A3A) for panel differentiation
- **Accent Colors:** Blue (#4A90E2) for active elements and Orange (#F5A623) for warnings/highlights
- **Text Colors:** Light gray (#E0E0E0) for primary text, white (#FFFFFF) for headers
- **Selection Colors:** Bright green (#7ED321) for selected objects, yellow (#F8E71C) for hover states

**Typography:**
- **Primary Font:** Segoe UI or system default sans-serif for cross-platform compatibility
- **Font Sizes:** 12px for body text, 14px for panel headers, 10px for small labels
- **Font Weights:** Regular for body text, semi-bold for headers and important elements

### 5.5. Responsive Layout and Docking

**Panel Management:**
- **Dockable Panels:** All panels can be undocked, moved, and redocked for custom layouts
- **Collapsible Sections:** Panels can be minimized to save screen space
- **Tabbed Interface:** Multiple panels can share the same space with tab navigation
- **Splitter Controls:** Adjustable panel sizes with drag handles between sections

**Accessibility Features:**
- **High Contrast Mode:** Alternative color scheme for users with visual impairments
- **Keyboard Navigation:** Full keyboard support for all interface elements
- **Tooltips:** Comprehensive help text for all tools and buttons
- **Customizable Shortcuts:** User-definable hotkeys for frequently used actions

### 5.6. Performance Considerations for Low-End PCs

**Optimization Strategies:**
- **Efficient Rendering:** Level-of-detail (LOD) system for 3D viewport to maintain smooth performance
- **Lazy Loading:** Assets loaded on-demand rather than all at once
- **Minimal UI Updates:** Only refresh UI elements when necessary to reduce CPU usage
- **Memory Management:** Proper cleanup of unused assets and UI elements
- **Scalable Interface:** UI elements that scale appropriately on different screen resolutions

The visual design balances professional functionality with user-friendly accessibility, ensuring that both experienced developers and newcomers can effectively use the editor. The interface draws inspiration from proven game development tools while incorporating modern UX principles for an optimal editing experience.


## 6. Core Framework Implementation

The core framework has been implemented with a robust, modular architecture that provides the foundation for the OpenRW Level and Mission Editor. This section details the implemented components and their functionality.

### 6.1. Entity-Component System

The editor uses a modern entity-component system (ECS) architecture that provides flexibility and extensibility for game objects. This design pattern separates data (components) from behavior (systems) and allows for dynamic composition of entity functionality.

**Key Components:**
- **Entity:** Base container class with unique ID and name, manages components
- **Component:** Base class for all data components (Transform, Mesh, Light, Script)
- **EntityManager:** Singleton managing entity lifecycle and serialization
- **ComponentType:** Enumeration defining available component types

**Benefits:**
- **Modularity:** Components can be mixed and matched to create different object types
- **Performance:** Data-oriented design enables efficient processing
- **Extensibility:** New component types can be added without modifying existing code
- **Serialization:** Built-in support for saving/loading entity configurations

### 6.2. File Format Parsers

Comprehensive parsers have been implemented for all major GTA file formats, enabling the editor to load and interpret existing game assets.

#### 6.2.1. DFF Parser (3D Models)
The DFF parser handles RenderWare model files used throughout the GTA series:
- **Chunk-based parsing:** Follows RenderWare's hierarchical chunk structure
- **Geometry extraction:** Vertices, normals, texture coordinates, and indices
- **Material support:** Basic material properties and texture references
- **Bounding box calculation:** Automatic computation for collision and culling

#### 6.2.2. TXD Parser (Textures)
The TXD parser processes RenderWare texture dictionary files:
- **Multiple formats:** Support for DXT1/3/5 compression and uncompressed formats
- **Platform compatibility:** Handles PC, PlayStation 2, and Xbox texture formats
- **Automatic decompression:** Converts compressed textures to standard image formats
- **Mipmap support:** Handles multiple resolution levels for performance optimization

#### 6.2.3. IPL Parser (Object Placement)
The IPL parser reads item placement list files that define object positions in the world:
- **Text and binary formats:** Supports both human-readable and optimized binary formats
- **Instance data:** Position, rotation, scale, and metadata for placed objects
- **Multiple sections:** Handles various IPL sections (INST, ZONE, CULL, etc.)
- **Cross-referencing:** Links placement data with IDE object definitions

#### 6.2.4. IDE Parser (Object Definitions)
The IDE parser processes item definition files that specify object properties:
- **Object metadata:** Model names, texture references, draw distances, and flags
- **Multiple object types:** Static objects, vehicles, pedestrians, and weapons
- **Flag interpretation:** Decodes object behavior flags for proper rendering and interaction
- **Asset linking:** Connects object IDs with their corresponding model and texture files

#### 6.2.5. DAT Parser (Data Files)
The DAT parser handles various data files including paths, vehicle handling, and water definitions:
- **Path networks:** AI navigation nodes with connections and properties
- **Vehicle handling:** Physics parameters for realistic vehicle behavior
- **Water planes:** Water surface definitions for environmental effects
- **Binary and text support:** Handles both format types depending on the specific DAT file

### 6.3. Scene Management System

The SceneManager provides centralized control over the 3D world and all entities within it:

**Core Functionality:**
- **Entity lifecycle:** Creation, destruction, and management of scene objects
- **Selection system:** Multi-object selection with primary selection tracking
- **Spatial queries:** Radius-based searches, bounding box queries, and raycasting
- **Layer management:** Organizational layers with visibility and lock controls
- **Grid and snapping:** Precision placement tools for level design

**Advanced Features:**
- **Camera management:** 3D viewport camera positioning and orientation
- **Rendering settings:** Wireframe mode, grid display, and bounding box visualization
- **Mission integration:** Trigger zones and objective management for mission design
- **Serialization:** Complete scene save/load functionality with JSON format

### 6.4. Mathematical Utilities

A comprehensive math library provides essential 3D operations:
- **Vector operations:** Distance calculations, interpolation, and clamping
- **Quaternion utilities:** Euler angle conversion and rotation operations
- **Matrix operations:** Transformation matrices and camera projections
- **Intersection tests:** Ray-sphere and ray-box collision detection
- **Snapping functions:** Grid alignment and angle snapping for precise editing

### 6.5. Type System and Data Structures

Well-defined data structures ensure type safety and clarity throughout the codebase:
- **Transform:** Position, rotation, and scale with matrix generation
- **BoundingBox:** Spatial bounds with containment testing
- **GTA-specific types:** Vertex, Material, Mesh, and Model structures
- **Mission types:** Trigger zones and objectives for gameplay scripting
- **Smart pointers:** Memory-safe reference counting with automatic cleanup

### 6.6. Cross-Platform Compatibility

The framework is designed for cross-platform deployment:
- **Qt framework:** Ensures consistent behavior across Windows, Linux, and macOS
- **Standard C++17:** Modern language features with broad compiler support
- **CMake build system:** Flexible build configuration for different platforms
- **Endianness handling:** Proper byte order management for file format compatibility

### 6.7. Performance Considerations

The implementation prioritizes performance for smooth editing on low-end hardware:
- **Lazy loading:** Assets loaded on-demand to minimize memory usage
- **Efficient data structures:** Optimized containers and algorithms
- **Spatial optimization:** Bounding box hierarchies for fast spatial queries
- **Memory management:** Smart pointers and RAII for automatic resource cleanup

This core framework provides a solid foundation for the remaining editor components, ensuring reliability, performance, and extensibility for the complete OpenRW Level and Mission Editor.

