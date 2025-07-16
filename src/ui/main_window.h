#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

class ViewportWidget;
class PropertyInspector;
class AssetBrowser;
class WorldOutliner;
class SceneManager;

// Main application window
class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
protected:
    void closeEvent(QCloseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    
private slots:
    // File menu
    void onNewScene();
    void onOpenScene();
    void onSaveScene();
    void onSaveSceneAs();
    void onImportGTAMap();
    void onImportDFFModel();
    void onExportScene();
    void onRecentFileTriggered();
    void onExit();
    
    // Edit menu
    void onUndo();
    void onRedo();
    void onCut();
    void onCopy();
    void onPaste();
    void onDelete();
    void onSelectAll();
    void onDeselectAll();
    void onDuplicate();
    
    // View menu
    void onToggleGrid();
    void onToggleBoundingBoxes();
    void onToggleWireframe();
    void onResetCamera();
    void onFocusSelection();
    void onFrameAll();
    void onToggleFullscreen();
    
    // Tools menu
    void onTransformModeChanged();
    void onSnapToGrid();
    void onSnapSettings();
    void onPreferences();
    
    // Build menu
    void onTestMission();
    void onBuildMission();
    void onValidateScene();
    
    // Help menu
    void onAbout();
    void onDocumentation();
    void onReportBug();
    
    // Viewport events
    void onEntitySelected(EntityId id);
    void onEntityDeselected(EntityId id);
    void onSelectionChanged(const QVector<EntityId>& selectedIds);
    void onCameraChanged();
    
    // Scene events
    void onSceneChanged();
    void onSceneLoaded(const QString& filePath);
    void onSceneSaved(const QString& filePath);
    
    // Asset events
    void onAssetSelected(const AssetBrowser::AssetInfo& asset);
    void onAssetDoubleClicked(const AssetBrowser::AssetInfo& asset);
    void onAssetDragStarted(const AssetBrowser::AssetInfo& asset);
    
    // Property events
    void onPropertyChanged(EntityId entityId, const QString& propertyName, const QVariant& value);
    
    // Status updates
    void updateStatusBar();
    void showStatusMessage(const QString& message, int timeout = 2000);
    void updateWindowTitle();
    
private:
    // UI setup
    void setupUI();
    void setupMenuBar();
    void setupToolBars();
    void setupStatusBar();
    void setupDockWidgets();
    void setupCentralWidget();
    void setupConnections();
    
    // Menu creation
    void createFileMenu();
    void createEditMenu();
    void createViewMenu();
    void createToolsMenu();
    void createBuildMenu();
    void createHelpMenu();
    
    // Toolbar creation
    void createMainToolBar();
    void createTransformToolBar();
    void createViewToolBar();
    
    // Settings
    void loadSettings();
    void saveSettings();
    void restoreLayout();
    void saveLayout();
    
    // File operations
    bool saveCurrentScene();
    bool loadScene(const QString& filePath);
    bool importGTAMap(const QString& iplPath, const QString& idePath);
    bool importDFFModel(const QString& dffPath);
    void addRecentFile(const QString& filePath);
    void updateRecentFileActions();
    
    // Utility
    bool confirmUnsavedChanges();
    QString getSceneFileFilter() const;
    QString getGTAFileFilter() const;
    
    // Central widget
    QWidget* m_centralWidget;
    QSplitter* m_mainSplitter;
    ViewportWidget* m_viewport;
    
    // Dock widgets
    QDockWidget* m_propertyDock;
    QDockWidget* m_assetBrowserDock;
    QDockWidget* m_worldOutlinerDock;
    
    // UI components
    PropertyInspector* m_propertyInspector;
    AssetBrowser* m_assetBrowser;
    WorldOutliner* m_worldOutliner;
    
    // Menu bar
    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_viewMenu;
    QMenu* m_toolsMenu;
    QMenu* m_buildMenu;
    QMenu* m_helpMenu;
    QMenu* m_recentFilesMenu;
    
    // Tool bars
    QToolBar* m_mainToolBar;
    QToolBar* m_transformToolBar;
    QToolBar* m_viewToolBar;
    
    // Status bar
    QLabel* m_statusLabel;
    QLabel* m_selectionLabel;
    QLabel* m_cameraLabel;
    QProgressBar* m_progressBar;
    
    // Actions - File
    QAction* m_newSceneAction;
    QAction* m_openSceneAction;
    QAction* m_saveSceneAction;
    QAction* m_saveSceneAsAction;
    QAction* m_importGTAMapAction;
    QAction* m_importDFFModelAction;
    QAction* m_exportSceneAction;
    QAction* m_exitAction;
    QVector<QAction*> m_recentFileActions;
    
    // Actions - Edit
    QAction* m_undoAction;
    QAction* m_redoAction;
    QAction* m_cutAction;
    QAction* m_copyAction;
    QAction* m_pasteAction;
    QAction* m_deleteAction;
    QAction* m_selectAllAction;
    QAction* m_deselectAllAction;
    QAction* m_duplicateAction;
    
    // Actions - View
    QAction* m_toggleGridAction;
    QAction* m_toggleBoundingBoxesAction;
    QAction* m_toggleWireframeAction;
    QAction* m_resetCameraAction;
    QAction* m_focusSelectionAction;
    QAction* m_frameAllAction;
    QAction* m_toggleFullscreenAction;
    
    // Actions - Tools
    QActionGroup* m_transformModeGroup;
    QAction* m_translateModeAction;
    QAction* m_rotateModeAction;
    QAction* m_scaleModeAction;
    QAction* m_snapToGridAction;
    QAction* m_snapSettingsAction;
    QAction* m_preferencesAction;
    
    // Actions - Build
    QAction* m_testMissionAction;
    QAction* m_buildMissionAction;
    QAction* m_validateSceneAction;
    
    // Actions - Help
    QAction* m_aboutAction;
    QAction* m_documentationAction;
    QAction* m_reportBugAction;
    
    // Data
    SceneManager* m_sceneManager;
    QString m_currentSceneFile;
    bool m_sceneModified;
    QSettings* m_settings;
    
    // Constants
    static const int MaxRecentFiles = 10;
    static const QString OrganizationName;
    static const QString ApplicationName;
};

#endif // MAIN_WINDOW_H

