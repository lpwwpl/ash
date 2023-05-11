#pragma once

#include <Mesh.h>

using namespace std;


enum ModelType {
    eM_SPHERE,
    eM_BOX,
    eM_CYLINDER,
    eM_CAPSULE,
    eM_MESH,
    eM_Robot
};

class Model: public AbstractEntity {
    Q_OBJECT

public:
    Model(QObject * parent = 0);
    Model(const Model& model);
    Model(QString name, QString file_name, QString mat_name, ModelType type_, QMatrix4x4 joint_transmat, QMatrix4x4 vis_transmat, QVector3D scale_, QVector3D axis_, Model* parent)
    {
        setObjectName(name);
        m_fileName = file_name;
        m_matiral_name = mat_name;
        m_joint_transmat = joint_transmat;
        m_scaling = scale_;
        m_parent = parent;
        m_axis = axis_;
        m_type = type_;
        m_vis_transmat = vis_transmat;

        //type = type_;        
    }
    ~Model();

    bool addChildMesh(Mesh* mesh);
    bool addChildModel(Model* model);

    bool removeChildMesh(QObject* mesh, bool recursive);
    bool removeChildModel(QObject* model, bool recursive);

    void dumpObjectInfo(int level = 0) override;
    void dumpObjectTree(int level = 0) override;

    bool isGizmo() const override;
    bool isLight() const override;
    bool isMesh() const override;
    bool isModel() const override;

    QVector3D centerOfMass() const;
    float mass() const;

    Mesh* assemble() const;

    const QVector<Mesh*> & childMeshes() const;
    const QVector<Model*> & childModels() const;



public slots:
    void reverseNormals();
    void reverseTangents();
    void reverseBitangents();

signals:
    void childMeshAdded(Mesh* mesh);
    void childMeshRemoved(QObject* object);
    void childModelAdded(Model* model);
    void childModelRemoved(QObject* object);

protected:
    void childEvent(QChildEvent *event) override;

public:
    QMatrix4x4 m_joint_transmat;
    QMatrix4x4 m_vis_transmat;
    QVector3D m_axis;
    Model* m_parent;
    QString m_fileName;
    QVector<Mesh*> m_childMeshes;
    QVector<Model*> m_childModels;
    QMatrix4x4 m_transmat;
    QString m_matiral_name;
    ModelType m_type;
};
