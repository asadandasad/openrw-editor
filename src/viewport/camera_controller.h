#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "types.h"
#include "math_utils.h"
#include <QObject>
#include <QVector3D>
#include <QMatrix4x4>
#include <QPoint>

// Camera controller for 3D viewport navigation
class CameraController : public QObject {
    Q_OBJECT
    
public:
    enum CameraMode {
        Orbit,      // Orbit around a target point
        Fly,        // Free-flying camera
        Walk        // First-person walking
    };
    
    explicit CameraController(QObject* parent = nullptr);
    
    // Camera properties
    void setPosition(const QVector3D& position);
    QVector3D getPosition() const;
    
    void setTarget(const QVector3D& target);
    QVector3D getTarget() const;
    
    void setUp(const QVector3D& up);
    QVector3D getUp() const;
    
    void setDistance(float distance);
    float getDistance() const;
    
    // Camera mode
    void setCameraMode(CameraMode mode);
    CameraMode getCameraMode() const;
    
    // Projection settings
    void setFieldOfView(float fov);
    float getFieldOfView() const;
    
    void setNearPlane(float nearPlane);
    float getNearPlane() const;
    
    void setFarPlane(float farPlane);
    float getFarPlane() const;
    
    void setAspectRatio(float aspectRatio);
    float getAspectRatio() const;
    
    // Movement settings
    void setMovementSpeed(float speed);
    float getMovementSpeed() const;
    
    void setRotationSpeed(float speed);
    float getRotationSpeed() const;
    
    void setZoomSpeed(float speed);
    float getZoomSpeed() const;
    
    // Matrix generation
    QMatrix4x4 getViewMatrix() const;
    QMatrix4x4 getProjectionMatrix() const;
    QMatrix4x4 getViewProjectionMatrix() const;
    
    // Camera direction vectors
    QVector3D getForward() const;
    QVector3D getRight() const;
    QVector3D getUpVector() const;
    
    // Input handling
    void handleMouseMove(const QPoint& delta, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
    void handleMouseWheel(int delta, Qt::KeyboardModifiers modifiers);
    void handleKeyPress(Qt::Key key, Qt::KeyboardModifiers modifiers);
    void handleKeyRelease(Qt::Key key);
    
    // Camera operations
    void reset();
    void lookAt(const QVector3D& position, const QVector3D& target, const QVector3D& up = QVector3D(0, 1, 0));
    void orbitAround(const QVector3D& target, float distance);
    void focusOn(const BoundingBox& bounds);
    void focusOn(const QVector3D& point, float distance = 10.0f);
    
    // Frame camera to fit objects
    void frameAll(const QVector<BoundingBox>& bounds);
    void frameSelected(const QVector<BoundingBox>& bounds);
    
    // Ray casting
    QVector3D screenToWorldRay(const QPoint& screenPos, int screenWidth, int screenHeight) const;
    QVector3D screenToWorldPoint(const QPoint& screenPos, int screenWidth, int screenHeight, float depth) const;
    QPoint worldToScreen(const QVector3D& worldPos, int screenWidth, int screenHeight) const;
    
    // Animation
    void animateTo(const QVector3D& targetPosition, const QVector3D& targetTarget, float duration = 1.0f);
    void stopAnimation();
    bool isAnimating() const;
    
signals:
    void cameraChanged();
    void positionChanged(const QVector3D& position);
    void targetChanged(const QVector3D& target);
    
private slots:
    void updateAnimation();
    
private:
    void updateCameraVectors();
    void constrainPitch();
    void updateOrbitCamera();
    void updateFlyCamera();
    void updateWalkCamera();
    
    // Camera state
    CameraMode m_mode;
    QVector3D m_position;
    QVector3D m_target;
    QVector3D m_up;
    QVector3D m_worldUp;
    
    // Orbit mode specific
    float m_distance;
    float m_yaw;
    float m_pitch;
    
    // Fly/Walk mode specific
    QVector3D m_forward;
    QVector3D m_right;
    
    // Projection settings
    float m_fieldOfView;
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;
    
    // Movement settings
    float m_movementSpeed;
    float m_rotationSpeed;
    float m_zoomSpeed;
    
    // Input state
    QSet<Qt::Key> m_pressedKeys;
    bool m_isFirstMouse;
    QPoint m_lastMousePos;
    
    // Animation
    bool m_isAnimating;
    QVector3D m_animStartPosition;
    QVector3D m_animStartTarget;
    QVector3D m_animEndPosition;
    QVector3D m_animEndTarget;
    float m_animProgress;
    float m_animDuration;
    QTimer* m_animTimer;
    
    // Constraints
    float m_minDistance;
    float m_maxDistance;
    float m_minPitch;
    float m_maxPitch;
};

#endif // CAMERA_CONTROLLER_H

