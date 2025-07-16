#ifndef VIEWPORT_WIDGET_H
#define VIEWPORT_WIDGET_H

#include "types.h"
#include "camera_controller.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTimer>

class Entity;
class SceneManager;

// 3D viewport widget for displaying and interacting with the scene
class ViewportWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
    
public:
    enum RenderMode {
        Wireframe,
        Solid,
        Textured,
        Lit
    };
    
    enum SelectionMode {
        Single,
        Multiple,
        Marquee
    };
    
    explicit ViewportWidget(QWidget* parent = nullptr);
    ~ViewportWidget();
    
    // Rendering settings
    void setRenderMode(RenderMode mode);
    RenderMode getRenderMode() const;
    
    void setShowGrid(bool show);
    bool isShowGrid() const;
    
    void setShowBoundingBoxes(bool show);
    bool isShowBoundingBoxes() const;
    
    void setGridSize(float size);
    float getGridSize() const;
    
    // Selection
    void setSelectionMode(SelectionMode mode);
    SelectionMode getSelectionMode() const;
    
    // Camera control
    CameraController* getCameraController();
    void resetCamera();
    void focusOnSelection();
    void focusOnEntity(Entity* entity);
    
    // Transformation gizmos
    void setShowGizmos(bool show);
    bool isShowGizmos() const;
    
    void setGizmoMode(int mode); // 0=translate, 1=rotate, 2=scale
    int getGizmoMode() const;
    
    // Snapping
    void setSnapToGrid(bool enabled);
    bool isSnapToGrid() const;
    
    void setSnapAngle(float angle);
    float getSnapAngle() const;
    
    // Viewport info
    QVector3D screenToWorld(const QPoint& screenPos, float depth = 0.0f) const;
    QPoint worldToScreen(const QVector3D& worldPos) const;
    QVector3D getMouseRay(const QPoint& screenPos) const;
    
signals:
    void entitySelected(EntityId id);
    void entityDeselected(EntityId id);
    void selectionChanged(const QVector<EntityId>& selectedIds);
    void entityTransformed(EntityId id, const Transform& newTransform);
    void cameraChanged(const QVector3D& position, const QVector3D& target);
    
protected:
    // QOpenGLWidget overrides
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    // Input handling
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    
private slots:
    void onSceneChanged();
    void onSelectionChanged(const QVector<EntityId>& selectedIds);
    void onEntityTransformed();
    void updateViewport();
    
private:
    // Initialization
    void initializeShaders();
    void initializeBuffers();
    void setupLighting();
    
    // Rendering
    void renderScene();
    void renderEntities();
    void renderEntity(Entity* entity);
    void renderGrid();
    void renderBoundingBoxes();
    void renderGizmos();
    void renderSelectionOutline();
    
    // Mesh rendering
    void renderMesh(const GTAMesh& mesh, const QMatrix4x4& modelMatrix);
    void uploadMeshToGPU(const GTAMesh& mesh);
    
    // Selection
    void performSelection(const QPoint& screenPos);
    void performMarqueeSelection(const QRect& rect);
    Entity* pickEntity(const QPoint& screenPos);
    
    // Gizmo interaction
    bool isGizmoHovered(const QPoint& screenPos);
    void startGizmoInteraction(const QPoint& screenPos);
    void updateGizmoInteraction(const QPoint& screenPos);
    void endGizmoInteraction();
    
    // Utility
    QMatrix4x4 getViewMatrix() const;
    QMatrix4x4 getProjectionMatrix() const;
    QVector3D getWorldPosition(const QPoint& screenPos, float depth) const;
    
    // Camera and scene
    CameraController* m_cameraController;
    SceneManager* m_sceneManager;
    
    // Rendering state
    RenderMode m_renderMode;
    bool m_showGrid;
    bool m_showBoundingBoxes;
    bool m_showGizmos;
    float m_gridSize;
    
    // Selection state
    SelectionMode m_selectionMode;
    QVector<EntityId> m_selectedEntities;
    bool m_isSelecting;
    QPoint m_selectionStart;
    QRect m_marqueeRect;
    
    // Gizmo state
    int m_gizmoMode; // 0=translate, 1=rotate, 2=scale
    bool m_isGizmoActive;
    QPoint m_gizmoStartPos;
    Transform m_gizmoStartTransform;
    
    // Snapping
    bool m_snapToGrid;
    float m_snapAngle;
    
    // OpenGL resources
    QOpenGLShaderProgram* m_basicShader;
    QOpenGLShaderProgram* m_gridShader;
    QOpenGLShaderProgram* m_gizmoShader;
    
    QOpenGLBuffer m_gridVBO;
    QOpenGLBuffer m_gizmoVBO;
    QOpenGLVertexArrayObject m_gridVAO;
    QOpenGLVertexArrayObject m_gizmoVAO;
    
    // Mesh cache
    struct MeshData {
        QOpenGLBuffer vbo;
        QOpenGLBuffer ebo;
        QOpenGLVertexArrayObject vao;
        int indexCount;
        bool isUploaded;
    };
    QMap<QString, MeshData> m_meshCache;
    
    // Timing
    QTimer* m_updateTimer;
    qint64 m_lastFrameTime;
    
    // Viewport dimensions
    int m_viewportWidth;
    int m_viewportHeight;
    
    // Input state
    bool m_isMousePressed;
    QPoint m_lastMousePos;
    Qt::MouseButton m_pressedButton;
    Qt::KeyboardModifiers m_keyModifiers;
};

#endif // VIEWPORT_WIDGET_H

