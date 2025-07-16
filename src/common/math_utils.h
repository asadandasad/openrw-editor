#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>
#include <QtMath>

namespace MathUtils {

// Constants
constexpr float PI = 3.14159265359f;
constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / PI;

// Angle conversions
inline float degreesToRadians(float degrees) {
    return degrees * DEG_TO_RAD;
}

inline float radiansToDegrees(float radians) {
    return radians * RAD_TO_DEG;
}

// Vector utilities
inline float distance(const QVector3D& a, const QVector3D& b) {
    return (a - b).length();
}

inline float distanceSquared(const QVector3D& a, const QVector3D& b) {
    return (a - b).lengthSquared();
}

inline QVector3D lerp(const QVector3D& a, const QVector3D& b, float t) {
    return a + t * (b - a);
}

inline QVector3D clamp(const QVector3D& value, const QVector3D& min, const QVector3D& max) {
    return QVector3D(
        qBound(min.x(), value.x(), max.x()),
        qBound(min.y(), value.y(), max.y()),
        qBound(min.z(), value.z(), max.z())
    );
}

// Quaternion utilities
inline QQuaternion eulerToQuaternion(float pitch, float yaw, float roll) {
    // Convert Euler angles (in degrees) to quaternion
    float pitchRad = degreesToRadians(pitch) * 0.5f;
    float yawRad = degreesToRadians(yaw) * 0.5f;
    float rollRad = degreesToRadians(roll) * 0.5f;
    
    float cy = qCos(yawRad);
    float sy = qSin(yawRad);
    float cp = qCos(pitchRad);
    float sp = qSin(pitchRad);
    float cr = qCos(rollRad);
    float sr = qSin(rollRad);
    
    return QQuaternion(
        cr * cp * cy + sr * sp * sy,  // w
        sr * cp * cy - cr * sp * sy,  // x
        cr * sp * cy + sr * cp * sy,  // y
        cr * cp * sy - sr * sp * cy   // z
    );
}

inline QVector3D quaternionToEuler(const QQuaternion& q) {
    // Convert quaternion to Euler angles (in degrees)
    float w = q.scalar();
    float x = q.x();
    float y = q.y();
    float z = q.z();
    
    // Roll (x-axis rotation)
    float sinr_cosp = 2 * (w * x + y * z);
    float cosr_cosp = 1 - 2 * (x * x + y * y);
    float roll = qAtan2(sinr_cosp, cosr_cosp);
    
    // Pitch (y-axis rotation)
    float sinp = 2 * (w * y - z * x);
    float pitch;
    if (qAbs(sinp) >= 1)
        pitch = qCopySign(PI / 2, sinp); // Use 90 degrees if out of range
    else
        pitch = qAsin(sinp);
    
    // Yaw (z-axis rotation)
    float siny_cosp = 2 * (w * z + x * y);
    float cosy_cosp = 1 - 2 * (y * y + z * z);
    float yaw = qAtan2(siny_cosp, cosy_cosp);
    
    return QVector3D(
        radiansToDegrees(pitch),
        radiansToDegrees(yaw),
        radiansToDegrees(roll)
    );
}

// Matrix utilities
inline QMatrix4x4 lookAt(const QVector3D& eye, const QVector3D& center, const QVector3D& up) {
    QMatrix4x4 matrix;
    matrix.lookAt(eye, center, up);
    return matrix;
}

inline QMatrix4x4 perspective(float fovy, float aspect, float nearPlane, float farPlane) {
    QMatrix4x4 matrix;
    matrix.perspective(fovy, aspect, nearPlane, farPlane);
    return matrix;
}

// Intersection tests
inline bool rayIntersectsSphere(const QVector3D& rayOrigin, const QVector3D& rayDirection,
                               const QVector3D& sphereCenter, float sphereRadius,
                               float& distance) {
    QVector3D oc = rayOrigin - sphereCenter;
    float a = QVector3D::dotProduct(rayDirection, rayDirection);
    float b = 2.0f * QVector3D::dotProduct(oc, rayDirection);
    float c = QVector3D::dotProduct(oc, oc) - sphereRadius * sphereRadius;
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) {
        return false;
    }
    
    distance = (-b - qSqrt(discriminant)) / (2.0f * a);
    return distance >= 0;
}

inline bool rayIntersectsBox(const QVector3D& rayOrigin, const QVector3D& rayDirection,
                            const QVector3D& boxMin, const QVector3D& boxMax,
                            float& distance) {
    QVector3D invDir = QVector3D(1.0f / rayDirection.x(), 1.0f / rayDirection.y(), 1.0f / rayDirection.z());
    QVector3D t1 = QVector3D((boxMin.x() - rayOrigin.x()) * invDir.x(),
                            (boxMin.y() - rayOrigin.y()) * invDir.y(),
                            (boxMin.z() - rayOrigin.z()) * invDir.z());
    QVector3D t2 = QVector3D((boxMax.x() - rayOrigin.x()) * invDir.x(),
                            (boxMax.y() - rayOrigin.y()) * invDir.y(),
                            (boxMax.z() - rayOrigin.z()) * invDir.z());
    
    QVector3D tMin = QVector3D(qMin(t1.x(), t2.x()), qMin(t1.y(), t2.y()), qMin(t1.z(), t2.z()));
    QVector3D tMax = QVector3D(qMax(t1.x(), t2.x()), qMax(t1.y(), t2.y()), qMax(t1.z(), t2.z()));
    
    float tNear = qMax(qMax(tMin.x(), tMin.y()), tMin.z());
    float tFar = qMin(qMin(tMax.x(), tMax.y()), tMax.z());
    
    if (tNear > tFar || tFar < 0) {
        return false;
    }
    
    distance = tNear >= 0 ? tNear : tFar;
    return true;
}

// Utility functions for snapping
inline float snapToGrid(float value, float gridSize) {
    return qRound(value / gridSize) * gridSize;
}

inline QVector3D snapToGrid(const QVector3D& position, float gridSize) {
    return QVector3D(
        snapToGrid(position.x(), gridSize),
        snapToGrid(position.y(), gridSize),
        snapToGrid(position.z(), gridSize)
    );
}

inline float snapToAngle(float angle, float angleStep) {
    return qRound(angle / angleStep) * angleStep;
}

} // namespace MathUtils

#endif // MATH_UTILS_H

