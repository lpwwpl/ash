#include <MeshProperty.h>

MeshProperty::MeshProperty(Mesh * mesh, QWidget * parent): QWidget(parent) {
    m_host = mesh;

    m_visibleCheckBox = new QCheckBox("Visible", this);
    m_wireFrameModeCheckBox = new QCheckBox("WireFrame Mode", this);
    m_meshTypeTextLabel = new QLabel("Mesh Type:", this);
    m_meshTypeValueLabel = new QLabel(this);
    m_numOfVerticesTextLabel = new QLabel("Vertices:", this);
    m_numOfVerticesValueLabel = new QLabel(QString::number(m_host->vertices().size()), this);
    
    if (m_host->meshType() == Mesh::Triangle) {
        m_meshTypeValueLabel->setText("Triangle");
        m_numOfFacesTextLabel = new QLabel("Faces:", this);
        m_numOfFacesValueLabel = new QLabel(QString::number(m_host->indices().size() / 3), this);
    } else if (m_host->meshType() == Mesh::Line) {
        m_meshTypeValueLabel->setText("Line");
        m_numOfFacesTextLabel = m_numOfFacesValueLabel = 0;
    } else {
        m_meshTypeValueLabel->setText("Point");
        m_numOfFacesTextLabel = m_numOfFacesValueLabel = 0;
    }
    
    m_positionEdit = new Vector3DEdit("Position", Qt::Horizontal, "X", "Y", "Z", -inf, inf, 2, this);
    m_rotationEditSlider = new Vector3DEditSlider("Rotation", Qt::Horizontal, "X", "Y", "Z", -180.0f, 180.0f, 2, this);
    m_scalingEdit = new Vector3DEdit("Scaling", Qt::Horizontal, "X", "Y", "Z", 0.0f, inf, 2, this);
    
    m_visibleCheckBox->setChecked(m_host->visible());
    m_wireFrameModeCheckBox->setChecked(m_host->wireFrameMode());
    m_positionEdit->setValue(m_host->position());
    m_rotationEditSlider->setValue(m_host->rotation());
    m_scalingEdit->setValue(m_host->scaling());

    configLayout();
    configSignals();
}

void MeshProperty::configLayout() {
    QGridLayout * subLayout = new QGridLayout;
    subLayout->setAlignment(Qt::AlignTop);
    subLayout->setVerticalSpacing(10);
    subLayout->addWidget(m_visibleCheckBox, 0, 0, 1, 2);
    subLayout->addWidget(m_wireFrameModeCheckBox, 1, 0, 1, 2);
    subLayout->addWidget(m_meshTypeTextLabel, 2, 0);
    subLayout->addWidget(m_meshTypeValueLabel, 2, 1);
    subLayout->addWidget(m_numOfVerticesTextLabel, 3, 0);
    subLayout->addWidget(m_numOfVerticesValueLabel, 3, 1);
    if (m_numOfFacesTextLabel && m_numOfFacesValueLabel) {
        subLayout->addWidget(m_numOfFacesTextLabel, 4, 0);
        subLayout->addWidget(m_numOfFacesValueLabel, 4, 1);
    }
    subLayout->addWidget(m_positionEdit, 5, 0, 1, 2);
    subLayout->addWidget(m_rotationEditSlider, 6, 0, 1, 2);
    subLayout->addWidget(m_scalingEdit, 7, 0, 1, 2);

    setLayout(subLayout);
}

void MeshProperty::configSignals() {
    connect(m_host, SIGNAL(destroyed(QObject*)), this, SLOT(hostDestroyed(QObject*)));
    connect(m_host, SIGNAL(meshTypeChanged(int)), this, SLOT(meshTypeChanged(int)));
    connect(m_host, SIGNAL(geometryChanged(QVector<Vertex>, QVector<uint32_t>)), this, SLOT(geometryChanged(QVector<Vertex>, QVector<uint32_t>)));

    connect(m_visibleCheckBox, SIGNAL(toggled(bool)), m_wireFrameModeCheckBox, SLOT(setEnabled(bool)));
    connect(m_visibleCheckBox, SIGNAL(toggled(bool)), m_positionEdit, SLOT(setEnabled(bool)));
    connect(m_visibleCheckBox, SIGNAL(toggled(bool)), m_rotationEditSlider, SLOT(setEnabled(bool)));
    connect(m_visibleCheckBox, SIGNAL(toggled(bool)), m_scalingEdit, SLOT(setEnabled(bool)));

    connect(m_visibleCheckBox, SIGNAL(toggled(bool)), m_host, SLOT(setVisible(bool)));
    connect(m_wireFrameModeCheckBox, SIGNAL(toggled(bool)), m_host, SLOT(setWireFrameMode(bool)));
    connect(m_positionEdit, SIGNAL(valueEdited(QVector3D)), m_host, SLOT(setPosition(QVector3D)));
    connect(m_rotationEditSlider, SIGNAL(valueEdited(QVector3D)), m_host, SLOT(setRotation(QVector3D)));
    connect(m_scalingEdit, SIGNAL(valueEdited(QVector3D)), m_host, SLOT(setScaling(QVector3D)));

    connect(m_host, SIGNAL(visibleChanged(bool)), m_visibleCheckBox, SLOT(setChecked(bool)));
    connect(m_host, SIGNAL(wireFrameModeChanged(bool)), m_wireFrameModeCheckBox, SLOT(setChecked(bool)));
    connect(m_host, SIGNAL(positionChanged(QVector3D)), m_positionEdit, SLOT(setValue(QVector3D)));
    connect(m_host, SIGNAL(rotationChanged(QVector3D)), m_rotationEditSlider, SLOT(setValue(QVector3D)));
    connect(m_host, SIGNAL(scalingChanged(QVector3D)), m_scalingEdit, SLOT(setValue(QVector3D)));
}

void MeshProperty::hostDestroyed(QObject *) {
    delete this;
}

void MeshProperty::meshTypeChanged(int meshType) {
    if (meshType == Mesh::Triangle) {
        m_meshTypeValueLabel->setText("Triangle");
    } else if (meshType == Mesh::Line) {
        m_meshTypeValueLabel->setText("Line");
    } else {
        m_meshTypeValueLabel->setText("Point");
    }
}

void MeshProperty::geometryChanged(const QVector<Vertex>& vertices, const QVector<uint32_t>& indices) {
    m_numOfVerticesValueLabel->setText(QString::number(vertices.size()));
    if (m_host->meshType() == Mesh::Triangle && m_numOfFacesValueLabel)
        m_numOfFacesValueLabel->setText(QString::number(indices.size() / 3));
}

