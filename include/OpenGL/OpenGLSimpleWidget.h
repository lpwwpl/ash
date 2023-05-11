#pragma once

#ifndef OPENGLSIMPLEWIDGET_H
#define OPENGLSIMPLEWIDGET_H
#include <OpenGLPerspectScene.h>
#include <OpenGLPerspectRenderer.h>
#include <qopenglwidget.h>

class OpenGLSimpleWidget: public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

public:
    OpenGLSimpleWidget();
    OpenGLSimpleWidget(OpenGLPerspectScene* openGLScene, OpenGLPerspectRenderer* renderer);

    QString rendererName();
    QString openGLVersion();
    QString shadingLanguageVersion();

    void setScene(OpenGLPerspectScene* openGLScene);
    void setRenderer(OpenGLPerspectRenderer* renderer);
    void setEnableMousePicking(bool enabled);
    void setCustomRenderingLoop(void (*customRenderingLoop)(Scene*));

protected:
    void initializeGL() override;
    void paintGL() override;

signals:
    void fpsChanged(int fps);

private:
    QPoint m_lastCursorPos;
    QTime m_lastMousePressTime;
    bool m_enableMousePicking;
    OpenGLPerspectScene* m_openGLScene;
    OpenGLPerspectRenderer* m_renderer;
    void (*m_customRenderingLoop)(Scene*);

    void configSignals();

private slots:
    void sceneDestroyed(QObject* host);

public:
    Camera* m_camera;
};
#endif