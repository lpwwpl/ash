#pragma once

#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H
#include <OpenGLScene.h>

class OpenGLRenderer: public QObject {
    Q_OBJECT

public:
    OpenGLRenderer(QObject* parent = 0);
    OpenGLRenderer(const OpenGLRenderer& renderer);

    bool hasErrorLog();
    QString errorLog();

    bool reloadShaders();
    void reloadFrameBuffers();

    uint32_t pickingPass(OpenGLScene* openGLScene, QPoint cursorPos);
    void render(OpenGLScene* openGLScene);
    void renderNotAxis(OpenGLScene* openGLScene);
private:
    QString m_log;
    QOpenGLShaderProgram *m_basicShader, *m_pickingShader, *m_phongShader;
    QOpenGLFramebufferObject *m_pickingPassFBO;
    //static QOpenGLFunctions_3_3_Core* m_glFuncs;
    QOpenGLShaderProgram * loadShaderFromFile(
        QString vertexShaderFilePath,
        QString fragmentShaderFilePath,
        QString geometryShaderFilePath = "");
};
#endif