#include "camera_controller.h"
#include <QTimer>
#include <QDebug>
#include <QtMath>

CameraController::CameraController(QObject* parent)
    : QObject(parent)
    , m_mode(Orbit)
    , m_position(0, 0, 10)
    , m_target(0, 0, 0)
    , m_up(0, 1, 0)
    , m_worldUp(0, 1, 0)
    , m_distance(10.0f)
    , m_yaw(-90.0f)
    , m_pitch(0.0f)
    , m_forward(0, 0, -1)
    , m_right(1, 0, 0)
    , m_fieldOfView(45.0f)
    , m_aspectRatio(16.0f / 9.0f)
    , m_nearPlane(0.1f)
    , m_farPlane(1000.0f)
    , m_movementSpeed(5.0f)
    , m_rotationSpeed(0.1f)
    , m_zoomSpeed(1.0f)
    , m_isFirstMouse(true)
    , m_isAnimating(false)
    , m_animProgress(0.0f)
    , m_animDuration(1.0f)
    , m_minDistance(1.0f)
    , m_maxDistance(100.0f)
    , m_minPitch(-89.0f)
    , m_maxPitch(89.0f)
{
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16); // ~60 FPS
    connect(m_animTimer, &QTimer::timeout, this, &CameraController::updateAnimation);
    
    updateCameraVectors();
}

void CameraController::setPosition(const QVector3D& position) {
    if (m_position != position) {
        m_position = position;
        updateCameraVectors();
        emit positionChanged(m_position);
        emit cameraChanged();
    }
}

QVector3D CameraController::getPosition() const {
    return m_position;
}

void CameraController::setTarget(const QVector3D& target) {
    if (m_target != target) {
        m_target = target;
        if (m_mode == Orbit) {
            updateOrbitCamera();
        }
        updateCameraVectors();
        emit targetChanged(m_target);
        emit cameraChanged();
    }
}

QVector3D CameraController::getTarget() const {
    return m_target;
}

void CameraController::setUp(const QVector3D& up) {
    if (m_up != up) {
        m_up = up;
        m_worldUp = up;
        updateCameraVectors();
        emit cameraChanged();
    }
}

QVector3D CameraController::getUp() const {
    return m_up;
}

void CameraController::setDistance(float distance) {
    distance = qBound(m_minDistance, distance, m_maxDistance);
    if (m_distance != distance) {
        m_distance = distance;
        if (m_mode == Orbit) {
            updateOrbitCamera();
        }
        emit cameraChanged();
    }
}

float CameraController::getDistance() const {
    return m_distance;
}

void CameraController::setCameraMode(CameraMode mode) {
    if (m_mode != mode) {
        m_mode = mode;
        updateCameraVectors();
        emit cameraChanged();
    }
}

CameraController::CameraMode CameraController::getCameraMode() const {
    return m_mode;
}

void CameraController::setFieldOfView(float fov) {
    fov = qBound(1.0f, fov, 179.0f);
    if (m_fieldOfView != fov) {
        m_fieldOfView = fov;
        emit cameraChanged();
    }
}

float CameraController::getFieldOfView() const {
    return m_fieldOfView;
}

void CameraController::setNearPlane(float nearPlane) {
    if (m_nearPlane != nearPlane && nearPlane > 0) {
        m_nearPlane = nearPlane;
        emit cameraChanged();
    }
}

float CameraController::getNearPlane() const {
    return m_nearPlane;
}

void CameraController::setFarPlane(float farPlane) {
    if (m_farPlane != farPlane && farPlane > m_nearPlane) {
        m_farPlane = farPlane;
        emit cameraChanged();
    }
}

float CameraController::getFarPlane() const {
    return m_farPlane;
}

void CameraController::setAspectRatio(float aspectRatio) {
    if (m_aspectRatio != aspectRatio && aspectRatio > 0) {
        m_aspectRatio = aspectRatio;
        emit cameraChanged();
    }
}

float CameraController::getAspectRatio() const {
    return m_aspectRatio;
}

void CameraController::setMovementSpeed(float speed) {
    if (speed > 0) {
        m_movementSpeed = speed;
    }
}

float CameraController::getMovementSpeed() const {
    return m_movementSpeed;
}

void CameraController::setRotationSpeed(float speed) {
    if (speed > 0) {
        m_rotationSpeed = speed;
    }
}

float CameraController::getRotationSpeed() const {
    return m_rotationSpeed;
}

void CameraController::setZoomSpeed(float speed) {
    if (speed > 0) {
        m_zoomSpeed = speed;
    }
}

float CameraController::getZoomSpeed() const {
    return m_zoomSpeed;
}

QMatrix4x4 CameraController::getViewMatrix() const {
    return MathUtils::lookAt(m_position, m_target, m_up);
}

QMatrix4x4 CameraController::getProjectionMatrix() const {
    return MathUtils::perspective(m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane);
}

QMatrix4x4 CameraController::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

QVector3D CameraController::getForward() const {
    return m_forward;
}

QVector3D CameraController::getRight() const {
    return m_right;
}

QVector3D CameraController::getUpVector() const {
    return m_up;
}

void CameraController::handleMouseMove(const QPoint& delta, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers) {
    if (m_isAnimating) {
        stopAnimation();
    }
    
    float xOffset = delta.x() * m_rotationSpeed;
    float yOffset = -delta.y() * m_rotationSpeed; // Reversed since y-coordinates go from bottom to top
    
    if (buttons & Qt::LeftButton) {
        if (m_mode == Orbit) {
            m_yaw += xOffset;
            m_pitch += yOffset;
            constrainPitch();
            updateOrbitCamera();
        } else if (m_mode == Fly || m_mode == Walk) {
            m_yaw += xOffset;
            m_pitch += yOffset;
            constrainPitch();
            updateCameraVectors();
        }
        emit cameraChanged();
    } else if (buttons & Qt::MiddleButton) {
        // Pan camera
        QVector3D right = getRight();
        QVector3D up = getUpVector();
        
        float panSpeed = m_movementSpeed * 0.01f;
        QVector3D offset = (-xOffset * right + yOffset * up) * panSpeed;
        
        if (m_mode == Orbit) {
            m_target += offset;
            updateOrbitCamera();
        } else {
            m_position += offset;
            m_target += offset;
        }
        emit cameraChanged();
    }
}

void CameraController::handleMouseWheel(int delta, Qt::KeyboardModifiers modifiers) {
    if (m_isAnimating) {
        stopAnimation();
    }
    
    float zoomAmount = delta * m_zoomSpeed * 0.001f;
    
    if (m_mode == Orbit) {
        setDistance(m_distance - zoomAmount);
    } else {
        // Move camera forward/backward
        QVector3D forward = getForward();
        m_position += forward * zoomAmount;
        if (m_mode == Fly) {
            m_target = m_position + forward;
        }
        emit cameraChanged();
    }
}

void CameraController::handleKeyPress(Qt::Key key, Qt::KeyboardModifiers modifiers) {
    m_pressedKeys.insert(key);
    
    if (m_mode == Fly || m_mode == Walk) {
        float velocity = m_movementSpeed * 0.1f; // Adjust for frame rate
        
        QVector3D movement;
        if (m_pressedKeys.contains(Qt::Key_W)) {
            movement += getForward();
        }
        if (m_pressedKeys.contains(Qt::Key_S)) {
            movement -= getForward();
        }
        if (m_pressedKeys.contains(Qt::Key_A)) {
            movement -= getRight();
        }
        if (m_pressedKeys.contains(Qt::Key_D)) {
            movement += getRight();
        }
        if (m_pressedKeys.contains(Qt::Key_Q) && m_mode == Fly) {
            movement -= m_worldUp;
        }
        if (m_pressedKeys.contains(Qt::Key_E) && m_mode == Fly) {
            movement += m_worldUp;
        }
        
        if (!movement.isNull()) {
            movement.normalize();
            m_position += movement * velocity;
            if (m_mode == Fly) {
                m_target = m_position + getForward();
            } else if (m_mode == Walk) {
                m_target = m_position + QVector3D(getForward().x(), 0, getForward().z()).normalized();
            }
            emit cameraChanged();
        }
    }
}

void CameraController::handleKeyRelease(Qt::Key key) {
    m_pressedKeys.remove(key);
}

void CameraController::reset() {
    stopAnimation();
    
    m_position = QVector3D(0, 0, 10);
    m_target = QVector3D(0, 0, 0);
    m_up = QVector3D(0, 1, 0);
    m_worldUp = QVector3D(0, 1, 0);
    m_distance = 10.0f;
    m_yaw = -90.0f;
    m_pitch = 0.0f;
    
    updateCameraVectors();
    emit cameraChanged();
}

void CameraController::lookAt(const QVector3D& position, const QVector3D& target, const QVector3D& up) {
    stopAnimation();
    
    m_position = position;
    m_target = target;
    m_up = up;
    m_worldUp = up;
    
    if (m_mode == Orbit) {
        m_distance = MathUtils::distance(position, target);
        QVector3D direction = (position - target).normalized();
        m_yaw = MathUtils::radiansToDegrees(qAtan2(direction.z(), direction.x()));
        m_pitch = MathUtils::radiansToDegrees(qAsin(direction.y()));
    }
    
    updateCameraVectors();
    emit cameraChanged();
}

void CameraController::orbitAround(const QVector3D& target, float distance) {
    stopAnimation();
    
    m_mode = Orbit;
    m_target = target;
    setDistance(distance);
    updateOrbitCamera();
}

void CameraController::focusOn(const BoundingBox& bounds) {
    QVector3D center = bounds.center();
    QVector3D size = bounds.size();
    float maxSize = qMax(qMax(size.x(), size.y()), size.z());
    float distance = maxSize / qTan(MathUtils::degreesToRadians(m_fieldOfView * 0.5f)) * 1.5f;
    
    focusOn(center, distance);
}

void CameraController::focusOn(const QVector3D& point, float distance) {
    if (m_mode == Orbit) {
        animateTo(point + QVector3D(0, 0, distance), point);
    } else {
        animateTo(point + QVector3D(0, 0, distance), point);
    }
}

void CameraController::frameAll(const QVector<BoundingBox>& bounds) {
    if (bounds.isEmpty()) {
        return;
    }
    
    BoundingBox combinedBounds = bounds.first();
    for (const auto& box : bounds) {
        combinedBounds.min = QVector3D(
            qMin(combinedBounds.min.x(), box.min.x()),
            qMin(combinedBounds.min.y(), box.min.y()),
            qMin(combinedBounds.min.z(), box.min.z())
        );
        combinedBounds.max = QVector3D(
            qMax(combinedBounds.max.x(), box.max.x()),
            qMax(combinedBounds.max.y(), box.max.y()),
            qMax(combinedBounds.max.z(), box.max.z())
        );
    }
    
    focusOn(combinedBounds);
}

void CameraController::frameSelected(const QVector<BoundingBox>& bounds) {
    frameAll(bounds);
}

QVector3D CameraController::screenToWorldRay(const QPoint& screenPos, int screenWidth, int screenHeight) const {
    // Convert screen coordinates to normalized device coordinates
    float x = (2.0f * screenPos.x()) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * screenPos.y()) / screenHeight;
    
    // Create ray in clip space
    QVector4D rayClip(x, y, -1.0f, 1.0f);
    
    // Transform to eye space
    QMatrix4x4 projMatrix = getProjectionMatrix();
    QVector4D rayEye = projMatrix.inverted() * rayClip;
    rayEye = QVector4D(rayEye.x(), rayEye.y(), -1.0f, 0.0f);
    
    // Transform to world space
    QMatrix4x4 viewMatrix = getViewMatrix();
    QVector4D rayWorld = viewMatrix.inverted() * rayEye;
    
    return QVector3D(rayWorld.x(), rayWorld.y(), rayWorld.z()).normalized();
}

QVector3D CameraController::screenToWorldPoint(const QPoint& screenPos, int screenWidth, int screenHeight, float depth) const {
    QVector3D ray = screenToWorldRay(screenPos, screenWidth, screenHeight);
    return m_position + ray * depth;
}

QPoint CameraController::worldToScreen(const QVector3D& worldPos, int screenWidth, int screenHeight) const {
    QMatrix4x4 mvp = getViewProjectionMatrix();
    QVector4D clipPos = mvp * QVector4D(worldPos, 1.0f);
    
    if (clipPos.w() == 0.0f) {
        return QPoint(-1, -1); // Invalid
    }
    
    // Perspective divide
    QVector3D ndc = QVector3D(clipPos.x(), clipPos.y(), clipPos.z()) / clipPos.w();
    
    // Convert to screen coordinates
    int x = static_cast<int>((ndc.x() + 1.0f) * 0.5f * screenWidth);
    int y = static_cast<int>((1.0f - ndc.y()) * 0.5f * screenHeight);
    
    return QPoint(x, y);
}

void CameraController::animateTo(const QVector3D& targetPosition, const QVector3D& targetTarget, float duration) {
    m_animStartPosition = m_position;
    m_animStartTarget = m_target;
    m_animEndPosition = targetPosition;
    m_animEndTarget = targetTarget;
    m_animDuration = duration;
    m_animProgress = 0.0f;
    m_isAnimating = true;
    
    m_animTimer->start();
}

void CameraController::stopAnimation() {
    if (m_isAnimating) {
        m_isAnimating = false;
        m_animTimer->stop();
    }
}

bool CameraController::isAnimating() const {
    return m_isAnimating;
}

void CameraController::updateAnimation() {
    if (!m_isAnimating) {
        return;
    }
    
    m_animProgress += m_animTimer->interval() / 1000.0f / m_animDuration;
    
    if (m_animProgress >= 1.0f) {
        m_animProgress = 1.0f;
        m_isAnimating = false;
        m_animTimer->stop();
    }
    
    // Smooth interpolation using ease-in-out
    float t = m_animProgress;
    t = t * t * (3.0f - 2.0f * t); // Smoothstep
    
    m_position = MathUtils::lerp(m_animStartPosition, m_animEndPosition, t);
    m_target = MathUtils::lerp(m_animStartTarget, m_animEndTarget, t);
    
    updateCameraVectors();
    emit cameraChanged();
}

void CameraController::updateCameraVectors() {
    switch (m_mode) {
        case Orbit:
            updateOrbitCamera();
            break;
        case Fly:
            updateFlyCamera();
            break;
        case Walk:
            updateWalkCamera();
            break;
    }
}

void CameraController::constrainPitch() {
    m_pitch = qBound(m_minPitch, m_pitch, m_maxPitch);
}

void CameraController::updateOrbitCamera() {
    // Calculate position based on spherical coordinates
    float yawRad = MathUtils::degreesToRadians(m_yaw);
    float pitchRad = MathUtils::degreesToRadians(m_pitch);
    
    QVector3D direction;
    direction.setX(qCos(yawRad) * qCos(pitchRad));
    direction.setY(qSin(pitchRad));
    direction.setZ(qSin(yawRad) * qCos(pitchRad));
    
    m_position = m_target + direction * m_distance;
    m_forward = -direction;
    m_right = QVector3D::crossProduct(m_forward, m_worldUp).normalized();
    m_up = QVector3D::crossProduct(m_right, m_forward).normalized();
}

void CameraController::updateFlyCamera() {
    // Calculate direction vectors from yaw and pitch
    float yawRad = MathUtils::degreesToRadians(m_yaw);
    float pitchRad = MathUtils::degreesToRadians(m_pitch);
    
    m_forward.setX(qCos(yawRad) * qCos(pitchRad));
    m_forward.setY(qSin(pitchRad));
    m_forward.setZ(qSin(yawRad) * qCos(pitchRad));
    m_forward = m_forward.normalized();
    
    m_right = QVector3D::crossProduct(m_forward, m_worldUp).normalized();
    m_up = QVector3D::crossProduct(m_right, m_forward).normalized();
    
    m_target = m_position + m_forward;
}

void CameraController::updateWalkCamera() {
    // Similar to fly camera but constrain Y movement
    float yawRad = MathUtils::degreesToRadians(m_yaw);
    
    m_forward.setX(qCos(yawRad));
    m_forward.setY(0.0f); // No vertical movement in walk mode
    m_forward.setZ(qSin(yawRad));
    m_forward = m_forward.normalized();
    
    m_right = QVector3D::crossProduct(m_forward, m_worldUp).normalized();
    m_up = m_worldUp;
    
    m_target = m_position + m_forward;
}

#include "camera_controller.moc"

