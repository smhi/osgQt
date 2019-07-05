#include "osgqtwidgets.h"

#include <QHBoxLayout>
#include <osgDB/WriteFile>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osg/Point>

#include <osg/Notify>
#include <osg/Endian>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReaderWriter>

#include <osgUtil/TriStripVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osg/TriangleFunctor>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LightSource>
#include <osg/BoundingBox>
#include <osg/ComputeBoundsVisitor>

#include <osgVolume/Volume>
#include <osgVolume/VolumeTile>
#include <osgVolume/Property>


#include <string.h>
#include <string>

#include <unistd.h>

#include <memory>
#include <iostream>

#include "back.xpm"
#include "start.xpm"
#include "stopp.xpm"
#include "slutt.xpm"
#include "loop.xpm"
#include "forward.xpm"
#include "autoupdate.xpm"
#include <QMimeData>

#include <ClientSelection.h>
#include <miMessage.h>
#include <QLetterCommands.h>


bool MainWidget::inUpdate = false;

class FindVolumeTiles : public osg::NodeVisitor
{
public:
    FindVolumeTiles(): osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    typedef std::vector< osg::ref_ptr<osgVolume::VolumeTile> > Tiles;
    Tiles _tiles;

    void apply(osg::Group& group)
    {
        osgVolume::VolumeTile* tile = dynamic_cast<osgVolume::VolumeTile*>(&group);
        if (tile) _tiles.push_back(tile);
        else traverse(group);
    }
};

OsgWidget::OsgWidget(osg::ArgumentParser& arguments,QWidget *parent):Viewer(arguments)
{
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--image <filename>","Load an image and render it on a quad");
    arguments.getApplicationUsage()->addCommandLineOption("--dem <filename>","Load an image/DEM and render it on a HeightField");
    arguments.getApplicationUsage()->addCommandLineOption("--login <url> <username> <password>","Provide authentication information for http file access.");
    arguments.getApplicationUsage()->addCommandLineOption("-p <filename>","Play specified camera path animation file, previously saved with 'z' key.");
    arguments.getApplicationUsage()->addCommandLineOption("--speed <factor>","Speed factor for animation playing (1 == normal speed).");
    arguments.getApplicationUsage()->addCommandLineOption("--device <device-name>","add named device to the viewer");

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        exit(0);
    }

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        exit(0);
    }

    if (arguments.argc()<=2)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
    }

    std::string url, username, password;
    while(arguments.read("--login",url, username, password))
    {
        if (!osgDB::Registry::instance()->getAuthenticationMap())
        {
            osgDB::Registry::instance()->setAuthenticationMap(new osgDB::AuthenticationMap);
            osgDB::Registry::instance()->getAuthenticationMap()->addAuthenticationDetails(
                url,
                new osgDB::AuthenticationDetails(username, password)
            );
        }
    }

    std::string device;
    while(arguments.read("--device", device))
    {
        osg::ref_ptr<osgGA::Device> dev = osgDB::readRefFile<osgGA::Device>(device);
        if (dev.valid())
        {
            addDevice(dev);
        }
    }

    osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;

    setThreadingModel(threadingModel);
    m_root=new osg::Group;
    
    osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

    keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
    keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
    keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
    keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );
    keyswitchManipulator->addMatrixManipulator( '5', "Orbit", new osgGA::OrbitManipulator() );
    keyswitchManipulator->addMatrixManipulator( '6', "FirstPerson", new osgGA::FirstPersonManipulator() );
    keyswitchManipulator->addMatrixManipulator( '7', "Spherical", new osgGA::SphericalManipulator() );

    std::string pathfile;
    double animationSpeed = 1.0;
    while(arguments.read("--speed",animationSpeed) ) {}
    char keyForAnimationPath = '8';
    while (arguments.read("-p",pathfile))
    {
      osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
      if (apm || !apm->valid())
      {
        apm->setTimeScale(animationSpeed);

        unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
        keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
        keyswitchManipulator->selectMatrixManipulator(num);
        ++keyForAnimationPath;
      }
    }
    keyswitchManipulator->selectMatrixManipulator(0);
    m_widget=addViewWidget(createCamera(0, 0, 1024, 768), keyswitchManipulator.get());
    
    getCameraManipulator()->computeHomePosition(getCamera(),true);
    getCameraManipulator()->home(0);
   
    //setCameraManipulator( keyswitchManipulator.get(), false );
    
    addEventHandler(new osgViewer::StatsHandler());
    addEventHandler( new osgGA::StateSetManipulator(getCamera()->getOrCreateStateSet()) );
    //setCameraManipulator( keyswitchManipulator.get() );
    
    // add the thread model handler
    addEventHandler(new osgViewer::ThreadingHandler);

    // add the window size toggle handler
    //addEventHandler(new osgViewer::WindowSizeHandler);

    // add the help handler
    //addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

    // add the record camera path handler
    addEventHandler(new osgViewer::RecordCameraPathHandler);

    // add the LOD Scale handler
    addEventHandler(new osgViewer::LODScaleHandler);

    // add the screen capture handler
    addEventHandler(new osgViewer::ScreenCaptureHandler);
    
    // add the picker
    //addEventHandler(picker.get());

    setSceneData(m_root.get());
}

QWidget* OsgWidget::addViewWidget(osg::Camera* camera, osgGA::CameraManipulator* manipulator)
{
    setCamera( camera );
    setCameraManipulator( manipulator, true );

    osgQt::GraphicsWindowQt* gw = dynamic_cast<osgQt::GraphicsWindowQt*>( camera->getGraphicsContext() );
    return gw ? gw->getGLWidget() : NULL;
}

osg::Camera* OsgWidget::createCamera( int x, int y, int w, int h )
{
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->windowName = "SceneView";
    traits->windowDecoration = false;
    traits->x = x;
    traits->y = y;
    traits->width = w;
    traits->height = h;
    traits->doubleBuffer = true;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();

    osg::Camera * camera = getCamera ();
    camera->setGraphicsContext( new osgQt::GraphicsWindowQt( traits.get() ) );

    camera->setClearColor( osg::Vec4(0.2, 0.2, 0.6, 1.0) );
    camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
    camera->setProjectionMatrixAsPerspective(
                30.0f, static_cast<double>(traits->width) / static_cast<double>(traits->height), 1.0f, 10000.0f );
    return camera;
}

QWidget* OsgWidget::getWidget()
{
    return m_widget;
}

void OsgWidget::updateScene( osg::Node* node, std::string oldFileName, std::string newFileName)
{
    osgUtil::Optimizer optimizer;
    optimizer.optimize(node);
 
    if(m_root->getNumChildren()) {
        // Get properties from old node
        // see examples/osgtransferfunction.cpp
        // Apply this to the new node only if the same type of model, eg directory equals
        // FIXME, this works well for volumes from osgvolume, but not for volumes from vpb
        // We must keep LOD for vpb volumes
        std::string oldnodedir, nodedir;
        size_t oldpos, pos;
        oldpos = oldFileName.find_last_of('/');
        pos = newFileName.find_last_of('/');
        oldnodedir = oldFileName.substr(0,oldpos);
        nodedir=newFileName.substr(0,pos);
    
        if (oldnodedir == nodedir) {
          osg::Node* oldnode = m_root->getChild(0);
          float afProperty_value = -1.0; 
          float tpProperty_value = -1.0;
          float sampleDensityProperty_value = -1.0;
          float isoProperty_value = -1.0;
          /*
          float mipProperty_value = -1.0;
          float lightingProperty_value = -1.0;
          */
          osgVolume::ImageLayer* imageLayer = 0;
          FindVolumeTiles fvt;
          oldnode->accept(fvt);

          if (!fvt._tiles.empty())
          {
            osgVolume::VolumeTile* tile = fvt._tiles[0].get();
            imageLayer = dynamic_cast<osgVolume::ImageLayer*>(tile->getLayer());
            tile->addEventCallback(new osgVolume::PropertyAdjustmentCallback());
          }
          if (imageLayer)
          {
            osgVolume::Property* property = imageLayer->getProperty();
            if (property)
            {
              osgVolume::CollectPropertiesVisitor cpv;
              property->accept(cpv);
              if (cpv._afProperty.valid()) {
                afProperty_value = cpv._afProperty->getValue();
              }
              if (cpv._transparencyProperty.valid()) {
                tpProperty_value=cpv._transparencyProperty->getValue();
              }
              if (cpv._sampleDensityProperty.valid()) {
                sampleDensityProperty_value=cpv._sampleDensityProperty->getValue();
              }
              if (cpv._isoProperty.valid()) {
                isoProperty_value=cpv._isoProperty->getValue();
              }
              /*
              Not scalar properties.
              if (cpv._mipProperty.valid()) {
                mipProperty_value=cpv._mipProperty->getValue();
              }
              if (cpv._lightingProperty.valid()) {
                lightingProperty_value=cpv._lightingProperty->getValue();
              }
              */

            }
          }
          imageLayer = 0;
          FindVolumeTiles fvtn;
          node->accept(fvtn);

          if (!fvtn._tiles.empty())
          {
            osgVolume::VolumeTile* tile = fvtn._tiles[0].get();
            imageLayer = dynamic_cast<osgVolume::ImageLayer*>(tile->getLayer());
            tile->addEventCallback(new osgVolume::PropertyAdjustmentCallback());
          }
          if (imageLayer)
          {
            osgVolume::Property* property = imageLayer->getProperty();
            if (property)
            {
              osgVolume::CollectPropertiesVisitor cpv;
              property->accept(cpv);
              if (cpv._afProperty.valid()) {
                if (afProperty_value != -1.0)
                  cpv._afProperty->setValue(afProperty_value);
              }
              if (cpv._transparencyProperty.valid()) {
                if (tpProperty_value != -1.0)
                  cpv._transparencyProperty->setValue(tpProperty_value);
              }
              if (cpv._sampleDensityProperty.valid()) {
                if (sampleDensityProperty_value != -1.0)
                  cpv._sampleDensityProperty->setValue(sampleDensityProperty_value);
              }
              if (cpv._isoProperty.valid()) {
                if (isoProperty_value != -1.0)
                  cpv._isoProperty->setValue(isoProperty_value);
              }
              /*
              Not scalar properties
              if (cpv._mipProperty.valid()) {
                if (mipProperty_value != -1.0)
                  cpv._mipProperty->setValue(mipProperty_value);
              }
              if (cpv._lightingProperty.valid()) {
                if (lightingProperty_value != -1.0)
                  cpv._lightingProperty->setValue(lightingProperty_value);
              }
              */
              
            }
          }
        } // End dir names equal
        m_root->replaceChild(m_root->getChild(0),node);      
    } else {
        m_root->addChild(node);
    }
   
    setSceneData(m_root.get());
    requestRedraw();
    getWidget()->update();
}

MainWidget::MainWidget(int argc, char *argv[]):timeron(0),timeloop(false),currentIndex(0),timeout_ms(1000)
{
    // Parse the arguments and send to viewer implementation
    osg::ArgumentParser arguments(&argc,argv);
    
    m_ModelManager = new ModelManager();
    m_ModelManager->parseSetup();
    FieldModelGroupInfo_v test = m_ModelManager->getFieldModelGroups();
// Just debug
    for (size_t i = 0; i < test.size(); i++)
    {
      std::cerr << test[i].groupName << std::endl;
      FieldModelInfo_v test1 = test[i].models;
      for (size_t j = 0; j < test1.size(); j++)
      {
        std::cerr << test1[j].modelName << std::endl;
        std::cerr << test1[j].setupInfo << std::endl;
        std::vector<std::string> names = m_ModelManager->getFileNames(test1[j].modelName);
        for (size_t k = 0; k < names.size(); k++)
        {
          std::cerr << names[k] << std::endl;
        }
      }
    }      
    
    OsgWidget* viewer=new OsgWidget(arguments);
    QWidget* widget=viewer->getWidget();
    widget->setMinimumSize(QSize(1024, 768));
    setCentralWidget(widget);
    m_viewThread.reset(new ViewerFrameThread(viewer,true));
    m_updateOperation=new UpdateOperation();
    viewer->addUpdateOperation(m_updateOperation);
    m_viewThread->start();
    setAcceptDrops(true);

    QMenu* fileMenu = menuBar()->addMenu("&File");
    openAct = new QAction("&Open...",this);
    fileQuitAction = new QAction("&Quit...", this );
    fileQuitAction->setShortcut(Qt::CTRL+Qt::Key_Q);
    fileQuitAction->setShortcutContext(Qt::ApplicationShortcut);
    fileMenu->addAction(openAct);
    fileMenu->addSeparator();
    fileMenu->addAction( fileQuitAction );
    connect(openAct,SIGNAL(triggered()),this,SLOT(openFile()));
    connect( fileQuitAction, SIGNAL( triggered() ) , SLOT( filequit() ) );
    
    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpDocAction = new QAction( tr("Q&uick help"), this );
    helpDocAction->setShortcutContext(Qt::ApplicationShortcut);
    helpDocAction->setShortcut(Qt::Key_F1);
    helpDocAction->setCheckable(false);
    helpMenu->addAction(helpDocAction);
    connect( helpDocAction, SIGNAL( triggered() ) ,  SLOT( showHelp() ) );
    // timecommands ======================
  // --------------------------------------------------------------------
  timeBackwardAction = new QAction(QIcon( QPixmap(start_xpm )),tr("Run Backwards"), this);
  timeBackwardAction->setShortcutContext(Qt::ApplicationShortcut);
  timeBackwardAction->setShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_Left);
  timeBackwardAction->setCheckable(true);
  timeBackwardAction->setIconVisibleInMenu(true);
  connect( timeBackwardAction, SIGNAL( triggered() ) ,  SLOT( animationBack() ) );
  // --------------------------------------------------------------------
  timeForewardAction = new QAction(QIcon( QPixmap(slutt_xpm )),tr("Run Forewards"), this );
  timeForewardAction->setShortcutContext(Qt::ApplicationShortcut);
  timeForewardAction->setShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_Right);
  timeForewardAction->setCheckable(true);
  timeForewardAction->setIconVisibleInMenu(true);
  connect( timeForewardAction, SIGNAL( triggered() ) ,  SLOT( animation() ) );
  // --------------------------------------------------------------------
  timeStepBackwardAction = new QAction(QIcon( QPixmap(back_xpm )),tr("Step Backwards"), this );
  timeStepBackwardAction->setShortcut(Qt::CTRL+Qt::Key_Left);
  timeStepBackwardAction->setCheckable(false);
  timeStepBackwardAction->setIconVisibleInMenu(true);
  connect( timeStepBackwardAction, SIGNAL( triggered() ) ,  SLOT( stepback() ) );
  // --------------------------------------------------------------------
  timeStepForewardAction = new QAction(QIcon( QPixmap(forward_xpm )),tr("Step Forewards"), this );
  timeStepForewardAction->setShortcut(Qt::CTRL+Qt::Key_Right);
  timeStepForewardAction->setCheckable(false);
  timeStepForewardAction->setIconVisibleInMenu(true);
  connect( timeStepForewardAction, SIGNAL( triggered() ) ,  SLOT( stepforward() ) );
  // --------------------------------------------------------------------
  timeStopAction = new QAction(QIcon( QPixmap(stop_xpm )),tr("Stop"), this );
  timeStopAction->setShortcutContext(Qt::ApplicationShortcut);
  timeStopAction->setShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_Down);
  timeStopAction->setCheckable(false);
  timeStopAction->setIconVisibleInMenu(true);
  connect( timeStopAction, SIGNAL( triggered() ) ,  SLOT( animationStop() ) );
  // --------------------------------------------------------------------
  timeLoopAction = new QAction(QIcon( QPixmap(loop_xpm )),tr("Run in loop"), this );
  timeLoopAction->setShortcutContext(Qt::ApplicationShortcut);
  timeLoopAction->setShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_Up);
  timeLoopAction->setCheckable(true);
  timeLoopAction->setIconVisibleInMenu(true);
  connect( timeLoopAction, SIGNAL( triggered() ) ,  SLOT( animationLoop() ) );
  
  // Autoupdate ===============================
  // --------------------------------------------------------------------
  autoUpdateAction = new QAction(QIcon( QPixmap(autoupdate_xpm)),
      tr("Automatic updates"), this );
  autoUpdateAction->setCheckable(true);
  autoUpdateAction->setIconVisibleInMenu(true);
  doAutoUpdate = false;
  connect( autoUpdateAction, SIGNAL( triggered() ), SLOT(autoUpdate()));
  
  timerToolbar= new QToolBar("TimerToolBar",this);
  
  timerToolbar->setObjectName("TimerToolBar");
  
  addToolBar(Qt::TopToolBarArea,timerToolbar);

  timerToolbar->addAction( timeBackwardAction    );
  timerToolbar->addAction( timeStepBackwardAction );
  timerToolbar->addAction( timeStepForewardAction );
  timerToolbar->addAction( timeForewardAction  );
  timerToolbar->addAction( timeStopAction      );
  timerToolbar->addAction( timeLoopAction        );
  timerToolbar->addAction( autoUpdateAction        );
  
  // Connect to filewatcher/coserver/ClientSelection
  pluginB = new ClientSelection("osgQtWidget", this);
  pluginB->client()->setServerCommand(QString::fromStdString("/usr/bin/coserver"));
  connect(pluginB, SIGNAL(receivedMessage(int, const miQMessage&)),
      SLOT(processLetter(int, const miQMessage&)));
  connect(pluginB, SIGNAL(disconnected()),
      SLOT(connectionClosed()));
  connect(pluginB, SIGNAL(renamed(const QString&)),
      SLOT(setInstanceName(const QString&)));
  setInstanceName("osgQtWidget");

  QToolButton* clientbutton = new QToolButton(timerToolbar);
  clientbutton->setDefaultAction(pluginB->getToolButtonAction());
  timerToolbar->addWidget(clientbutton);
  
  
    setWindowTitle("osgQtWidget - Press SPACE first time you have loaded a model");
}

void MainWidget::dropEvent( QDropEvent *event_in )
{
    if(event_in->mimeData()->hasFormat("text/uri-list")){
        QString fileName = event_in->mimeData()->urls().first().toLocalFile();
        m_fileNames.append(fileName);
        m_updateOperation->updateScene(fileName.toStdString());
        setWindowTitle("osgQtWidget - " + fileName.mid(fileName.lastIndexOf('/') + 1));
    }
}

void MainWidget::openFile()
{
    
    QStringList fileNames;
    QString fileName = m_updateOperation->getNodeFileName().c_str();
    if(!fileName.isEmpty()) {
      fileNames = QFileDialog::getOpenFileNames(this,"Open File", fileName, "Model Files (*.ive *.gz *.dae);;All files (*.*)");
    } else {
      fileNames = QFileDialog::getOpenFileNames(this,"Open File", QString(), "Model Files (*.ive *.gz *.dae);;All files (*.*)");
    }
    if (!fileNames.isEmpty())  
    {
      m_fileNames = fileNames;
      currentIndex = 0;
      fileName = fileNames.at(currentIndex);
      setWindowTitle("osgQtWidget - " + fileName.mid(fileName.lastIndexOf('/') + 1));
    }
    m_updateOperation->updateScene(fileName.toStdString());
}

void MainWidget::filequit()
{
  // quit sends aboutToQuit SIGNAL, which is connected to slot writeLogFile
  QApplication::exit(0);
}

void MainWidget::showHelp()
{
  QWidget * widget = new QWidget;
  widget->setLayout(new QVBoxLayout);

  QString text = 
               QString("Menu commands:\n") +
               QString("File                Opens a multi selection file open dialog the select what models you will se.\n") +
               QString("Quit                Exits the application.\n") +
               QString("Help                Show this help text.\n") +              
               QString("\n\n\n")+
               QString("Menu buttons:\n") +
               QString("<<:                 Run backwards, no wrap around.\n") +
               QString("<-:                 Step backward, wrap around.\n") +
               QString("->:                 Step forward.\n") +
               QString(">>:                 Run forwards, no wrap around\n") +
               QString("[]:                 Stop animation\n") +
               QString("@:                  Loop (animate in a loop)\n\n\n") +
               QString("Drag and drop:      You can drag an icon from the file manager and drop it in the main window in order to display the model.\n\n\n") +
               QString("Key you may use to change the viewers behaviour:\n\n") + 
               QString("*                   Increase LODScale. \n") + 
               QString("/                   Decrease LODScale.\n") + 
               QString("1                   Select 'Trackball' camera manipulator (default).\n") +
               QString("2                   Select 'Flight' camera manipulator.\n") +
               QString("3                   Select 'Drive' camera manipulator.\n") +
               QString("4                   Select 'Terrain' camera manipulator.\n") +
               QString("5                   Select 'Orbit' camera manipulator.\n") +
               QString("6                   Select 'First Person' camera manipulator.\n") +
               QString("7                   Select 'Sperical' camera manipulator.\n") +
               QString("<                   Decrease the screen resulotion (in Windowed mode).\n") +
               QString(">                   Increase the screen resulotion (in Windowed mode).\n") +
               QString("Drive: Down         Cursor down key to look downwards.\n") +
               QString("Drive: Space        Reset the viewing position to home.\n") +
               QString("Drive: Up           Cursor up key to look upwards.\n") +
               QString("Drive: a            Use mouse middle, right buttons for speed.\n") +
               QString("First Person: Space Reset the viewing position to home.\n") +
               QString("Flight: Space       Reset the viewing position to home.\n") +
               QString("Flight: a           No yaw when banked.\n") +
               QString("Flight: q           Automatically yaw when banked (default).\n") +
               QString("S                   Output stats to console.\n") +
               QString("Spherical: ALT      Rotates horizontally only.\n") +
               QString("Spherical: SHIFT    Rotates vertically only.\n") +
               QString("Spherical: Space    Reset the viewing position to home.\n") +
               QString("Terrain: Space      Reset the viewing position to home.\n") +
               QString("Tractball: Space    Reset the viewing position to home.\n") +
               QString("Z                   Toggle camera path playback.\n") +
               QString("b                   Toggle backface culling.\n") +
               QString("c                   Take screenshot.\n") +
               QString("e                   Toggle the placement off the end of frame barrier.\n") +
               QString("l                   Toggle lightning.\n") +
               QString("m                   Toggle threading modell.\n") +
               QString("s                   On screen stats.\n") +
               QString("t                   Toggle texturing.\n") +
               QString("w                   Toggle polygon fill mode between fill, line and points.\n") +
               QString("\n\n\n")+
               QString("Use t + mouse move up or down to change texuring.\n") +
               QString("Use a + mouse move up or down to change transparency.\n") +
               QString("ESC                 Exits the application.\n\n\n");
               
               
  
  QLabel* label = new QLabel(text);
  label->setWordWrap(true);
  label->setTextInteractionFlags(Qt::TextEditorInteraction);

  QPalette palette_ = label->palette();
  palette_.setColor(QPalette::Highlight, Qt::darkBlue);
  palette_.setColor(QPalette::HighlightedText, Qt::white);
  label->setPalette(palette_);

  QScrollArea* scrollArea = new QScrollArea;
  scrollArea->setWidget(label);

  widget->layout()->addWidget(scrollArea);

  widget->setGeometry(50, 50, 1020, 764);
  
  QGraphicsScene * graphicsScene = new QGraphicsScene;
  graphicsScene->addWidget(widget);

  QGraphicsView* graphicsView = new QGraphicsView;
  graphicsView->setScene(graphicsScene);

  QMainWindow* mainWindow = new QMainWindow;
  mainWindow->setCentralWidget(graphicsView);
  mainWindow->setGeometry(50, 50, 1024, 768);
  mainWindow->show();
  mainWindow->raise();
}

void MainWidget::stopAnimation()
{
  timeBackwardAction->setChecked( false );
  timeForewardAction->setChecked( false );
  killTimer(animationTimer);
  currentIndex = 0;
  timeron=0;

}

void MainWidget::animationLoop()
{
  timeloop= !timeloop;
  timeLoopAction->setChecked( timeloop );
}

void MainWidget::timerEvent(QTimerEvent *e)
{
  if (e->timerId()==animationTimer){
    if(MainWidget::inUpdate) return;
    if (!timeloop) {
      if (timeron == 1) {
        // stop animation if end of stringlist
        currentIndex++;
        if (currentIndex > m_fileNames.size() - 1) {
          stopAnimation();
          return;
        }
      } else if (timeron == -1) {
        // stop animation if beginning of stringlist
        currentIndex--;
        if (currentIndex < 0) {
          currentIndex = m_fileNames.size() - 1;
          stopAnimation();
          return;
        }
      }
    } else {
      if (timeron == 1) {
        // loop to the beginning if end of stringlist
        currentIndex++;
        if (currentIndex > m_fileNames.size() - 1)
          currentIndex = 0;
      } else if (timeron == -1) {
        // loop to the end if beginning of stringlist
        currentIndex--;
        if (currentIndex < 0)
          currentIndex = m_fileNames.size() - 1;
      }
    }
    MainWidget::inUpdate = true;
    QString fileName = m_fileNames.at(currentIndex);    
    m_updateOperation->updateScene(fileName.toStdString());
    setWindowTitle("osgQtWidget - " + fileName.mid(fileName.lastIndexOf('/') + 1));
  }
}

void MainWidget::animation()
{
  if (timeron!=0)
    stopAnimation();

  timeForewardAction->setChecked( true );
  //currentIndex = 0;
  animationTimer= startTimer(timeout_ms);
  timeron=1;
}

void MainWidget::animationBack()
{
  if (timeron!=0)
    stopAnimation();

  timeBackwardAction->setChecked( true );
  //currentIndex = m_fileNames.size() - 1;
  animationTimer= startTimer(timeout_ms);
  timeron=-1;
}

void MainWidget::animationStop()
{
  stopAnimation();
}

void MainWidget::stepforward()
{
  if (timeron) return;
  if (!m_fileNames.size()) return;
  currentIndex++;
  if (currentIndex > m_fileNames.size() - 1)
    currentIndex = 0;
  QString fileName = m_fileNames.at(currentIndex);
  m_updateOperation->updateScene(fileName.toStdString());
  setWindowTitle("osgQtWidget - " + fileName.mid(fileName.lastIndexOf('/') + 1));
}

void MainWidget::stepback()
{
  if (timeron) return;
  if (!m_fileNames.size()) return;
  currentIndex--;
  if (currentIndex < 0)
    currentIndex = m_fileNames.size() - 1;
  QString fileName = m_fileNames.at(currentIndex);
  m_updateOperation->updateScene(fileName.toStdString());
  setWindowTitle("osgQtWidget - " + fileName.mid(fileName.lastIndexOf('/') + 1));
}

void MainWidget::autoUpdate()
{
  doAutoUpdate = !doAutoUpdate;
  autoUpdateAction->setChecked(doAutoUpdate);
}

void MainWidget::setInstanceName(QString name)
{
  pluginB->setClientName(name);
}

void MainWidget::connectionClosed()
{
  // Nothing to do for the moment.
}

std::string MainWidget::getNewFile(QString & directory)
{
  QString theFile;
  QDir d(directory);
	d.setSorting(QDir::Time|QDir::Reversed);
	QStringList patterns("*.ive");
  patterns.append("*.gz");
  QFileInfoList list = d.entryInfoList(patterns);
  QStringList theFiles;
  if (list.size() > 0)
	{
    for (int k = 0; k < list.size(); k++)
		{
			QFileInfo fileInfo = list.at(k);
			theFile = fileInfo.absoluteFilePath();
      theFiles.append(theFile);
		}
  }
  // Sanity check
  if (theFiles.size() > 0) {
    QList<QString>::iterator i = m_fileNames.begin();
    for (; i != m_fileNames.end();)
    {
      if (!theFiles.contains(*i)) {
        // File removed from disk ?
        i=m_fileNames.erase(i);
      } else {
        i++;
      }
    }
  }
  return theFile.toStdString();
}

void MainWidget::processLetter(int fromId, const miQMessage &qletter)
{
  miMessage letter;
  convert(fromId, 0 /*my id, unused*/, qletter, letter);
  const QString& command = qletter.command();
  // If autoupdate is active, reread sat/radarfiles and
  // show the latest timestep
  if (command == qmstrings::directory_changed) {
    const int c_cmd = qletter.findCommonDesc("directory changed");
    QString dir = qletter.getCommonValue(c_cmd);
    std::string tmp = m_updateOperation->getNodeFileName();
    std::string currentDir = tmp.substr(0,tmp.find_last_of("/"));
    if (currentDir != dir.toStdString())
      return;
    if (doAutoUpdate) {      
      std::string new_file = getNewFile(dir);
      if (timeron) {
        // check if animation is on, insert to the end of stringlist.
        // check for duplicates
        if(!m_fileNames.contains(new_file.c_str())) {
          m_fileNames.append(new_file.c_str());
        }
        
      }
      else {
        // if not, insert at the end, and read the new model.
        // Check for duplicates
        if(!m_fileNames.contains(new_file.c_str())) {
          m_fileNames.append(new_file.c_str());
          // set the index to the next newest file
          currentIndex = m_fileNames.size() - 2;
          // Sleep for some time to avoid sync problems with the file system
          sleep(5);
          stepforward();
        }
      }
       
    }
    return;
  }

  // If autoupdate is active, do the same thing as
  // when the user presses the updateObs button.
  else if (command == qmstrings::file_changed) {
    if (doAutoUpdate) {
      // Not used for the moment.
    }
    return;
  }
  else {
    return; //nothing to do
  }
}

void MainWidget::dragEnterEvent( QDragEnterEvent *event_in )
{
    if (event_in->mimeData()->hasFormat("text/uri-list"))
        event_in->acceptProposedAction();
}

UpdateOperation::UpdateOperation()
    :osg::Operation("update operation",true)
    ,m_loadedFlag(true),m_newScene(false)
{

}

void UpdateOperation::updateScene( const std::string& name)
{
    if(m_nodeFileName!=name){
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        m_oldnodeFileName = m_nodeFileName;
        m_nodeFileName=name;
        m_loadedFlag=false;
    }
}

void UpdateOperation::operator()( osg::Object* callingObject )
{
    // decided which method to call according to whole has called me.
    // FIXME: Keep sittings from previous scene if applicable.
    
    if(m_loadedFlag) {
      if (m_newScene) {
        OsgWidget* viewer = dynamic_cast<OsgWidget*>(callingObject);
        viewer->getCameraManipulator()->computeHomePosition(viewer->getCamera(),true);
        viewer->getCameraManipulator()->home(0);
        viewer->requestRedraw();
        m_newScene=false;
        MainWidget::inUpdate = false;
      }
      return;
    }
    OsgWidget* viewer = dynamic_cast<OsgWidget*>(callingObject);
    if (viewer&&!m_nodeFileName.empty()){
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        osgDB::ReaderWriter::ReadResult r = osgDB::readNodeFile (m_nodeFileName);
        osg::ref_ptr<osg::Node> node = r.getNode();
        if(node){
            viewer->getCameraManipulator()->setNode(node);
            viewer->updateScene(node,m_oldnodeFileName,m_nodeFileName);
            m_newScene=true;
            OSG_WARN<<m_nodeFileName<<" load successfully.\n";
        }else{
            OSG_WARN<<m_nodeFileName<<" load failed.\n";
        }
        m_loadedFlag=true;
        QApplication::restoreOverrideCursor();
        
    }
}

std::string UpdateOperation::getNodeFileName()
{
  OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
  std::string tmp = m_nodeFileName;
  return tmp;
}

ViewerFrameThread::~ViewerFrameThread()
{
        cancel();
        wait();
}

int ViewerFrameThread::cancel()
{
    if (_viewerBase) _viewerBase->setDone(true);
     quit();
     return 0;
}

void ViewerFrameThread::run()
{
    if (_viewerBase) _viewerBase->run();

    if (_doQApplicationExit) QApplication::exit(0);
    exec();
}
