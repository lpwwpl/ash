#pragma once
#ifndef URDFLOADER_H
#define URDFLOADER_H
#include "../include/urdf/umodel.h"
#include "Model.h"
#include "TextureLoader.h"
#include <QDir>

using namespace urdf;

struct aiNode;
struct aiMesh;
struct aiMaterial;
struct aiScene;

class UrdfLoader {
public:
    UrdfLoader() { m_aiScenePtr = 0; }

    Model* loadModelFromFile(QString filePath);

    Material* loadMaterial(const aiMaterial* aiMaterialPtr);
    Model* loadModel(const aiNode* aiNodePtr);
    Mesh* loadMesh(const aiMesh* aiMeshPtr);

    Model* loadModel(std::shared_ptr<UrdfModel> urdfModel);

    Model* loadModel(std::shared_ptr<Geometry>);
    Mesh* loadMesh(std::shared_ptr<Geometry>);
    
    //Mesh* loadMeshFromFile(QString filePath);

    void addChildLinks(std::shared_ptr<Link>, Model* parent);
    QMatrix4x4 calTransMat(Model* link);
    QMatrix4x4 calFinalMat(shared_ptr<Model> link);
    //bool hasErrorLog();
    //QString errorLog();

public:
    std::shared_ptr<UrdfModel> m_urdfModel;
    map<string, Model*> model_map;
   // std::vector<Model*> model_map;
    QDir m_dir;
    QString m_log;
    TextureLoader textureLoader;
    const aiScene* m_aiScenePtr;
};
#endif