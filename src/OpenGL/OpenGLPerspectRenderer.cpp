#include <OpenGLPerspectRenderer.h>

OpenGLPerspectRenderer::OpenGLPerspectRenderer(QObject* parent): QObject(0) {
    m_log = "";
    //glFuncs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    m_pickingShader = m_basicShader = m_phongShader = 0;
    setParent(parent);
}

OpenGLPerspectRenderer::OpenGLPerspectRenderer(const OpenGLPerspectRenderer &): QObject(0) {
    m_log = "";
    //glFuncs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    m_basicShader = m_phongShader = 0;
}

bool OpenGLPerspectRenderer::hasErrorLog() {
    return m_log != "";
}

QString OpenGLPerspectRenderer::errorLog() {
    QString tmp = m_log;
    m_log = "";
    return tmp;
}


bool OpenGLPerspectRenderer::reloadShaders() {
    if (m_pickingShader) {
        delete m_pickingShader;
        m_pickingShader = 0;
    }
    if (m_basicShader) {
        delete m_basicShader;
        m_basicShader = 0;
    }
    if (m_phongShader) {
        delete m_phongShader;
        m_phongShader = 0;
    }

    m_pickingShader = loadShaderFromFile(":/resources/shaders/picking.vert", ":/resources/shaders/picking.frag");
    m_basicShader = loadShaderFromFile(":/resources/shaders/basic.vert", ":/resources/shaders/basic.frag");
    m_phongShader = loadShaderFromFile(":/resources/shaders/phong.vert", ":/resources/shaders/phong.frag");

    if (m_phongShader) {
        m_phongShader->bind();
        m_phongShader->setUniformValue("diffuseMap", 0);
        m_phongShader->setUniformValue("specularMap", 1);
        m_phongShader->setUniformValue("bumpMap", 2);
        m_phongShader->release();
    }

    return m_pickingShader && m_basicShader && m_phongShader;
}

void OpenGLPerspectRenderer::render(OpenGLPerspectScene* openGLScene) {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (m_basicShader) {
        m_basicShader->bind();
        openGLScene->renderGridlines();
        openGLScene->renderLights();
        m_basicShader->release();
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (m_phongShader) {
        m_phongShader->bind();
        openGLScene->renderModels();
        m_phongShader->release();
    }
    if (m_basicShader) {
        m_basicShader->bind();
        openGLScene->renderAxis();
        m_basicShader->release();
    }
}

QOpenGLShaderProgram * OpenGLPerspectRenderer::loadShaderFromFile(
    QString vertexShaderFilePath,
    QString fragmentShaderFilePath,
    QString geometryShaderFilePath) {
    QFile glslDefineFile(":/resources/shaders/define.glsl");
    QFile glslUBOFile(":/resources/shaders/ubo.glsl");
    if (!glslDefineFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_log += "Failed to load glsl definitions: " + glslDefineFile.errorString();
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "Failed to load glsl definitions:" << glslDefineFile.errorString();
        return 0;
    }
    if (!glslUBOFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_log += "Failed to load glsl UBOs: " + glslUBOFile.errorString();
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "Failed to load glsl UBOs:" << glslUBOFile.errorString();
        return 0;
    }

    QFile vertexShaderFile(vertexShaderFilePath);
    QFile fragmentShaderFile(fragmentShaderFilePath);
    QFile geometryShaderFile(geometryShaderFilePath);
    if (!vertexShaderFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_log += "Failed to open file " + vertexShaderFilePath;
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "Failed to open file" + vertexShaderFilePath;
        return 0;
    }
    if (!fragmentShaderFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_log += "Failed to open file " + fragmentShaderFilePath;
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "Failed to open file" + fragmentShaderFilePath;
        return 0;
    }
    if (geometryShaderFilePath != "" && !geometryShaderFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_log += "Failed to open file " + geometryShaderFilePath;
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "Failed to open file" + geometryShaderFilePath;
        return 0;
    }

    QByteArray glslDefineCode = glslDefineFile.readAll();
    QByteArray glslUBOCode = glslUBOFile.readAll();

    QByteArray vertexShaderCode = "#version 330 core\n"
        + glslDefineCode + "\n"
        + glslUBOCode + "\n"
        + vertexShaderFile.readAll();
    QByteArray fragmentShaderCode = "#version 330 core\n"
        + glslDefineCode + "\n"
        + glslUBOCode + "\n"
        + fragmentShaderFile.readAll();
    QByteArray geometryShaderCode = "#version 330 core\n"
        + glslDefineCode + "\n"
        + glslUBOCode + "\n"
        + (geometryShaderFilePath != "" ? geometryShaderFile.readAll() : "");

    QOpenGLShaderProgram* shader = new QOpenGLShaderProgram(this);
    if (!shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderCode)) {
        m_log += "Failed to compile vertex shader: " + shader->log();
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "Failed to compile vertex shader:" + shader->log();
        return 0;
    }
    if (!shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderCode)) {
        m_log += "Failed to compile fragment shader: " + shader->log();
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "Failed to compile fragment shader:" + shader->log();
        return 0;
    }
    if (geometryShaderFilePath != "" && !shader->addShaderFromSourceCode(QOpenGLShader::Geometry, geometryShaderCode)) {
        m_log += "Failed to compile geometry shader: " + shader->log();
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "Failed to compile geometry shader:" + shader->log();
        return 0;
    }
    if (!shader->link()) {
        m_log += "Failed to link shaders: " + shader->log();
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "Failed to link shaders:" + shader->log();
        return 0;
    }
    //glFuncs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    //GLuint indx = glFuncs->glGetUniformBlockIndex(shader->programId(), "CameraInfo");
    //glFuncs->glUniformBlockBinding(shader->programId(), indx, CAMERA_INFO_BINDING_POINT);
    //indx = glFuncs->glGetUniformBlockIndex(shader->programId(), "ModelInfo");
    //glFuncs->glUniformBlockBinding(shader->programId(), indx, MODEL_INFO_BINDING_POINT);
    //indx = glFuncs->glGetUniformBlockIndex(shader->programId(), "MaterialInfo");
    //glFuncs->glUniformBlockBinding(shader->programId(), indx, MATERIAL_INFO_BINDING_POINT);
    //indx = glFuncs->glGetUniformBlockIndex(shader->programId(), "LightInfo");
    //glFuncs->glUniformBlockBinding(shader->programId(), indx, LIGHT_INFO_BINDING_POINT);
    OpenGLUniformBufferObject::bindUniformBlock(shader);
    return shader;
}
