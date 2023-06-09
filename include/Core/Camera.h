#pragma once

#include <Common.h>

enum ECameraType
{
    ePespectiveCamera,
    eSceneCamera,
};
class Camera: public QObject {
    Q_OBJECT

public:
    Camera(ECameraType,QObject* parent = 0);
    Camera(QVector3D position, QVector3D direction, ECameraType,QObject* parent = 0);
    Camera(const Camera& camera);
    ~Camera();

    void moveForward(float shift);
    void moveRight(float shift);
    void moveUp(float shift);
    void turnLeft(float angle);
    void lookUp(float angle);

    void dumpObjectInfo(int level = 0);
    void dumpObjectTree(int level = 0);

    QVector3D position() const;
    QVector3D direction() const;

    float movingSpeed() const;
    float fieldOfView() const;
    float aspectRatio() const;
    float nearPlane() const;
    float farPlane() const;

    void SetCameraType(ECameraType type) { m_c_type = type; }
    ECameraType GetCameraType() { return m_c_type; }
    QMatrix4x4 projectionMatrix() const;
    QMatrix4x4 viewMatrix() const;

public slots:
    void reset();
    void setMovingSpeed(float movingSpeed);
    void setFieldOfView(float fieldOfView);
    void setAspectRatio(float aspectRatio);
    void setNearPlane(float nearPlane);
    void setFarPlane(float farPlane);
    void setPosition(QVector3D position);
    void setDirection(QVector3D direction);

signals:
    void movingSpeedChanged(float movingSpeed);
    void fieldOfViewChanged(float fieldOfView);
    void aspectRatioChanged(float aspectRatio);
    void nearPlaneChanged(float nearPlane);
    void farPlaneChanged(float farPlane);
    void positionChanged(QVector3D position);
    void directionChanged(QVector3D direction);

private:
    float m_movingSpeed, m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane;
    QVector3D m_position, m_direction, m_up;
    ECameraType m_c_type;
    bool m_isEnable;
    void setUpVector();
};
