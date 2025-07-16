#include "viewport_widget.h"
#include "scene_manager.h"
#include "entity_system.h"
#include <QDebug>
#include <QApplication>
#include <QOpenGLShader>
#include <QtMath>

ViewportWidget::ViewportWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_cameraController(nullptr)
    , m_sceneManager(&SceneManager::instance())
    , m_renderMode(Textured)
    , m_showGrid(true)
    , m_showBoundingBoxes(false)
    , m_showGizmos(true)
    , m_gridSize(1.0f)
    , m_selectionMode(Single)
    , m_isSelecting(false)
    , m_gizmoMode(0)
    , m_isGizmoActive(false)
    , m_snapToGrid(false)
    , m_snapAngle(15.0f)
    , m_basicShader(nullptr)
    , m_gridShader(nullptr)
    , m_gizmoShader(nullptr)
    , m_updateTimer(nullptr)
    , m_lastFrameTime(0)
    , m_viewportWidth(800)
    , m_viewportHeight(600)
    , m_isMousePressed(false)
    , m_pressedButton(Qt::NoButton)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    // Create camera controller
    m_cameraController = new CameraController(this);
    connect(m_cameraController, &CameraController::cameraChanged, this, &ViewportWidget::updateViewport);
    
    // Connect to scene manager
    connect(m_sceneManager, &SceneManager::sceneChanged, this, &ViewportWidget::onSceneChanged);
    connect(m_sceneManager, &SceneManager::selectionChanged, this, &ViewportWidget::onSelectionChanged);
    
    // Setup update timer
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(16); // ~60 FPS
    connect(m_updateTimer, &QTimer::timeout, this, &ViewportWidget::updateViewport);
    m_updateTimer->start();
}

ViewportWidget::~ViewportWidget() {
    makeCurrent();
    
    // Clean up OpenGL resources
    for (auto& meshData : m_meshCache) {
        meshData.vbo.destroy();
        meshData.ebo.destroy();
        meshData.vao.destroy();
    }
    
    delete m_basicShader;
    delete m_gridShader;
    delete m_gizmoShader;
    
    doneCurrent();
}

void ViewportWidget::setRenderMode(RenderMode mode) {
    if (m_renderMode != mode) {
        m_renderMode = mode;
        update();
    }
}

ViewportWidget::RenderMode ViewportWidget::getRenderMode() const {
    return m_renderMode;
}

void ViewportWidget::setShowGrid(bool show) {
    if (m_showGrid != show) {
        m_showGrid = show;
        update();
    }
}

bool ViewportWidget::isShowGrid() const {
    return m_showGrid;
}

void ViewportWidget::setShowBoundingBoxes(bool show) {
    if (m_showBoundingBoxes != show) {
        m_showBoundingBoxes = show;
        update();
    }
}

bool ViewportWidget::isShowBoundingBoxes() const {
    return m_showBoundingBoxes;
}

void ViewportWidget::setGridSize(float size) {
    if (size > 0 && m_gridSize != size) {
        m_gridSize = size;
        update();
    }
}

float ViewportWidget::getGridSize() const {
    return m_gridSize;
}

void ViewportWidget::setSelectionMode(SelectionMode mode) {
    m_selectionMode = mode;
}

ViewportWidget::SelectionMode ViewportWidget::getSelectionMode() const {
    return m_selectionMode;
}

CameraController* ViewportWidget::getCameraController() {
    return m_cameraController;
}

void ViewportWidget::resetCamera() {
    m_cameraController->reset();
}

void ViewportWidget::focusOnSelection() {
    QVector<EntityId> selected = m_sceneManager->getSelectedEntities();
    if (selected.isEmpty()) {
        return;
    }
    
    QVector<BoundingBox> bounds;
    for (EntityId id : selected) {
        Entity* entity = m_sceneManager->getEntity(id);
        if (entity) {
            MeshComponent* meshComp = entity->getComponent<MeshComponent>();
            if (meshComp) {
                // Transform bounding box to world space
                Transform* transform = entity->getTransform();
                if (transform) {
                    QMatrix4x4 worldMatrix = transform->getMatrix();
                    QVector3D worldMin = worldMatrix * meshComp->boundingBox.min;
                    QVector3D worldMax = worldMatrix * meshComp->boundingBox.max;
                    bounds.append(BoundingBox(worldMin, worldMax));
                }
            }
        }
    }
    
    if (!bounds.isEmpty()) {
        m_cameraController->frameSelected(bounds);
    }
}

void ViewportWidget::focusOnEntity(Entity* entity) {
    if (!entity) {
        return;
    }
    
    MeshComponent* meshComp = entity->getComponent<MeshComponent>();
    if (meshComp) {
        Transform* transform = entity->getTransform();
        if (transform) {
            QMatrix4x4 worldMatrix = transform->getMatrix();
            QVector3D worldMin = worldMatrix * meshComp->boundingBox.min;
            QVector3D worldMax = worldMatrix * meshComp->boundingBox.max;
            m_cameraController->focusOn(BoundingBox(worldMin, worldMax));
        }
    } else {
        // Focus on entity position
        m_cameraController->focusOn(entity->getPosition());
    }
}

void ViewportWidget::setShowGizmos(bool show) {
    if (m_showGizmos != show) {
        m_showGizmos = show;
        update();
    }
}

bool ViewportWidget::isShowGizmos() const {
    return m_showGizmos;
}

void ViewportWidget::setGizmoMode(int mode) {
    if (mode >= 0 && mode <= 2 && m_gizmoMode != mode) {
        m_gizmoMode = mode;
        update();
    }
}

int ViewportWidget::getGizmoMode() const {
    return m_gizmoMode;
}

void ViewportWidget::setSnapToGrid(bool enabled) {
    m_snapToGrid = enabled;
}

bool ViewportWidget::isSnapToGrid() const {
    return m_snapToGrid;
}

void ViewportWidget::setSnapAngle(float angle) {
    if (angle > 0) {
        m_snapAngle = angle;
    }
}

float ViewportWidget::getSnapAngle() const {
    return m_snapAngle;
}

QVector3D ViewportWidget::screenToWorld(const QPoint& screenPos, float depth) const {
    return m_cameraController->screenToWorldPoint(screenPos, width(), height(), depth);
}

QPoint ViewportWidget::worldToScreen(const QVector3D& worldPos) const {
    return m_cameraController->worldToScreen(worldPos, width(), height());
}

QVector3D ViewportWidget::getMouseRay(const QPoint& screenPos) const {
    return m_cameraController->screenToWorldRay(screenPos, width(), height());
}

void ViewportWidget::initializeGL() {
    initializeOpenGLFunctions();
    
    qDebug() << "OpenGL Version:" << reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << "OpenGL Vendor:" << reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    qDebug() << "OpenGL Renderer:" << reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    
    // Set clear color
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Initialize shaders and buffers
    initializeShaders();
    initializeBuffers();
    setupLighting();
}

void ViewportWidget::resizeGL(int w, int h) {
    m_viewportWidth = w;
    m_viewportHeight = h;
    
    glViewport(0, 0, w, h);
    
    if (m_cameraController) {
        m_cameraController->setAspectRatio(static_cast<float>(w) / static_cast<float>(h));
    }
}

void ViewportWidget::paintGL() {
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (!m_cameraController) {
        return;
    }
    
    // Set wireframe mode if needed
    if (m_renderMode == Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // Render scene
    renderScene();
    
    // Render UI elements
    if (m_showGrid) {
        renderGrid();
    }
    
    if (m_showBoundingBoxes) {
        renderBoundingBoxes();
    }
    
    if (m_showGizmos && !m_selectedEntities.isEmpty()) {
        renderGizmos();
    }
    
    renderSelectionOutline();
}

void ViewportWidget::mousePressEvent(QMouseEvent* event) {
    m_isMousePressed = true;
    m_pressedButton = event->button();
    m_lastMousePos = event->pos();
    m_keyModifiers = event->modifiers();
    
    setFocus(); // Ensure we receive key events
    
    if (event->button() == Qt::LeftButton) {
        // Check if clicking on gizmo
        if (m_showGizmos && !m_selectedEntities.isEmpty() && isGizmoHovered(event->pos())) {
            startGizmoInteraction(event->pos());
            return;
        }
        
        // Perform selection
        if (m_selectionMode == Marquee) {
            m_isSelecting = true;
            m_selectionStart = event->pos();
            m_marqueeRect = QRect(m_selectionStart, m_selectionStart);
        } else {
            performSelection(event->pos());
        }
    }
}

void ViewportWidget::mouseMoveEvent(QMouseEvent* event) {
    QPoint delta = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();
    
    if (m_isGizmoActive) {
        updateGizmoInteraction(event->pos());
        return;
    }
    
    if (m_isSelecting && m_selectionMode == Marquee) {
        m_marqueeRect = QRect(m_selectionStart, event->pos()).normalized();
        update();
        return;
    }
    
    if (m_isMousePressed) {
        // Handle camera movement
        m_cameraController->handleMouseMove(delta, m_pressedButton, m_keyModifiers);
    }
}

void ViewportWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (m_isGizmoActive) {
        endGizmoInteraction();
    }
    
    if (m_isSelecting && m_selectionMode == Marquee) {
        performMarqueeSelection(m_marqueeRect);
        m_isSelecting = false;
        update();
    }
    
    m_isMousePressed = false;
    m_pressedButton = Qt::NoButton;
}

void ViewportWidget::wheelEvent(QWheelEvent* event) {
    m_cameraController->handleMouseWheel(event->angleDelta().y(), event->modifiers());
}

void ViewportWidget::keyPressEvent(QKeyEvent* event) {
    m_cameraController->handleKeyPress(static_cast<Qt::Key>(event->key()), event->modifiers());
    
    // Handle editor-specific keys
    switch (event->key()) {
        case Qt::Key_F:
            focusOnSelection();
            break;
        case Qt::Key_G:
            setGizmoMode(0); // Translate
            break;
        case Qt::Key_R:
            setGizmoMode(1); // Rotate
            break;
        case Qt::Key_S:
            setGizmoMode(2); // Scale
            break;
        case Qt::Key_Delete:
            // Delete selected entities
            for (EntityId id : m_selectedEntities) {
                m_sceneManager->destroyEntity(id);
            }
            break;
    }
}

void ViewportWidget::keyReleaseEvent(QKeyEvent* event) {
    m_cameraController->handleKeyRelease(static_cast<Qt::Key>(event->key()));
}

void ViewportWidget::onSceneChanged() {
    update();
}

void ViewportWidget::onSelectionChanged(const QVector<EntityId>& selectedIds) {
    m_selectedEntities = selectedIds;
    update();
}

void ViewportWidget::onEntityTransformed() {
    update();
}

void ViewportWidget::updateViewport() {
    update();
}

void ViewportWidget::initializeShaders() {
    // Basic shader for rendering meshes
    m_basicShader = new QOpenGLShaderProgram(this);
    
    // Vertex shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoord;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform mat3 normalMatrix;
        
        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoord;
        
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = normalMatrix * aNormal;
            TexCoord = aTexCoord;
            
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";
    
    // Fragment shader
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoord;
        
        uniform vec3 lightPos;
        uniform vec3 lightColor;
        uniform vec3 viewPos;
        uniform vec3 objectColor;
        uniform bool useTexture;
        uniform sampler2D texture1;
        
        void main() {
            vec3 color = objectColor;
            if (useTexture) {
                color = texture(texture1, TexCoord).rgb;
            }
            
            // Ambient
            float ambientStrength = 0.3;
            vec3 ambient = ambientStrength * lightColor;
            
            // Diffuse
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;
            
            // Specular
            float specularStrength = 0.5;
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = specularStrength * spec * lightColor;
            
            vec3 result = (ambient + diffuse + specular) * color;
            FragColor = vec4(result, 1.0);
        }
    )";
    
    m_basicShader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_basicShader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_basicShader->link();
    
    // Grid shader
    m_gridShader = new QOpenGLShaderProgram(this);
    
    const char* gridVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 mvp;
        
        void main() {
            gl_Position = mvp * vec4(aPos, 1.0);
        }
    )";
    
    const char* gridFragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec3 color;
        
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";
    
    m_gridShader->addShaderFromSourceCode(QOpenGLShader::Vertex, gridVertexShader);
    m_gridShader->addShaderFromSourceCode(QOpenGLShader::Fragment, gridFragmentShader);
    m_gridShader->link();
    
    // Gizmo shader (similar to grid shader for now)
    m_gizmoShader = new QOpenGLShaderProgram(this);
    m_gizmoShader->addShaderFromSourceCode(QOpenGLShader::Vertex, gridVertexShader);
    m_gizmoShader->addShaderFromSourceCode(QOpenGLShader::Fragment, gridFragmentShader);
    m_gizmoShader->link();
}

void ViewportWidget::initializeBuffers() {
    // Initialize grid VAO and VBO
    m_gridVAO.create();
    m_gridVBO.create();
    
    // Initialize gizmo VAO and VBO
    m_gizmoVAO.create();
    m_gizmoVBO.create();
}

void ViewportWidget::setupLighting() {
    // Setup default lighting parameters
    // This will be used in the shaders
}

void ViewportWidget::renderScene() {
    if (!m_basicShader || !m_cameraController) {
        return;
    }
    
    m_basicShader->bind();
    
    // Set matrices
    QMatrix4x4 view = m_cameraController->getViewMatrix();
    QMatrix4x4 projection = m_cameraController->getProjectionMatrix();
    
    m_basicShader->setUniformValue("view", view);
    m_basicShader->setUniformValue("projection", projection);
    
    // Set lighting
    m_basicShader->setUniformValue("lightPos", QVector3D(10.0f, 10.0f, 10.0f));
    m_basicShader->setUniformValue("lightColor", QVector3D(1.0f, 1.0f, 1.0f));
    m_basicShader->setUniformValue("viewPos", m_cameraController->getPosition());
    
    // Render entities
    renderEntities();
    
    m_basicShader->release();
}

void ViewportWidget::renderEntities() {
    for (Entity* entity : m_sceneManager->getAllEntities()) {
        if (entity) {
            renderEntity(entity);
        }
    }
}

void ViewportWidget::renderEntity(Entity* entity) {
    if (!entity) {
        return;
    }
    
    // Check if entity's layer is visible
    QString layer = m_sceneManager->getEntityLayer(entity->getId());
    if (!layer.isEmpty() && !m_sceneManager->isLayerVisible(layer)) {
        return;
    }
    
    MeshComponent* meshComp = entity->getComponent<MeshComponent>();
    Transform* transform = entity->getTransform();
    
    if (!meshComp || !transform || !meshComp->isVisible) {
        return;
    }
    
    // Set model matrix
    QMatrix4x4 model = transform->getMatrix();
    m_basicShader->setUniformValue("model", model);
    
    // Set normal matrix
    QMatrix3x3 normalMatrix = model.normalMatrix();
    m_basicShader->setUniformValue("normalMatrix", normalMatrix);
    
    // Set object color (placeholder)
    m_basicShader->setUniformValue("objectColor", QVector3D(0.8f, 0.8f, 0.8f));
    m_basicShader->setUniformValue("useTexture", false);
    
    // TODO: Render actual mesh data
    // For now, we'll render a simple cube as placeholder
    renderPlaceholderCube();
}

void ViewportWidget::renderGrid() {
    if (!m_gridShader) {
        return;
    }
    
    m_gridShader->bind();
    
    QMatrix4x4 mvp = m_cameraController->getViewProjectionMatrix();
    m_gridShader->setUniformValue("mvp", mvp);
    m_gridShader->setUniformValue("color", QVector3D(0.5f, 0.5f, 0.5f));
    
    // Generate grid lines
    QVector<QVector3D> gridLines;
    float gridExtent = 50.0f;
    int gridCount = static_cast<int>(gridExtent / m_gridSize);
    
    // Horizontal lines
    for (int i = -gridCount; i <= gridCount; ++i) {
        float pos = i * m_gridSize;
        gridLines.append(QVector3D(-gridExtent, 0, pos));
        gridLines.append(QVector3D(gridExtent, 0, pos));
    }
    
    // Vertical lines
    for (int i = -gridCount; i <= gridCount; ++i) {
        float pos = i * m_gridSize;
        gridLines.append(QVector3D(pos, 0, -gridExtent));
        gridLines.append(QVector3D(pos, 0, gridExtent));
    }
    
    // Upload and render grid
    m_gridVAO.bind();
    m_gridVBO.bind();
    m_gridVBO.allocate(gridLines.constData(), gridLines.size() * sizeof(QVector3D));
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
    
    glDrawArrays(GL_LINES, 0, gridLines.size());
    
    m_gridVAO.release();
    m_gridShader->release();
}

void ViewportWidget::renderBoundingBoxes() {
    // TODO: Implement bounding box rendering
}

void ViewportWidget::renderGizmos() {
    // TODO: Implement gizmo rendering
}

void ViewportWidget::renderSelectionOutline() {
    // TODO: Implement selection outline rendering
}

void ViewportWidget::renderMesh(const GTAMesh& mesh, const QMatrix4x4& modelMatrix) {
    // TODO: Implement actual mesh rendering
}

void ViewportWidget::uploadMeshToGPU(const GTAMesh& mesh) {
    // TODO: Implement mesh upload to GPU
}

void ViewportWidget::performSelection(const QPoint& screenPos) {
    Entity* entity = pickEntity(screenPos);
    
    if (entity) {
        EntityId id = entity->getId();
        
        if (m_keyModifiers & Qt::ControlModifier) {
            // Toggle selection
            if (m_selectedEntities.contains(id)) {
                m_sceneManager->deselectEntity(id);
                emit entityDeselected(id);
            } else {
                m_sceneManager->selectEntity(id);
                emit entitySelected(id);
            }
        } else {
            // Single selection
            m_sceneManager->clearSelection();
            m_sceneManager->selectEntity(id);
            emit entitySelected(id);
        }
    } else if (!(m_keyModifiers & Qt::ControlModifier)) {
        // Clear selection if not holding Ctrl
        m_sceneManager->clearSelection();
    }
}

void ViewportWidget::performMarqueeSelection(const QRect& rect) {
    // TODO: Implement marquee selection
}

Entity* ViewportWidget::pickEntity(const QPoint& screenPos) {
    QVector3D rayOrigin = m_cameraController->getPosition();
    QVector3D rayDirection = getMouseRay(screenPos);
    
    return m_sceneManager->raycast(rayOrigin, rayDirection);
}

bool ViewportWidget::isGizmoHovered(const QPoint& screenPos) {
    // TODO: Implement gizmo hover detection
    return false;
}

void ViewportWidget::startGizmoInteraction(const QPoint& screenPos) {
    // TODO: Implement gizmo interaction start
}

void ViewportWidget::updateGizmoInteraction(const QPoint& screenPos) {
    // TODO: Implement gizmo interaction update
}

void ViewportWidget::endGizmoInteraction() {
    // TODO: Implement gizmo interaction end
}

QMatrix4x4 ViewportWidget::getViewMatrix() const {
    return m_cameraController ? m_cameraController->getViewMatrix() : QMatrix4x4();
}

QMatrix4x4 ViewportWidget::getProjectionMatrix() const {
    return m_cameraController ? m_cameraController->getProjectionMatrix() : QMatrix4x4();
}

QVector3D ViewportWidget::getWorldPosition(const QPoint& screenPos, float depth) const {
    return screenToWorld(screenPos, depth);
}

void ViewportWidget::renderPlaceholderCube() {
    // Simple cube vertices for placeholder rendering
    static const float vertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    };
    
    static const unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,   // Front
        4, 5, 6, 6, 7, 4,   // Back
        7, 6, 2, 2, 3, 7,   // Top
        4, 5, 1, 1, 0, 4,   // Bottom
        4, 0, 3, 3, 7, 4,   // Left
        1, 5, 6, 6, 2, 1    // Right
    };
    
    // Create temporary buffers for cube
    static QOpenGLBuffer cubeVBO;
    static QOpenGLBuffer cubeEBO;
    static QOpenGLVertexArrayObject cubeVAO;
    static bool initialized = false;
    
    if (!initialized) {
        cubeVAO.create();
        cubeVBO.create();
        cubeEBO.create();
        
        cubeVAO.bind();
        
        cubeVBO.bind();
        cubeVBO.allocate(vertices, sizeof(vertices));
        
        cubeEBO.bind();
        cubeEBO.allocate(indices, sizeof(indices));
        
        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
        
        // Normal attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
        
        // Texture coordinate attribute
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
        
        cubeVAO.release();
        initialized = true;
    }
    
    cubeVAO.bind();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    cubeVAO.release();
}

#include "viewport_widget.moc"

