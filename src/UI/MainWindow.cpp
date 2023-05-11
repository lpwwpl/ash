#include <MainWindow.h>


#include <qdockwidget.h>
MainWindow::MainWindow(QWidget * parent): QMainWindow(parent), m_host(0) {
    m_copyedObject.clear();

    m_fpsLabel = new QLabel(this);
    m_sceneTreeWidget = new SceneTreeWidget(this);
    m_openGLWindow = new OpenGLWindow;
    m_openGLWindow->setRenderer(new OpenGLRenderer);

    //m_perspectWindow = new OpenGLSimpleWidget;
    //m_perspectWindow->setRenderer(new OpenGLPerspectRenderer);

    perspect_widget1 = 0;
    perspect_widget2 = 0;

    m_propertyWidget = new QScrollArea(this);
    m_propertyWidget->setWidgetResizable(true);
    statusBar()->addPermanentWidget(m_fpsLabel);

    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    setCentralWidget(new QWidget);

    configMenu();
    configLayout();
    configSignals();

    //m_perspectWindow
    resize(1200, 720);
    fileNewScene();
    helpCheckForUpdates();
}


void MainWindow::resizeEvent(QResizeEvent *) {
    m_splitter->setSizes(QList<int>{160, width() - 160 - 300, 300});
}

bool MainWindow::askToSaveScene() {
    int answer = QMessageBox::question(this,
                                       "Unsaved scene",
                                       "Save current scene? Any unsaved changes will be lost.",
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (answer == QMessageBox::Cancel)
        return false;
    if (answer == QMessageBox::Yes)
        fileSaveScene();
    return true;
}

void MainWindow::configMenu() {
    QMenu *menuFile = menuBar()->addMenu("File");
    menuFile->addAction("New Scene", this, SLOT(fileNewScene()), QKeySequence(Qt::CTRL + Qt::Key_N));
    menuFile->addAction("Open Scene", this, SLOT(fileOpenScene()), QKeySequence(Qt::CTRL + Qt::Key_O));
    menuFile->addSeparator();
    menuFile->addAction("Import Model", this, SLOT(fileImportModel()));
    menuFile->addAction("Export Model", this, SLOT(fileExportModel()));
    menuFile->addSeparator();
    menuFile->addAction("Save Scene", this, SLOT(fileSaveScene()), QKeySequence(Qt::CTRL + Qt::Key_S));
    menuFile->addAction("Save Scene As", this, SLOT(fileSaveAsScene()));
    menuFile->addSeparator();
    menuFile->addAction("Quit", this, SLOT(fileQuit()), QKeySequence(Qt::CTRL + Qt::Key_Q));

    QMenu *menuEdit = menuBar()->addMenu("Edit");
    menuEdit->addAction("Copy", this, SLOT(editCopy()), QKeySequence(Qt::CTRL + Qt::Key_C));
    menuEdit->addAction("Paste", this, SLOT(editPaste()), QKeySequence(Qt::CTRL + Qt::Key_V));
    menuEdit->addSeparator();
    menuEdit->addAction("Remove", this, SLOT(editRemove()), QKeySequence(Qt::Key_Backspace));

    QMenu *menuCreate = menuBar()->addMenu("Create");
    menuCreate->addAction("Gridline", this, SLOT(createGridline()));

    menuCreate->addAction("PerspectCamera", this, SLOT(createPerspectCamera()));

    QMenu *menuCreateLight = menuCreate->addMenu("Light");
    menuCreateLight->addAction("Ambient Light", this, SLOT(createAmbientLight()));
    menuCreateLight->addAction("Directional Light", this, SLOT(createDirectionalLight()));
    menuCreateLight->addAction("Point Light", this, SLOT(createPointLight()));
    menuCreateLight->addAction("Spot Light", this, SLOT(createSpotLight()));

    QMenu *menuCreateBasicShapes = menuCreate->addMenu("Basic Shape");
    menuCreateBasicShapes->addAction("Cone", this, SLOT(createBasicCone()));
    menuCreateBasicShapes->addAction("Cube", this, SLOT(createBasicCube()));
    menuCreateBasicShapes->addAction("Cylinder", this, SLOT(createBasicCylinder()));
    menuCreateBasicShapes->addAction("Plane", this, SLOT(createBasicPlane()));
    menuCreateBasicShapes->addAction("Sphere", this, SLOT(createBasicSphere()));

    QMenu *menuMesh = menuBar()->addMenu("Mesh");
    menuMesh->addAction("Assign material", this, SLOT(polygonAssignMaterial()));
    menuMesh->addSeparator();
    menuMesh->addAction("Reverse Normals", this, SLOT(polygonReverseNormals()));
    menuMesh->addAction("Reverse Tangents", this, SLOT(polygonReverseTangents()));
    menuMesh->addAction("Reverse Bitangents", this, SLOT(polygonReverseBitangents()));

    QMenu *menuGizmo = menuBar()->addMenu("Gizmo");
    QAction *actionGizmoAlwaysOnTop = menuGizmo->addAction("Always On Top", this, SLOT(gizmoAlwaysOnTop(bool)));
    menuGizmo->addSeparator();
    QAction *actionGizmoTypeTranslate = menuGizmo->addAction("Translate", this, SLOT(gizmoTypeTranslate()), QKeySequence(Qt::CTRL + Qt::Key_F1));
    QAction *actionGizmoTypeRotate = menuGizmo->addAction("Rotate", this, SLOT(gizmoTypeRotate()), QKeySequence(Qt::CTRL + Qt::Key_F2));
    QAction *actionGizmoTypeScale = menuGizmo->addAction("Scale", this, SLOT(gizmoTypeScale()), QKeySequence(Qt::CTRL + Qt::Key_F3));

    actionGizmoAlwaysOnTop->setCheckable(true);
    actionGizmoTypeTranslate->setCheckable(true);
    actionGizmoTypeRotate->setCheckable(true);
    actionGizmoTypeScale->setCheckable(true);

    QActionGroup *actionAxisTypeGroup = new QActionGroup(menuGizmo);
    actionAxisTypeGroup->addAction(actionGizmoTypeTranslate);
    actionAxisTypeGroup->addAction(actionGizmoTypeRotate);
    actionAxisTypeGroup->addAction(actionGizmoTypeScale);
    actionGizmoAlwaysOnTop->setChecked(true);
    actionGizmoTypeTranslate->setChecked(true);

    QMenu *menuHelp = menuBar()->addMenu("Help");
    menuHelp->addAction("Check for Update", this, SLOT(helpCheckForUpdates()));
    menuHelp->addAction("View Source Code", this, SLOT(helpSourceCode()));
    menuHelp->addSeparator();
    menuHelp->addAction("Bug Report", this, SLOT(helpBugReport()));
    menuHelp->addAction("Feature Request", this, SLOT(helpFeatureRequest()));
    menuHelp->addSeparator();
    menuHelp->addAction("System Info", this, SLOT(helpSystemInfo()));
    menuHelp->addAction("About", this, SLOT(helpAbout()));
}

void MainWindow::configLayout() {
    m_splitter = new QSplitter(this);
    m_splitter->addWidget(m_sceneTreeWidget);
    m_splitter->addWidget(QWidget::createWindowContainer(m_openGLWindow));
    m_splitter->addWidget(m_propertyWidget);
    m_splitter->setSizes(QList<int>{160, width() - 160 - 300, 300});


    //m_dock_perspect_1 = new QDockWidget();
    //m_dock_perspect_1->setWidget(m_perspectWindow);
    //m_dock_perspect_2 = new QDockWidget();
    QHBoxLayout * mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_splitter);
    centralWidget()->setLayout(mainLayout);

    m_left = new QDockWidget();
    m_right = new QDockWidget();


    //m_left->set
}

void MainWindow::configSignals() {
    connect(m_openGLWindow, SIGNAL(fpsChanged(int)), this, SLOT(fpsChanged(int)));
    connect(m_sceneTreeWidget, SIGNAL(itemSelected(QVariant)), this, SLOT(itemSelected(QVariant)));
    connect(m_sceneTreeWidget, SIGNAL(itemDeselected(QVariant)), this, SLOT(itemDeselected(QVariant)));
}

void MainWindow::fpsChanged(int fps) {
    m_fpsLabel->setText("FPS: " + QString::number(fps));
}

void MainWindow::itemSelected(QVariant item) {
    delete m_propertyWidget->takeWidget();

    if (item.canConvert<Camera*>()) {
        m_propertyWidget->setWidget(new CameraProperty(item.value<Camera*>(), this));
    } else if (item.canConvert<Gridline*>()) {
        m_propertyWidget->setWidget(new GridlineProperty(item.value<Gridline*>(), this));
    } else if (item.canConvert<AmbientLight*>()) {
        m_propertyWidget->setWidget(new AmbientLightProperty(item.value<AmbientLight*>(), this));
    } else if (item.canConvert<DirectionalLight*>()) {
        m_propertyWidget->setWidget(new DirectionalLightProperty(item.value<DirectionalLight*>(), this));
    } else if (item.canConvert<PointLight*>()) {
        m_propertyWidget->setWidget(new PointLightProperty(item.value<PointLight*>(), this));
    } else if (item.canConvert<SpotLight*>()) {
        m_propertyWidget->setWidget(new SpotLightProperty(item.value<SpotLight*>(), this));
    } else if (item.canConvert<Model*>()) {
        m_propertyWidget->setWidget(new ModelProperty(item.value<Model*>(), this));
    } else if (item.canConvert<Mesh*>()) {
        m_propertyWidget->setWidget(new MeshProperty(item.value<Mesh*>(), this));
    } else if (item.canConvert<Material*>()) {
        m_propertyWidget->setWidget(new MaterialProperty(item.value<Material*>(), this));
    }
}

void MainWindow::itemDeselected(QVariant) {
    delete m_propertyWidget->takeWidget();
    m_propertyWidget->setWidget(0);
}

void MainWindow::fileNewScene() {
    if (m_host) {
        if (!askToSaveScene()) return;
        m_propertyWidget->setWidget(0);
        delete m_host;
        m_host = 0;
    }

    m_host = new Scene;
    m_host->addGridline(new Gridline);
    m_host->addDirectionalLight(new DirectionalLight(QVector3D(1, 1, 1), QVector3D(-2, -4, -3)));
    m_host->addModel(ModelLoader::loadCubeModel());

    m_sceneTreeWidget->setScene(m_host);
    m_openGLWindow->setScene(new OpenGLScene(m_host));


    /// <summary>
    /// ///////////
    /// </summary>
    //m_perspectWindow->setScene(new OpenGLPerspectScene(m_host));

    m_sceneFilePath = "";
}

void MainWindow::fileOpenScene() {
    if (!askToSaveScene()) return;
    QString filePath = QFileDialog::getOpenFileName(this, "Open Project", "", "Ash Engine Project (*.aeproj)");
    if (filePath == 0) return;

    SceneLoader loader;
    Scene* scene = loader.loadFromFile(filePath);

    if (loader.hasErrorLog()) {
        QString log = loader.errorLog();
        QMessageBox::critical(0, "Error", log);
        if (log_level >= LOG_LEVEL_ERROR)
            dout << log;
    }

    if (scene) {
        m_propertyWidget->setWidget(0);
        if (m_host) delete m_host;

        m_host = scene;
        m_sceneTreeWidget->setScene(m_host);
        m_openGLWindow->setScene(new OpenGLScene(m_host));
        /// <summary>
        /// //////////////////
        /// </summary>


        m_sceneFilePath = filePath;
    }
}

void MainWindow::fileImportModel() {
    QString filePath = QFileDialog::getOpenFileName(this, "Load Model", "", "All Files (*)");
    if (filePath == 0) return;

    ModelLoader loader;
    Model* model = loader.loadModelFromFile(filePath);

    if (loader.hasErrorLog()) {
        QString log = loader.errorLog();
        QMessageBox::critical(0, "Error", log);
        if (log_level >= LOG_LEVEL_ERROR)
            dout << log;
    }

    if (m_host && model) m_host->addModel(model);
}

void MainWindow::fileExportModel() {
    if (!m_host) return;
    if (AbstractEntity::getSelected() == 0 || (!AbstractEntity::getSelected()->isMesh() && !AbstractEntity::getSelected()->isModel())) {
        QMessageBox::critical(0, "Error", "Select a Model/Mesh to export.");
        return;
    }

    QString filter = "X Files (*.x);;";
    filter += "Step Files (*.stp);;";
    filter += "Wavefront OBJ format (*.obj);;";
    filter += "Stereolithography (*.stl);;";
    filter += "Stanford Polygon Library (*.ply);;";
    filter += "Autodesk 3DS (legacy) (*.3ds);;";
    filter += "Extensible 3D (*.x3d);;";
    filter += "3MF File Format (*.3mf);;";
    QString filePath = QFileDialog::getSaveFileName(this, "Export Model", "", filter);
    if (filePath == 0) return;

    ModelExporter exporter;
    if (Model* model = qobject_cast<Model*>(AbstractEntity::getSelected()))
        exporter.saveToFile(model, filePath);
    else if (Mesh* mesh = qobject_cast<Mesh*>(AbstractEntity::getSelected()))
        exporter.saveToFile(mesh, filePath);

    if (exporter.hasErrorLog()) {
        QString log = exporter.errorLog();
        QMessageBox::critical(0, "Error", log);
        if (log_level >= LOG_LEVEL_ERROR)
            dout << log;
    }
}

void MainWindow::fileSaveScene() {
    if (!m_host) return;
    if (m_sceneFilePath.length()) {
        SceneSaver saver(m_host);
        saver.saveToFile(m_sceneFilePath);
    } else
        fileSaveAsScene();
}

void MainWindow::fileSaveAsScene() {
    if (!m_host) return;
    QString filePath = QFileDialog::getSaveFileName(this, "Save Project", "", "Ash Engine Project (*.aeproj)");
    if (filePath == 0) return;

    SceneSaver saver(m_host);
    saver.saveToFile(filePath);
    m_sceneFilePath = filePath;

    if (saver.hasErrorLog()) {
        QString log = saver.errorLog();
        QMessageBox::critical(0, "Error", log);
        if (log_level >= LOG_LEVEL_ERROR)
            dout << log;
    }
}

void MainWindow::fileQuit() {
    if (!askToSaveScene()) return;
    QApplication::quit();
}

void MainWindow::editCopy() {
    if (m_sceneTreeWidget->hasFocus()) {
        QVariant item = m_sceneTreeWidget->currentItem()->data(0, Qt::UserRole);
        if (item.canConvert<Camera*>() || item.canConvert<Material*>())
            m_copyedObject.clear(); // Copy is not allowed
        else
            m_copyedObject = item;
    } else if (AbstractEntity::getSelected())
        m_copyedObject = m_sceneTreeWidget->currentItem()->data(0, Qt::UserRole);
    else
        m_copyedObject.clear();

    if (log_level >= LOG_LEVEL_INFO) {
        if (m_copyedObject.isValid())
            dout << m_copyedObject.value<QObject*>()->objectName() << "is copyed";
        else
            dout << "Nothing is copyed";
    }
}

void MainWindow::editPaste() {
    if (!m_copyedObject.isValid()) return;
    if (m_copyedObject.canConvert<AmbientLight*>()) {
        if (m_host->ambientLights().size() >= 8) {
            QMessageBox::critical(0, "Error", "The amount of ambient lights has reached the upper limit of 8.");
            return;
        }
        AmbientLight* light = m_copyedObject.value<AmbientLight*>();
        AmbientLight* newLight = new AmbientLight(*light);
        newLight->setParent(light->parent());
    } else if (m_copyedObject.canConvert<DirectionalLight*>()) {
        if (m_host->directionalLights().size() >= 8) {
            QMessageBox::critical(0, "Error", "The amount of directional lights has reached the upper limit of 8.");
            return;
        }
        DirectionalLight* light = m_copyedObject.value<DirectionalLight*>();
        DirectionalLight* newLight = new DirectionalLight(*light);
        newLight->setParent(light->parent());
    } else if (m_copyedObject.canConvert<PointLight*>()) {
        if (m_host->pointLights().size() >= 8) {
            QMessageBox::critical(0, "Error", "The amount of point lights has reached the upper limit of 8.");
            return;
        }
        PointLight* light = m_copyedObject.value<PointLight*>();
        PointLight* newLight = new PointLight(*light);
        newLight->setParent(light->parent());
    } else if (m_copyedObject.canConvert<SpotLight*>()) {
        if (m_host->spotLights().size() >= 8) {
            QMessageBox::critical(0, "Error", "The amount of spotlights has reached the upper limit of 8.");
            return;
        }
        SpotLight* light = m_copyedObject.value<SpotLight*>();
        SpotLight* newLight = new SpotLight(*light);
        newLight->setParent(light->parent());
    } else if (m_copyedObject.canConvert<Gridline*>()) {
        Gridline* gridline = m_copyedObject.value<Gridline*>();
        Gridline* newGridline = new Gridline(*gridline);
        newGridline->setParent(gridline->parent());
    } else if (m_copyedObject.canConvert<Model*>()) {
        Model* model = m_copyedObject.value<Model*>();
        Model* newModel = new Model(*model);
        newModel->setParent(model->parent());
    } else if (m_copyedObject.canConvert<Mesh*>()) {
        Mesh* mesh = m_copyedObject.value<Mesh*>();
        Mesh* newMesh = new Mesh(*mesh);
        newMesh->setParent(mesh->parent());
    } else {
        if (log_level >= LOG_LEVEL_ERROR)
            dout << "Failed to paste: Unknown type";
    }
}

void MainWindow::editRemove() {
    if (!m_host || !m_sceneTreeWidget->currentItem()) return;
    QVariant item = m_sceneTreeWidget->currentItem()->data(0, Qt::UserRole);
    QObject* obj = item.value<QObject*>();
    Camera* c = dynamic_cast<Camera*>(obj);
    if (c && (c->GetCameraType() == eSceneCamera || c == m_host->camera())) {
        QMessageBox::warning(this, "Warning", "Camera can not be deleted.");
        if (log_level >= LOG_LEVEL_WARNING)
            dout << "Warning: Camera can not be deleted";
        return;
    }
    m_sceneTreeWidget->setCurrentItem(0);
    if (m_copyedObject == item)
        m_copyedObject.clear();
    delete item.value<QObject*>();
}

void MainWindow::createGridline() 
{
    if (m_host) m_host->addGridline(new Gridline);
}
//perspect
void MainWindow::createPerspectCamera()
{
    if (m_host) 
    {   
        if (m_host->GetPerspectCameras().size() >= 2)
        {
            QMessageBox::warning(this, "Warning", "PerspectCamera limits <=2.");
            return;
        }
        Camera* c = new Camera(ePespectiveCamera);
        bool ret = m_host->addPerspectCamera(c);    
        if (ret)
        {

            m_openGLWindow->capture(c);
            //if (!perspect_widget1)
            //{
            //    perspect_widget1 = new OpenGLSimpleWidget();
            //    perspect_widget1->m_camera = c;
            //    perspect_widget1->setRenderer(new OpenGLPerspectRenderer);
            //    perspect_widget1->setScene(new OpenGLPerspectScene(m_host));
            //    perspect_widget1->setWindowFlag(Qt::WindowStaysOnTopHint);
            //    perspect_widget1->setFixedSize(QSize(320, 240));
            //    //perspect_widget->setObjectName();
            //    perspect_widget1->show();
            //    return;
            //}
            //if (!perspect_widget2)
            //{
            //    perspect_widget2 = new OpenGLSimpleWidget();
            //    perspect_widget2->m_camera = c;
            //    perspect_widget2->setRenderer(new OpenGLPerspectRenderer);
            //    perspect_widget2->setScene(new OpenGLPerspectScene(m_host));
            //    perspect_widget2->setWindowFlag(Qt::WindowStaysOnTopHint);
            //    perspect_widget2->setFixedSize(QSize(320, 240));
            //    //perspect_widget->setObjectName();
            //    perspect_widget2->show();
            //}
          /*  QDockWidget* dockWidget = new QDockWidget();
            dockWidget->setAttribute(Qt::WA_DeleteOnClose);
            dockWidget->setWidget(perspect_widget);
            dockWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable| QDockWidget::DockWidgetClosable);
            dockWidget->setFixedSize(QSize(640, 480));
            this->addDockWidget(Qt::DockWidgetArea(Qt::RightDockWidgetArea),dockWidget);*/
        }
    }
}

void MainWindow::createAmbientLight() {
   if (m_host) {
       if (m_host->ambientLights().size() >= 8) {
           QMessageBox::critical(0, "Error", "The amount of ambient lights has reached the upper limit of 8.");
           return;
       }
       m_host->addLight(new AmbientLight);
   }
}

void MainWindow::createDirectionalLight() {
    if (m_host) {
        if (m_host->directionalLights().size() >= 8) {
            QMessageBox::critical(0, "Error", "The amount of directional lights has reached the upper limit of 8.");
            return;
        }
        m_host->addLight(new DirectionalLight);
    }
}

void MainWindow::createPointLight() {
    if (m_host) {
        if (m_host->pointLights().size() >= 8) {
            QMessageBox::critical(0, "Error", "The amount of point lights has reached the upper limit of 8.");
            return;
        }
        m_host->addLight(new PointLight);
    }
}

void MainWindow::createSpotLight() {
    if (m_host) {
        if (m_host->spotLights().size() >= 8) {
            QMessageBox::critical(0, "Error", "The amount of spotlights has reached the upper limit of 8.");
            return;
        }
        m_host->addLight(new SpotLight);
    }
}

void MainWindow::createBasicCone() {
    if (m_host) m_host->addModel(ModelLoader::loadConeModel());
}

void MainWindow::createBasicCube() {
    if (m_host) m_host->addModel(ModelLoader::loadCubeModel());
}

void MainWindow::createBasicCylinder() {
    if (m_host) m_host->addModel(ModelLoader::loadCylinderModel());
}

void MainWindow::createBasicPlane() {
    if (m_host) m_host->addModel(ModelLoader::loadPlaneModel());
}

void MainWindow::createBasicSphere() {
    if (m_host) m_host->addModel(ModelLoader::loadSphereModel());
}

void MainWindow::polygonAssignMaterial() {
    if (AbstractEntity::getSelected() && AbstractEntity::getSelected()->isMesh())
        static_cast<Mesh*>(AbstractEntity::getSelected())->setMaterial(new Material);
    else
        QMessageBox::critical(0, "Error", "Select a mesh to do this operation.");
}

void MainWindow::polygonReverseNormals() {
    if (AbstractEntity::getSelected() && AbstractEntity::getSelected()->isModel())
        static_cast<Model*>(AbstractEntity::getSelected())->reverseNormals();
    else if (AbstractEntity::getSelected() && AbstractEntity::getSelected()->isMesh())
        static_cast<Mesh*>(AbstractEntity::getSelected())->reverseNormals();
    else
        QMessageBox::critical(0, "Error", "Select a model/mesh to do this operation.");
}

void MainWindow::polygonReverseTangents() {
    if (AbstractEntity::getSelected() && AbstractEntity::getSelected()->isModel())
        static_cast<Model*>(AbstractEntity::getSelected())->reverseTangents();
    else if (AbstractEntity::getSelected() && AbstractEntity::getSelected()->isMesh())
        static_cast<Mesh*>(AbstractEntity::getSelected())->reverseTangents();
    else
        QMessageBox::critical(0, "Error", "Select a model/mesh to do this operation.");
}

void MainWindow::polygonReverseBitangents() {
    if (AbstractEntity::getSelected() && AbstractEntity::getSelected()->isModel())
        static_cast<Model*>(AbstractEntity::getSelected())->reverseBitangents();
    else if (AbstractEntity::getSelected() && AbstractEntity::getSelected()->isMesh())
        static_cast<Mesh*>(AbstractEntity::getSelected())->reverseBitangents();
    else
        QMessageBox::critical(0, "Error", "Select a model/mesh to do this operation.");
}

void MainWindow::gizmoAlwaysOnTop(bool alwaysOnTop) {
    if (m_host) m_host->transformGizmo()->setAlwaysOnTop(alwaysOnTop);
}

void MainWindow::gizmoTypeTranslate() {
    if (m_host) m_host->transformGizmo()->setTransformMode(TransformGizmo::Translate);
}

void MainWindow::gizmoTypeRotate() {
    if (m_host) m_host->transformGizmo()->setTransformMode(TransformGizmo::Rotate);
}

void MainWindow::gizmoTypeScale() {
    if (m_host)m_host->transformGizmo()->setTransformMode(TransformGizmo::Scale);
}

void MainWindow::helpCheckForUpdates() {
    QString url = "https://api.github.com/repos/afterthat97/AshEngine/releases/latest";
    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyOfUpdates(QNetworkReply*)));
    networkManager->get(QNetworkRequest(QUrl(url)));
    if (log_level >= LOG_LEVEL_INFO)
        dout << "GET request is sent to " << url;
}

void MainWindow::helpSourceCode() {
    QDesktopServices::openUrl(QUrl("https://github.com/afterthat97/AshEngine"));
}

void MainWindow::helpBugReport() {
    QDesktopServices::openUrl(QUrl("https://github.com/afterthat97/AshEngine/issues/new?template=bug_report.md"));
}

void MainWindow::helpFeatureRequest() {
    QDesktopServices::openUrl(QUrl("https://github.com/afterthat97/AshEngine/issues/new?template=feature_request.md"));
}

void MainWindow::helpSystemInfo() {
    QString info = "";
    info += "Renderer: " + m_openGLWindow->rendererName() + "\n";
    info += "OpenGL Version: " + m_openGLWindow->openGLVersion() + "\n";
    info += "GLSL Version: " + m_openGLWindow->shadingLanguageVersion() + "\n";
    QMessageBox::about(this, "System Info", info);
}

void MainWindow::helpAbout() {
    QString info = "Current version: " + QString(APP_VERSION) + "\n\n";
    info += "Ash Engine is a cross-platform 3D engine for learning purpose, based on Qt, OpenGL and Assimp.\n\n";
    info += "Author: Alfred Liu\n";
    info += "Email:  afterthat97@foxmail.com";
    QMessageBox::about(this, "About", info);
}

void MainWindow::replyOfUpdates(QNetworkReply * reply) {
    QString strReply = (QString) reply->readAll();
    QJsonObject jsonObject = QJsonDocument::fromJson(strReply.toUtf8()).object();
    QString latestVersion = jsonObject["tag_name"].toString();

    if (log_level >= LOG_LEVEL_INFO)
        dout << "Reply: the latest version is" << latestVersion;

    if (latestVersion != APP_VERSION && latestVersion != "") {
        QString info = "A new version has been released, do you want to upgrade?\n\n";
        info += "Current version: " + QString(APP_VERSION) + "\n";
        info += "Latest version: " + latestVersion;
        if (QMessageBox::question(this, "New Update Available", info, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            QDesktopServices::openUrl(QUrl(jsonObject["html_url"].toString()));
    }
}

