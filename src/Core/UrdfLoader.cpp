
#include "UrdfLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/importerdesc.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stlloader.h"
Model* UrdfLoader::loadModelFromFile(QString filePath)
{
    if (filePath.length() == 0) {
        m_log += "Filepath is empty.";
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "Failed to load model: filepath is empty";
        return 0;
    }
   

    if (log_level >= LOG_LEVEL_INFO)
        dout << "Loading" << filePath;

    if (filePath[0] == ':') { // qrc
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            if (log_level >= LOG_LEVEL_ERROR)
                dout << "FATAL: failed to open internal file" << filePath;
            exit(-1);
        }

        QByteArray bytes = file.readAll();
    }
    else {
        m_dir = QFileInfo(filePath).absoluteDir();
        ifstream ifile(filePath.toStdString());
        ostringstream buf;
        char ch;
        while (ifile.get(ch))
            buf.put(ch);
        std::string urdf_str = buf.str();
        m_urdfModel = UrdfModel::fromUrdfStr(urdf_str);
    }


    Model* model = loadModel(m_urdfModel);
    model->setObjectName(QFileInfo(filePath).baseName());

    return model;

}

Mesh* UrdfLoader::loadMesh(const aiMesh* aiMeshPtr) {
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

Material* UrdfLoader::loadMaterial(const aiMaterial* aiMaterialPtr) {
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

Model* UrdfLoader::loadModel(const aiNode* aiNodePtr) 
{
    Model* model = new Model;
    model->setObjectName(aiNodePtr->mName.length ? aiNodePtr->mName.C_Str() : "Untitled");
    for (uint32_t i = 0; i < aiNodePtr->mNumMeshes; i++)
    {
        Mesh* mesh = loadMesh(m_aiScenePtr->mMeshes[aiNodePtr->mMeshes[i]]);

        model->addChildMesh(mesh);
        
    }
       
    for (uint32_t i = 0; i < aiNodePtr->mNumChildren; i++)
        model->addChildModel(loadModel(aiNodePtr->mChildren[i]));

    QVector3D center = model->centerOfMass();

    //for (int i = 0; i < model->childMeshes().size(); i++)
    //    model->childMeshes()[i]->translate(-center);
    //for (int i = 0; i < model->childModels().size(); i++)
    //    model->childModels()[i]->translate(-center);

    //model->translate(center);

    return model;
}

Model* UrdfLoader::loadModel(std::shared_ptr<Geometry> geo)
{
    Model* model = new Model;
    try {
        UMesh* umesh = dynamic_cast<UMesh*>(geo.get());
        QString filePath = QString("%1/%2").arg(m_dir.absolutePath()).arg(umesh->filename.c_str());

        Assimp::Importer importer;

        unsigned int flags =
            aiProcess_Triangulate |
            aiProcess_CalcTangentSpace |
            aiProcess_GenSmoothNormals |
            aiProcess_JoinIdenticalVertices |
            aiProcess_OptimizeGraph |
            aiProcess_GenUVCoords;

        m_aiScenePtr = importer.ReadFile(filePath.toStdString(), flags);

        if (!m_aiScenePtr || !m_aiScenePtr->mRootNode || m_aiScenePtr->mFlags == AI_SCENE_FLAGS_INCOMPLETE) {
            m_log += importer.GetErrorString();
            if (log_level >= LOG_LEVEL_ERROR)
                dout << importer.GetErrorString();
            //        return 0;
        }

        model = loadModel(m_aiScenePtr->mRootNode);
        QFileInfo fileInfo(filePath);
        model->setObjectName(fileInfo.baseName());
        for (int i = 0; i < model->childMeshes().size(); i++)
        {
            Mesh* mesh = model->childMeshes()[i];
            if (mesh->objectName() == "Untitled")
                model->childMeshes()[i]->setObjectName(model->objectName());
        }

    } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
    }
    return model;
}
Mesh* UrdfLoader::loadMesh(std::shared_ptr<Geometry> geo)
{
    Mesh* mesh = NULL;
    if (geo->type == MESH)
    {
        UMesh* umesh = dynamic_cast<UMesh*>(geo.get());

        try {
            stlloader::Mesh stl_mesh;
            QString filePath = QString("%1/%2").arg(m_dir.absolutePath()).arg(umesh->filename.c_str());

            //Assimp::Importer importer;

            stlloader::parse_file(filePath.toStdString().c_str(), stl_mesh);
            std::vector<stlloader::Facet> facets = stl_mesh.facets;
            mesh = new Mesh;
            map<std::string, std::pair<Vertex,uint>> vertex_map;

            uint value = 0;
            for (size_t itri = 0; itri < facets.size(); ++itri) {


                stlloader::Normal normal = facets[itri].normal;


                for (uint i = 0; i < 3; i++)
                {
                    Vertex vertex;
                    stlloader::Vertex vertex_stlloader = facets[itri].vertices[i];

                    vertex.position = QVector3D(vertex_stlloader.x, vertex_stlloader.y, vertex_stlloader.z);
                    vertex.normal = QVector3D(normal.x, normal.y, normal.z);

                    uint indice = 0;
                    QString v_str = QString("%1_%2_%3").arg(vertex.position.x()).arg(vertex.position.y()).arg(vertex.position.z());

                    if (vertex_map.find(v_str.toStdString()) == vertex_map.end())
                    {   
                        vertex_map.emplace(v_str.toStdString(), std::make_pair(vertex,value));
                        indice = value;
                        value++;
                    }
                    else
                    {
                        indice = vertex_map[v_str.toStdString()].second;
                    }
                    mesh->m_vertices.push_back(vertex);
                    mesh->m_indices.push_back(indice);
                    //mesh->m_indices.push_back(facets[itri].mIndices[i]);
                }
            }



            QVector3D center = mesh->centerOfMass();

            for (int i = 0; i < mesh->m_vertices.size(); i++)
                mesh->m_vertices[i].position -= center;

            mesh->m_position = center;

            QFileInfo fileInfo(filePath);
            mesh->setObjectName(fileInfo.baseName());
            //model->setObjectName(mesh->objectName().size() > 0 ? mesh->objectName() : "Untitled");
            //model->addChildMesh(mesh);
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }

    }
    return mesh;
}

QMatrix4x4 UrdfLoader::calTransMat(Model* link)
{
    if (link->m_parent == nullptr)
        return link->m_joint_transmat;

    return link->m_parent->m_joint_transmat * link->m_joint_transmat;
}

QMatrix4x4 UrdfLoader::calFinalMat(shared_ptr<Model> link)
{
    //QMatrix4x4 transmat = QMatrix4x4();
    //transmat.scale(link->m_scaling);
    //transmat = transmat
    //transmat = meshes[i]->vis_transmat * transmat;


    //transmat = calTransMat(meshes[i]) * transmat;
    //shader.setVec3("color", meshes[i]->color);
    //shader.setMat4("model", transmat);
    return QMatrix4x4();
}



void UrdfLoader::addChildLinks(std::shared_ptr<Link> link, Model* parent)
{
    double roll, pitch, yaw;
    double x, y, z;
    double roll_, pitch_, yaw_;
    double x_, y_, z_;
    double axis_x, axis_y, axis_z = 0.0f;

    std::vector<std::shared_ptr<Link>> child = link->child_links;
    for (std::vector<std::shared_ptr<Link>>::iterator it = child.begin(); it != child.end(); it++)
    {
        (*it)->parent_joint->parent_to_joint_transform.rotation.getRpy(roll, pitch, yaw);
        x = (*it)->parent_joint->parent_to_joint_transform.position.x;
        y = (*it)->parent_joint->parent_to_joint_transform.position.y;
        z = (*it)->parent_joint->parent_to_joint_transform.position.z;
        axis_x = (*it)->parent_joint->axis.x;
        axis_y = (*it)->parent_joint->axis.y;
        axis_z = (*it)->parent_joint->axis.z;

        QVector3D axis = QVector3D(axis_x, axis_y, axis_z);

        if ((*it)->visuals.size()>0)
        {
            std::shared_ptr<Geometry> geo = (*it)->visuals[0]->geometry;
            QMatrix4x4 joint_transmat = QMatrix4x4();
            joint_transmat.setToIdentity();
            joint_transmat.translate(QVector3D(x, y, z));
            joint_transmat.rotate(float(roll), QVector3D(-1, 0, 0));
            joint_transmat.rotate(float(pitch), QVector3D(0, -1, 0));
            joint_transmat.rotate(float(yaw), QVector3D(0, 0, -1));

            QMatrix4x4 vis_transmat = QMatrix4x4();
            vis_transmat.setToIdentity();
            (*it)->visuals[0]->origin.rotation.getRpy(roll_, pitch_, yaw_);
            x_ = (*it)->visuals[0]->origin.position.x;
            y_ = (*it)->visuals[0]->origin.position.y;
            z_ = (*it)->visuals[0]->origin.position.z;
            vis_transmat.translate(QVector3D(x_, y_, z_));
            vis_transmat.rotate(float(roll_), QVector3D(1, 0, 0));
            vis_transmat.rotate(float(pitch_), QVector3D(0, 1, 0));
            vis_transmat.rotate(float(yaw_), QVector3D(0, 0, 1));

            QString mat_name;
            if (geo->type == MESH)
            {
                std::shared_ptr<UMesh> m = dynamic_pointer_cast<UMesh> (geo);
              
                QString fileName = QString("%1/%2").arg(m_dir.absolutePath()).arg(m->filename.c_str());
                QString token = QString::fromStdString(m->filename.substr(m->filename.find_last_of("/")+1,m->filename.length()));

                if ((*it)->visuals[0]->material != nullptr)
                    mat_name = QString::fromStdString((*it)->visuals[0]->material->name);
                else
                    mat_name == "Black";

                QVector3D scale = QVector3D(m->scale.x, m->scale.y, m->scale.z);
                //std::shared_ptr<Model> child_model(new Model(token,fileName,mat_name,joint_transmat,vis_transmat, scale,axis, parent));

                Model* child_model = (this->loadModel(m));
                child_model->setObjectName(token);
                child_model->m_fileName = fileName;
                child_model->m_joint_transmat = joint_transmat;
                child_model->m_vis_transmat = vis_transmat;
                child_model->m_scaling = scale;
                child_model->m_axis = axis;
                child_model->m_parent = parent;
    
                QMatrix4x4 transmat = QMatrix4x4();
                //transmat.scale(QVector3D(10, 10, 10));
               
                transmat = vis_transmat * transmat;
                transmat = /*calTransMat(child_model)*/child_model->m_joint_transmat * transmat;


                //child_model->setScaling(QVector3D(10, 10, 10));
               // child_model->m_transmat = transmat;
                          
                QVector3D pos = QVector3D(transmat.row(0).w(),transmat.row(1).w() ,transmat.row(2).w());
     
                child_model->setPosition(pos);

                
                QMatrix3x3 matrix33 = QMatrix3x3();
                float* data = matrix33.data();
                *(data) = transmat.row(0).x();
                *(data+1) = transmat.row(0).y();
                *(data+2) = transmat.row(0).z();
                *(data+3) = transmat.row(1).x();
                *(data+4) = transmat.row(1).y();
                *(data+5) = transmat.row(1).z();
                *(data+6) = transmat.row(2).x();
                *(data+7) = transmat.row(2).y();
                *(data+8) = transmat.row(2).z();

                QVector3D rot = rotationMatrixToEulerAngles(matrix33);
                //matrix33.
                child_model->setRotation(rot);
                //model_map.push_back(child_model);
                model_map.emplace((*it)->name, child_model);
  
                parent->addChildModel(child_model);
                addChildLinks(*it, child_model);
            }
            else if (geo->type == SPHERE)
            {
                std::shared_ptr<Sphere> m = dynamic_pointer_cast<Sphere>(geo);
                QVector3D scale = QVector3D(m->radius, m->radius, m->radius);
                Model* child_model = (new Model("",  "", mat_name, eM_MESH,joint_transmat, vis_transmat, scale, axis, parent));

                model_map.emplace((*it)->name, child_model);
  
                parent->addChildModel(child_model);
                addChildLinks(*it, child_model);
            }
            else if (geo->type == BOX)
            {
                std::shared_ptr<Box> m = dynamic_pointer_cast<Box>(geo);
                QVector3D scale = QVector3D(m->dim.x, m->dim.y, m->dim.z);
                Model* child_model=(new Model("", "", mat_name, eM_MESH, joint_transmat, vis_transmat, scale, axis, parent));

                model_map.emplace((*it)->name,child_model);

                parent->addChildModel(child_model);
                addChildLinks(*it, child_model);
            }
            else if (geo->type == CYLINDER)
            {
                std::shared_ptr<Cylinder> m = dynamic_pointer_cast<Cylinder>(geo);
                QVector3D scale = QVector3D(m->radius, m->radius, m->length);
                Model* child_model = (new Model("",  "", mat_name, eM_MESH, joint_transmat, vis_transmat, scale, axis, parent));

                model_map.emplace((*it)->name, child_model);
     
                parent->addChildModel(child_model);
                addChildLinks(*it, child_model);
            }
        }
        else
        {
           /* std::vector<std::shared_ptr<Link>> cchild =  (*it)->child_links;
           
            child.insert(child.end(),cchild.begin(),cchild.end());*/
            addChildLinks(*it, parent);
        }
    }
}

Model* UrdfLoader::loadModel(std::shared_ptr<UrdfModel> urdfModel)
{
    std::shared_ptr<Link> rootLink = urdfModel->getRoot();
    QMatrix4x4 joint_transmat;
    joint_transmat.setToIdentity();
    QMatrix4x4 vis_transmat;
    vis_transmat.setToIdentity();
    QVector3D scale = QVector3D(1, 1, 1);
    QVector3D axis = QVector3D(0, 0, 0);
    QString matiral_name = "";
    if (rootLink->visuals.size() > 0)
        matiral_name = QString::fromStdString(rootLink->visuals[0]->material->name);

    QString name = QString::fromStdString(rootLink->name);
    Model* root_node = (new Model(name, "", matiral_name, eM_MESH,joint_transmat, vis_transmat, scale, axis, nullptr));

    addChildLinks(rootLink, root_node);

    Model* rootModel = new Model;
    rootModel->m_type = eM_Robot;
    rootModel->addChildModel(root_node);

    return rootModel;
}
