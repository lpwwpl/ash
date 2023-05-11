#pragma once
#ifndef OPENGLSCENE_H
#define OPENGLSCENE_H
#include <Scene.h>
#include <OpenGLMesh.h>
#include <OpenGLUniformBufferObject.h>
#include <ShaderStruct.h>
class OpenGLScene: public QObject {
    Q_OBJECT

public:
    OpenGLScene(Scene* scene);

    Scene* host() const;

    OpenGLMesh* pick(uint32_t pickingID);

    void renderAxis();
    void renderGridlines();
    void renderLights();
    void renderModels(bool pickingPass = false);

    void commitCameraInfo();
    void commitLightInfo();

protected:
    void childEvent(QChildEvent *event) override;

private:
    Scene* m_host;
    QVector<OpenGLMesh*> m_gizmoMeshes, m_gridlineMeshes, m_lightMeshes, m_normalMeshes;
    //OpenGLUniformBufferObject *m_cameraInfo, *m_lightInfo;
    //ShaderlightInfo shaderlightInfo;
    static OpenGLUniformBufferObject* m_cameraInfo, * m_lightInfo;

    ShaderAxisInfo shaderAxisInfo;
    ShaderCameraInfo shaderCameraInfo;

private slots:
    void gizmoAdded(AbstractGizmo* gizmo);
    void gridlineAdded(Gridline* gridline);
    void lightAdded(AbstractLight* light);
    void modelAdded(Model* model);
    void meshAdded(Mesh* mesh);
    void hostDestroyed(QObject* host);
};
#endif