#include <OpenGLSimpleWidget.h>
#include <ModelLoader.h>

OpenGLSimpleWidget::OpenGLSimpleWidget() {
    m_lastCursorPos = QCursor::pos();
    m_enableMousePicking = true;
    m_renderer = 0;
    m_openGLScene = 0;

    m_customRenderingLoop = 0;
    configSignals();
}

OpenGLSimpleWidget::OpenGLSimpleWidget(OpenGLPerspectScene * openGLScene, OpenGLPerspectRenderer* renderer) {
    m_lastCursorPos = QCursor::pos();
    m_enableMousePicking = true;
    m_renderer = renderer;
    m_openGLScene = openGLScene;

    m_customRenderingLoop = 0;
    configSignals();
}

QString OpenGLSimpleWidget::rendererName() {
    return isInitialized() ? QString((char*) glGetString(GL_RENDERER)) : "";
}

QString OpenGLSimpleWidget::openGLVersion() {
    return isInitialized() ? QString((char*) glGetString(GL_VERSION)) : "";
}

QString OpenGLSimpleWidget::shadingLanguageVersion() {
    return isInitialized() ? QString((char*) glGetString(GL_SHADING_LANGUAGE_VERSION)) : "";
}

void OpenGLSimpleWidget::setScene(OpenGLPerspectScene* openGLScene) {
    if (m_openGLScene)
        disconnect(m_openGLScene, 0, this, 0);
    m_openGLScene = openGLScene;
    if (m_openGLScene)
        connect(m_openGLScene, SIGNAL(destroyed(QObject*)), this, SLOT(sceneDestroyed(QObject*)));
}

void OpenGLSimpleWidget::setRenderer(OpenGLPerspectRenderer* renderer) {
    m_renderer = renderer;
    if (isInitialized() && m_renderer) {
        m_renderer->reloadShaders();
        if (m_renderer->hasErrorLog()) {
            QString log = m_renderer->errorLog();
            QMessageBox::critical(0, "Failed to load shaders", log);
            if (log_level >= LOG_LEVEL_ERROR)
                dout << log;
        }
    }
}

void OpenGLSimpleWidget::setEnableMousePicking(bool enabled) {
    m_enableMousePicking = enabled;
}

void OpenGLSimpleWidget::setCustomRenderingLoop(void (*customRenderingLoop)(Scene*)) {
    m_customRenderingLoop = customRenderingLoop;
}

void OpenGLSimpleWidget::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    if (m_renderer) {
        m_renderer->reloadShaders();
        if (m_renderer->hasErrorLog()) {
            QString log = m_renderer->errorLog();
            QMessageBox::critical(0, "Failed to load shaders", log);
            if (log_level >= LOG_LEVEL_ERROR)
                dout << log;
        }
     
    } else {
        QMessageBox::critical(0, "Failed to initialize OpenGL", "No renderer specified.");
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "No renderer specified";
    }
}

void OpenGLSimpleWidget::paintGL() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_renderer && m_openGLScene && m_openGLScene->host()->camera()) {
        if (m_customRenderingLoop)
            m_customRenderingLoop(m_openGLScene->host());

        m_openGLScene->host()->camera()->setAspectRatio(float(width()) / height());
        m_openGLScene->commitCameraInfo(m_camera);
        m_openGLScene->commitLightInfo();

        m_renderer->render(m_openGLScene);
    }
}


void OpenGLSimpleWidget::configSignals() {
    if (m_openGLScene)
        connect(m_openGLScene, SIGNAL(destroyed(QObject*)), this, SLOT(sceneDestroyed(QObject*)));
}

void OpenGLSimpleWidget::sceneDestroyed(QObject *) {
    m_openGLScene = 0;
}
