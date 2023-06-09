#include <ModelLoader.h>

// Assimp: 3D model loader
#include <assimp/Importer.hpp>
#include <assimp/importerdesc.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "UrdfLoader.h"
#include <qdebug.h>
ModelLoader::ModelLoader() {
    m_aiScenePtr = 0;
}

QMatrix4x4 ModelLoader::calTransMat(Model* link)
{
    if (link->m_parent == nullptr)
        return link->m_joint_transmat;

    return calTransMat(link->m_parent) * link->m_joint_transmat;

}



Model * ModelLoader::loadModelFromFile(QString filePath) {
    if (filePath.length() == 0) {
        m_log += "Filepath is empty.";
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "Failed to load model: filepath is empty";
        return 0;
    }

    Assimp::Importer importer;
    unsigned int flags =
        aiProcess_Triangulate |
        aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_OptimizeGraph |
        aiProcess_GenUVCoords;

    if (log_level >= LOG_LEVEL_INFO)
        dout << "Loading" << filePath;

    QFileInfo fileInfo(filePath);

    Model* model = NULL;
    qDebug() << fileInfo.suffix().toLower();
    if (filePath[0] == ':') { // qrc
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            if (log_level >= LOG_LEVEL_ERROR)
                dout << "FATAL: failed to open internal file" << filePath;
            exit(-1);
        }

        QByteArray bytes = file.readAll();
        m_aiScenePtr = importer.ReadFileFromMemory(bytes.constData(), bytes.length(), flags);

        if (!m_aiScenePtr || !m_aiScenePtr->mRootNode || m_aiScenePtr->mFlags == AI_SCENE_FLAGS_INCOMPLETE) {
            m_log += importer.GetErrorString();
            if (log_level >= LOG_LEVEL_ERROR)
                dout << importer.GetErrorString();
            //        return 0;
        }

        model = loadModel(m_aiScenePtr->mRootNode);

    } 
    else if (fileInfo.suffix().toLower().endsWith("urdf"))
    {
        UrdfLoader urdfLoader;
        model = urdfLoader.loadModelFromFile(filePath);
        //model->scale(QVector3D(10, 10, 10));
        map<string, Model*> model_map = urdfLoader.model_map;
        for (auto elem : model_map)
        {


            Model* model = elem.second;
            QVector3D axis = model->m_axis;


            if (axis.x() > 0)
            {
                //model->setRotation(QVector3D(30,0,0));
                QMatrix4x4 t_matrix = model->m_joint_transmat;
                t_matrix.rotate(30, 1, 0, 0);
                model->m_transmat = t_matrix;
            }
            else if(axis.y() > 0)
            {
               //model->setRotation(QVector3D(0, 30, 0));
                QMatrix4x4 t_matrix = model->m_joint_transmat;
                t_matrix.rotate(30, 0, 1, 0);
                model->m_transmat = t_matrix;
            }
            else if (axis.z() > 0)
            {
                //model->setRotation(QVector3D(0, 0, 30));
                QMatrix4x4 t_matrix = model->m_joint_transmat;
                t_matrix.rotate(30, 0, 0, 1);
                model->m_transmat = t_matrix;
            }

            QMatrix4x4 transmat = QMatrix4x4();
            transmat = model->m_vis_transmat * transmat;
            transmat = /*calTransMat(child_model)*/model->m_transmat * transmat;
            QVector3D pos = QVector3D(transmat.row(0).w(), transmat.row(1).w(), transmat.row(2).w());
            model->setPosition(pos);
            QMatrix3x3 matrix33 = QMatrix3x3();
            float* data = matrix33.data();
            *(data) = transmat.row(0).x();
            *(data + 1) = transmat.row(0).y();
            *(data + 2) = transmat.row(0).z();
            *(data + 3) = transmat.row(1).x();
            *(data + 4) = transmat.row(1).y();
            *(data + 5) = transmat.row(1).z();
            *(data + 6) = transmat.row(2).x();
            *(data + 7) = transmat.row(2).y();
            *(data + 8) = transmat.row(2).z();

            QVector3D rot = rotationMatrixToEulerAngles(matrix33);
            model->setRotation(rot);

        }
    }
    else {
        m_dir = QFileInfo(filePath).absoluteDir();
        m_aiScenePtr = importer.ReadFile(filePath.toStdString(), flags);

        if (!m_aiScenePtr || !m_aiScenePtr->mRootNode || m_aiScenePtr->mFlags == AI_SCENE_FLAGS_INCOMPLETE) {
            m_log += importer.GetErrorString();
            if (log_level >= LOG_LEVEL_ERROR)
                dout << importer.GetErrorString();
            //        return 0;
        }

        model = loadModel(m_aiScenePtr->mRootNode);
    }


  /*  if(model)
        model->setObjectName(QFileInfo(filePath).baseName());*/

    return model;
}

Mesh * ModelLoader::loadMeshFromFile(QString filePath) {
    Model* model = loadModelFromFile(filePath);
    Mesh* assembledMesh = model->assemble();
    delete model;
    return assembledMesh;
}

Model * ModelLoader::loadConeModel() {
    static ModelLoader * loader = new ModelLoader;
    Model* model = loader->loadModelFromFile(":/resources/shapes/Cone.obj");
    return model;
}

Model * ModelLoader::loadCubeModel() {
    static ModelLoader * loader = new ModelLoader;
    Model* model = loader->loadModelFromFile(":/resources/shapes/Cube.obj");
    return model;
}

Model * ModelLoader::loadCylinderModel() {
    static ModelLoader * loader = new ModelLoader;
    Model* model = loader->loadModelFromFile(":/resources/shapes/Cylinder.obj");
    return model;
}

Model * ModelLoader::loadPlaneModel() {
    static ModelLoader * loader = new ModelLoader;
    Model* model = loader->loadModelFromFile(":/resources/shapes/Plane.obj");
    return model;
}

Model * ModelLoader::loadSphereModel() {
    static ModelLoader * loader = new ModelLoader;
    Model* model = loader->loadModelFromFile(":/resources/shapes/Sphere.obj");
    return model;
}

bool ModelLoader::hasErrorLog() {
    return m_log.length() != 0 || textureLoader.hasErrorLog();
}

QString ModelLoader::errorLog() {
    QString tmp = m_log + textureLoader.errorLog();
    m_log = "";
    return tmp;
}

Model * ModelLoader::loadModel(const aiNode * aiNodePtr) {
    Model* model = new Model;
    model->setObjectName(aiNodePtr->mName.length ? aiNodePtr->mName.C_Str() : "Untitled");
    for (uint32_t i = 0; i < aiNodePtr->mNumMeshes; i++)
        model->addChildMesh(loadMesh(m_aiScenePtr->mMeshes[aiNodePtr->mMeshes[i]]));
    for (uint32_t i = 0; i < aiNodePtr->mNumChildren; i++)
        model->addChildModel(loadModel(aiNodePtr->mChildren[i]));

    QVector3D center = model->centerOfMass();

    for (int i = 0; i < model->childMeshes().size(); i++)
        model->childMeshes()[i]->translate(-center);
    for (int i = 0; i < model->childModels().size(); i++)
        model->childModels()[i]->translate(-center);

    model->translate(center);

    return model;
}

Mesh * ModelLoader::loadMesh(const aiMesh * aiMeshPtr) {
    Mesh* mesh = new Mesh;
    mesh->setObjectName(aiMeshPtr->mName.length ? aiMeshPtr->mName.C_Str() : "Untitled");

    for (uint32_t i = 0; i < aiMeshPtr->mNumVertices; i++) {
        Vertex vertex;
        if (aiMeshPtr->HasPositions())
            vertex.position = QVector3D(aiMeshPtr->mVertices[i].x, aiMeshPtr->mVertices[i].y, aiMeshPtr->mVertices[i].z);
        if (aiMeshPtr->HasNormals())
            vertex.normal = QVector3D(aiMeshPtr->mNormals[i].x, aiMeshPtr->mNormals[i].y, aiMeshPtr->mNormals[i].z);
        if (aiMeshPtr->HasTangentsAndBitangents()) {
            // Use left-handed tangent space
            vertex.tangent = QVector3D(aiMeshPtr->mTangents[i].x, aiMeshPtr->mTangents[i].y, aiMeshPtr->mTangents[i].z);
            vertex.bitangent = QVector3D(aiMeshPtr->mBitangents[i].x, aiMeshPtr->mBitangents[i].y, aiMeshPtr->mBitangents[i].z);

            // Gram-Schmidt process, re-orthogonalize the TBN vectors
            vertex.tangent -= QVector3D::dotProduct(vertex.tangent, vertex.normal) * vertex.normal;
            vertex.tangent.normalize();

            // Deal with mirrored texture coordinates
            if (QVector3D::dotProduct(QVector3D::crossProduct(vertex.tangent, vertex.normal), vertex.bitangent) < 0.0f)
                vertex.tangent = -vertex.tangent;
        }
        if (aiMeshPtr->HasTextureCoords(0))
            vertex.texCoords = QVector2D(aiMeshPtr->mTextureCoords[0][i].x, aiMeshPtr->mTextureCoords[0][i].y);
        mesh->m_vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < aiMeshPtr->mNumFaces; i++)
        for (uint32_t j = 0; j < 3; j++)
        {
            mesh->m_indices.push_back(aiMeshPtr->mFaces[i].mIndices[j]);
        }
           

    QVector3D center = mesh->centerOfMass();

    for (int i = 0; i < mesh->m_vertices.size(); i++)
        mesh->m_vertices[i].position -= center;

    mesh->m_position = center;
    mesh->setMaterial(loadMaterial(m_aiScenePtr->mMaterials[aiMeshPtr->mMaterialIndex]));

    return mesh;
}

Material * ModelLoader::loadMaterial(const aiMaterial * aiMaterialPtr) {
    Material* material = new Material;
    aiColor4D color; float value; aiString aiStr;

    if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_NAME, aiStr))
        material->setObjectName(aiStr.length ? aiStr.C_Str() : "Untitled");
    if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_COLOR_AMBIENT, color))
        material->setAmbient((color.r + color.g + color.b) / 3.0f);
    if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
        material->setDiffuse((color.r + color.g + color.b) / 3.0f);
        material->setColor(QVector3D(color.r, color.g, color.b) / material->diffuse());
    }
    if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_COLOR_SPECULAR, color))
        material->setSpecular((color.r + color.g + color.b) / 3.0f);
    if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_SHININESS, value) && !qFuzzyIsNull(value))
        material->setShininess(value);
    if (AI_SUCCESS == aiMaterialPtr->GetTexture(aiTextureType_DIFFUSE, 0, &aiStr)) {
        QString filePath = m_dir.absolutePath() + '/' + QString(aiStr.C_Str()).replace('\\', '/');
        material->setDiffuseTexture(textureLoader.loadFromFile(Texture::Diffuse, filePath));
    }
    if (AI_SUCCESS == aiMaterialPtr->GetTexture(aiTextureType_SPECULAR, 0, &aiStr)) {
        QString filePath = m_dir.absolutePath() + '/' + QString(aiStr.C_Str()).replace('\\', '/');
        material->setSpecularTexture(textureLoader.loadFromFile(Texture::Specular, filePath));
    }
    if (AI_SUCCESS == aiMaterialPtr->GetTexture(aiTextureType_HEIGHT, 0, &aiStr)) {
        QString filePath = m_dir.absolutePath() + '/' + QString(aiStr.C_Str()).replace('\\', '/');
        material->setBumpTexture(textureLoader.loadFromFile(Texture::Bump, filePath));
    }
    return material;
}
