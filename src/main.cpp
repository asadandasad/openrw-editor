#include "ui/main_window.h"
#include "scene_manager.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QSurfaceFormat>
#include <QOpenGLContext>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("OpenRW Level Editor");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("OpenRW Team");
    app.setOrganizationDomain("openrw.org");
    
    // Set application icon
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));
    
    // Set up OpenGL format
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4); // 4x MSAA
    QSurfaceFormat::setDefaultFormat(format);
    
    // Check OpenGL support
    QOpenGLContext context;
    context.setFormat(format);
    if (!context.create()) {
        qCritical() << "Failed to create OpenGL context";
        return -1;
    }
    
    qDebug() << "OpenGL Version:" << context.format().majorVersion() << "." << context.format().minorVersion();
    qDebug() << "OpenGL Profile:" << (context.format().profile() == QSurfaceFormat::CoreProfile ? "Core" : "Compatibility");
    
    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Set dark theme
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    // Set stylesheet for additional styling
    app.setStyleSheet(R"(
        QToolTip {
            color: #ffffff;
            background-color: #2a2a2a;
            border: 1px solid white;
        }
        
        QGroupBox {
            font-weight: bold;
            border: 2px solid #555555;
            border-radius: 5px;
            margin-top: 1ex;
            padding-top: 5px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        
        QDockWidget {
            titlebar-close-icon: url(:/icons/close.png);
            titlebar-normal-icon: url(:/icons/undock.png);
        }
        
        QDockWidget::title {
            text-align: left;
            background: #404040;
            padding-left: 5px;
        }
        
        QTabWidget::pane {
            border: 1px solid #555555;
        }
        
        QTabBar::tab {
            background: #404040;
            border: 1px solid #555555;
            padding: 5px;
            margin-right: 2px;
        }
        
        QTabBar::tab:selected {
            background: #606060;
        }
        
        QSplitter::handle {
            background: #555555;
        }
        
        QSplitter::handle:horizontal {
            width: 3px;
        }
        
        QSplitter::handle:vertical {
            height: 3px;
        }
    )");
    
    // Create application directories
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    QDir().mkpath(appDataPath + "/projects");
    QDir().mkpath(appDataPath + "/temp");
    QDir().mkpath(appDataPath + "/logs");
    
    qDebug() << "Application data path:" << appDataPath;
    
    // Initialize scene manager
    SceneManager& sceneManager = SceneManager::instance();
    sceneManager.newScene();
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    // Handle command line arguments
    QStringList args = app.arguments();
    if (args.size() > 1) {
        QString filePath = args[1];
        if (QFile::exists(filePath)) {
            // Load scene file if provided
            qDebug() << "Loading scene from command line:" << filePath;
            // TODO: Load scene file
        }
    }
    
    qDebug() << "OpenRW Level Editor started successfully";
    
    return app.exec();
}

