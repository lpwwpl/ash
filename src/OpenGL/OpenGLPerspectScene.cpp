#include <OpenGLPerspectScene.h>



static ShaderlightInfo shaderlightInfo_;

static ShaderCameraInfo shaderCameraInfo_;
OpenGLUniformBufferObject* OpenGLPerspectScene::m_cameraInfo_ = 0;
OpenGLUniformBufferObject* OpenGLPerspectScene::m_lightInfo_ = 0;
OpenGLPerspectScene::OpenGLPerspectScene(Scene* scene) {
    m_host = scene;
    m_cameraInfo_ = 0;
    m_lightInfo_ = 0;
    //this->gizmoAdded(m_host->transformGizmo());
    for (int i = 0; i < m_host->gridlines().size(); i++)
        this->gridlineAdded(m_host->gridlines()[i]);
    for (int i = 0; i < m_host->pointLights().size(); i++)
        this->lightAdded(m_host->pointLights()[i]);
    for (int i = 0; i < m_host->spotLights().size(); i++)
        this->lightAdded(m_host->spotLights()[i]);
    for (int i = 0; i < m_host->models().size(); i++)
        this->modelAdded(m_host->models()[i]);

    connect(m_host, SIGNAL(gridlineAdded(Gridline*)), this, SLOT(gridlineAdded(Gridline*)));
    connect(m_host, SIGNAL(lightAdded(AbstractLight*)), this, SLOT(lightAdded(AbstractLight*)));
    connect(m_host, SIGNAL(modelAdded(Model*)), this, SLOT(modelAdded(Model*)));
    connect(m_host, SIGNAL(destroyed(QObject*)), this, SLOT(hostDestroyed(QObject*)));
}

Scene* OpenGLPerspectScene::host() const {
    return m_host;
}


void OpenGLPerspectScene::renderAxis() {
    if (m_host->transformGizmo()->alwaysOnTop())
        glClear(GL_DEPTH_BUFFER_BIT);
}

void OpenGLPerspectScene::renderGridlines() {
    for (int i = 0; i < m_gridlineMeshes.size(); i++)
        m_gridlineMeshes[i]->render();
}

void OpenGLPerspectScene::renderLights() {
    for (int i = 0; i < m_lightMeshes.size(); i++) {
        m_lightMeshes[i]->setPickingID(100 + i);
        m_lightMeshes[i]->render();
    }
}

void OpenGLPerspectScene::renderModels(bool pickingPass) {
    for (int i = 0; i < m_normalMeshes.size(); i++) {
        m_normalMeshes[i]->setPickingID(1000 + i);
        m_normalMeshes[i]->render(pickingPass);
    }
}

void OpenGLPerspectScene::commitCameraInfo(Camera* camera) {
    if (!camera) return;
    memcpy(shaderCameraInfo_.projMat, camera->projectionMatrix().constData(), 64);
    memcpy(shaderCameraInfo_.viewMat, camera->viewMatrix().constData(), 64);
    shaderCameraInfo_.cameraPos = camera->position();

    if (m_cameraInfo_ == 0) {
        m_cameraInfo_ = new OpenGLUniformBufferObject;
        m_cameraInfo_->create();
        m_cameraInfo_->bind();
        m_cameraInfo_->allocate(CAMERA_INFO_BINDING_POINT, NULL, sizeof(ShaderCameraInfo));
        m_cameraInfo_->release();
    }

    m_cameraInfo_->bind();
    m_cameraInfo_->write(0, &shaderCameraInfo_, sizeof(ShaderCameraInfo));
    m_cameraInfo_->release();
}

void OpenGLPerspectScene::commitLightInfo() {
    int ambientLightNum = 0, directionalLightNum = 0, pointLightNum = 0, spotLightNum = 0;
    for (int i = 0; i < m_host->ambientLights().size(); i++)
        if (m_host->ambientLights()[i]->enabled()) {
            shaderlightInfo_.ambientLight[ambientLightNum].color = m_host->ambientLights()[i]->color() * m_host->ambientLights()[i]->intensity();
            ambientLightNum++;
        }
    for (int i = 0; i < m_host->directionalLights().size(); i++)
        if (m_host->directionalLights()[i]->enabled()) {
            shaderlightInfo_.directionalLight[directionalLightNum].color = m_host->directionalLights()[i]->color() * m_host->directionalLights()[i]->intensity();
            shaderlightInfo_.directionalLight[directionalLightNum].direction = m_host->directionalLights()[i]->direction();
            directionalLightNum++;
        }
    for (int i = 0; i < m_host->pointLights().size(); i++)
        if (m_host->pointLights()[i]->enabled()) {
            shaderlightInfo_.pointLight[pointLightNum].color = m_host->pointLights()[i]->color() * m_host->pointLights()[i]->intensity();
            shaderlightInfo_.pointLight[pointLightNum].pos = m_host->pointLights()[i]->position();
            shaderlightInfo_.pointLight[pointLightNum].attenuation[0] = m_host->pointLights()[i]->enableAttenuation();
            shaderlightInfo_.pointLight[pointLightNum].attenuation[1] = m_host->pointLights()[i]->attenuationQuadratic();
            shaderlightInfo_.pointLight[pointLightNum].attenuation[2] = m_host->pointLights()[i]->attenuationLinear();
            shaderlightInfo_.pointLight[pointLightNum].attenuation[3] = m_host->pointLights()[i]->attenuationConstant();
            pointLightNum++;
        }
    for (int i = 0; i < m_host->spotLights().size(); i++)
        if (m_host->spotLights()[i]->enabled()) {
            shaderlightInfo_.spotLight[spotLightNum].color = m_host->spotLights()[i]->color() * m_host->spotLights()[i]->intensity();
            shaderlightInfo_.spotLight[spotLightNum].pos = m_host->spotLights()[i]->position();
            shaderlightInfo_.spotLight[spotLightNum].direction = m_host->spotLights()[i]->direction();
            shaderlightInfo_.spotLight[spotLightNum].attenuation[0] = m_host->spotLights()[i]->enableAttenuation();
            shaderlightInfo_.spotLight[spotLightNum].attenuation[1] = m_host->spotLights()[i]->attenuationQuadratic();
            shaderlightInfo_.spotLight[spotLightNum].attenuation[2] = m_host->spotLights()[i]->attenuationLinear();
            shaderlightInfo_.spotLight[spotLightNum].attenuation[3] = m_host->spotLights()[i]->attenuationConstant();
            shaderlightInfo_.spotLight[spotLightNum].cutOff[0] = (float)cos(rad(m_host->spotLights()[i]->innerCutOff()));
            shaderlightInfo_.spotLight[spotLightNum].cutOff[1] = (float)cos(rad(m_host->spotLights()[i]->outerCutOff()));
            spotLightNum++;
        }

    shaderlightInfo_.ambientLightNum = ambientLightNum;
    shaderlightInfo_.directionalLightNum = directionalLightNum;
    shaderlightInfo_.pointLightNum = pointLightNum;
    shaderlightInfo_.spotLightNum = spotLightNum;

    if (m_lightInfo_ == 0) {
        m_lightInfo_ = new OpenGLUniformBufferObject;
        m_lightInfo_->create();
        m_lightInfo_->bind();
        m_lightInfo_->allocate(LIGHT_INFO_BINDING_POINT, NULL, sizeof(ShaderlightInfo));
        m_lightInfo_->release();
    }
    m_lightInfo_->bind();
    m_lightInfo_->write(0, &shaderlightInfo_, sizeof(ShaderlightInfo));
    m_lightInfo_->release();
}


void OpenGLPerspectScene::gridlineAdded(Gridline* gridline) {
    m_gridlineMeshes.push_back(new OpenGLMesh(gridline->marker(), this));
}

void OpenGLPerspectScene::lightAdded(AbstractLight* light) {
    if (light->marker())
        m_lightMeshes.push_back(new OpenGLMesh(light->marker(), this));
}

void OpenGLPerspectScene::modelAdded(Model* model) {
    connect(model, SIGNAL(childMeshAdded(Mesh*)), this, SLOT(meshAdded(Mesh*)));
    for (int i = 0; i < model->childMeshes().size(); i++)
        meshAdded(model->childMeshes()[i]);
    for (int i = 0; i < model->childModels().size(); i++)
        modelAdded(model->childModels()[i]);
}

void OpenGLPerspectScene::meshAdded(Mesh* mesh) {
    m_normalMeshes.push_back(new OpenGLMesh(mesh, this));
}

void OpenGLPerspectScene::hostDestroyed(QObject*) {
    // Commit suicide
    delete this;
}
