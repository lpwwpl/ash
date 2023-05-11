#pragma once
#ifndef VRMLLOADER_H
#define VRMLLOADER_H
#include "../include/urdf/umodel.h"
#include "Model.h"
#include "TextureLoader.h"
#include <QDir>

using namespace urdf;

class VrmlLoader {
public:
    VrmlLoader();

    Model* loadModelFromFile(QString filePath);
    Model* loadModel(std::shared_ptr<UrdfModel> urdfModel);
    //Mesh* loadMeshFromFile(QString filePath);


    bool hasErrorLog();
    QString errorLog();

private:
    std::shared_ptr<UrdfModel> model;

    QDir m_dir;
    QString m_log;
    TextureLoader textureLoader;

};
#endif