#pragma once
#ifndef OPENGLPERSPECTRENDERER_H
#define OPENGLPERSPECTRENDERER_H
#include <OpenGLPerspectScene.h>

class OpenGLPerspectRenderer: public QObject {
    Q_OBJECT

public:
    OpenGLPerspectRenderer(QObject* parent = 0);
    OpenGLPerspectRenderer(const OpenGLPerspectRenderer& renderer);

    bool hasErrorLog();
    QString errorLog();

    bool reloadShaders();
    void render(OpenGLPerspectScene* openGLScene);

private:
    QString m_log;
    QOpenGLShaderProgram *m_basicShader, *m_pickingShader, *m_phongShader;

    QOpenGLFunctions_3_3_Core* glFuncs;
    QOpenGLShaderProgram * loadShaderFromFile(
        QString vertexShaderFilePath,
        QString fragmentShaderFilePath,
        QString geometryShaderFilePath = "");
};
#endif