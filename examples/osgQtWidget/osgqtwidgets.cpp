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

#include <string.h>

#include <memory>

#include "back.xpm"
#include "start.xpm"
#include "stopp.xpm"
#include "slutt.xpm"
#include "loop.xpm"
#include "forward.xpm"

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


#if QT_VERSION >= 0x050000
    // Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
    osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::SingleThreaded;
#else
    osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
#endif
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

    setSceneData(m_root);
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

void OsgWidget::updateScene( osg::Node* node)
{
    osgUtil::Optimizer optimizer;
    optimizer.optimize(node);
    
    if(m_root->getNumChildren())
        m_root->replaceChild(m_root->getChild(0),node);
    else
        m_root->addChild(node);
}

MainWidget::MainWidget(int argc, char *argv[]):timeron(0),timeloop(false),currentIndex(0)
{
    // Parse the arguments and send to viewer implementation
    osg::ArgumentParser arguments(&argc,argv);
    
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
  
  timerToolbar= new QToolBar("TimerToolBar",this);
  
  timerToolbar->setObjectName("TimerToolBar");
  
  addToolBar(Qt::TopToolBarArea,timerToolbar);

  timerToolbar->addAction( timeBackwardAction    );
  timerToolbar->addAction( timeStepBackwardAction );
  timerToolbar->addAction( timeStepForewardAction );
  timerToolbar->addAction( timeForewardAction  );
  timerToolbar->addAction( timeStopAction      );
  timerToolbar->addAction( timeLoopAction        );  
  
    setWindowTitle("osgQtWidget - Press SPACE first time you have loaded a model");
}

void MainWidget::dropEvent( QDropEvent *event )
{
    if(event->mimeData()->hasFormat("text/uri-list")){
        QString fileName = event->mimeData()->urls().first().toLocalFile();
        m_updateOperation->updateScene(fileName.toStdString());
    }
}

void MainWidget::openFile()
{
    
    QStringList fileNames;
    QString fileName = m_updateOperation->getNodeFileName().c_str();
    if(!fileName.isEmpty()) {
      fileNames = QFileDialog::getOpenFileNames(this,"Open File", fileName, "Model Files (*.ive)");
    } else {
      fileNames = QFileDialog::getOpenFileNames(this,"Open File", QString(), "Model Files (*.ive)");
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

void MainWidget::stopAnimation()
{
  timeBackwardAction->setChecked( false );
  timeForewardAction->setChecked( false );

  //killTimer(animationTimer);
  timeron=0;

}

void MainWidget::animationLoop()
{
  timeloop= !timeloop;
  //tslider->setLoop(timeloop);

  timeLoopAction->setChecked( timeloop );
}

void MainWidget::timerEvent(QTimerEvent *e)
{
  /*
  if (e->timerId()==animationTimer){
    miutil::miTime t;
    if (!tslider->nextTime(timeron, t, true)){
      stopAnimation();
      return;
    }
    setPlotTime(t);
  }
  */
}

void MainWidget::animation()
{
  if (timeron!=0)
    stopAnimation();

  timeForewardAction->setChecked( true );

  //animationTimer= startTimer(timeout_ms);
  timeron=1;
}

void MainWidget::animationBack()
{
  if (timeron!=0)
    stopAnimation();

  timeBackwardAction->setChecked( true );
  /*
  tslider->startAnimation();
  animationTimer= startTimer(timeout_ms);
  */
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
  setWindowTitle("osgQtWidget - " + fileName.mid(fileName.lastIndexOf('/') + 1));
  m_updateOperation->updateScene(fileName.toStdString());
}

void MainWidget::stepback()
{
  if (timeron) return;
  if (!m_fileNames.size()) return;
  currentIndex--;
  if (currentIndex < 0)
    currentIndex = m_fileNames.size() - 1;
  QString fileName = m_fileNames.at(currentIndex);
  setWindowTitle("osgQtWidget - " + fileName.mid(fileName.lastIndexOf('/') + 1));
  m_updateOperation->updateScene(fileName.toStdString());
}

void MainWidget::dragEnterEvent( QDragEnterEvent *event )
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
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
        m_nodeFileName=name;
        m_loadedFlag=false;
    }
}

void UpdateOperation::operator()( osg::Object* callingObject )
{
    // decided which method to call according to whole has called me.
    
    if(m_loadedFlag) {
      if (m_newScene) {
        OsgWidget* viewer = dynamic_cast<OsgWidget*>(callingObject);
        viewer->getCameraManipulator()->computeHomePosition(viewer->getCamera(),true);
        viewer->getCameraManipulator()->home(0);
        m_newScene=false;
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
            viewer->updateScene(node);
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
  return m_nodeFileName;
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
