#pragma once

#ifndef OPENGLPERSPECTSCENE_H
#define OPENGLPERSPECTSCENE_H
#include <Scene.h>
#include <OpenGLMesh.h>
#include <OpenGLUniformBufferObject.h>
#include <ShaderStruct.h>
class OpenGLPerspectScene: public QObject {
    Q_OBJECT

public:
    OpenGLPerspectScene(Scene* scene);

    Scene* host() const;

    void renderAxis();
    void renderGridlines();
    void renderLights();
    void renderModels(bool pickingPass = false);

    void commitCameraInfo(Camera*);
    void commitLightInfo();


private:
    Scene* m_host;
    QVector<OpenGLMesh*>  m_gridlineMeshes, m_lightMeshes, m_normalMeshes;
    static OpenGLUniformBufferObject * m_cameraInfo_, * m_lightInfo_;

private slots:
    void gridlineAdded(Gridline* gridline);
    void lightAdded(AbstractLight* light);
    void modelAdded(Model* model);
    void meshAdded(Mesh* mesh);
    void hostDestroyed(QObject* host);
};
#endif