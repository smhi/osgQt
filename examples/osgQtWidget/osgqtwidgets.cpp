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

    if (arguments.argc()<=1)
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
    m_widget=addViewWidget(createCamera(0,0,100,100));
    
    addEventHandler(new osgViewer::StatsHandler());
    addEventHandler( new osgGA::StateSetManipulator(getCamera()->getOrCreateStateSet()) );
    //setCameraManipulator( keyswitchManipulator.get() );
    
    // add the thread model handler
    addEventHandler(new osgViewer::ThreadingHandler);

    // add the window size toggle handler
    // addEventHandler(new osgViewer::WindowSizeHandler);

    // add the help handler
    // addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

    // add the record camera path handler
    addEventHandler(new osgViewer::RecordCameraPathHandler);

    // add the LOD Scale handler
    addEventHandler(new osgViewer::LODScaleHandler);

    // add the screen capture handler
    addEventHandler(new osgViewer::ScreenCaptureHandler);

    setSceneData(m_root);
}

QWidget* OsgWidget::addViewWidget(osg::Camera* camera)
{
    setCamera( camera );
    setCameraManipulator( new osgGA::TrackballManipulator() );

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
        getCameraManipulator()->home(0);
}

MainWidget::MainWidget(int argc, char *argv[])
{
    // Parse the arguments and send to viewer implementation
    osg::ArgumentParser arguments(&argc,argv);
    
    OsgWidget* viewer=new OsgWidget(arguments);
    QWidget* widget=viewer->getWidget();
    widget->setMinimumSize(QSize(100,100));
    setCentralWidget(widget);
    m_viewThread.reset(new ViewerFrameThread(viewer,true));
    m_updateOperation=new UpdateOperation();
    viewer->addUpdateOperation(m_updateOperation);
    m_viewThread->start();
    setAcceptDrops(true);

    QMenu* fileMenu = menuBar()->addMenu("&File");
    QAction* openAct = new QAction("&Open",this);
    fileMenu->addAction(openAct);
    connect(openAct,SIGNAL(triggered()),this,SLOT(openFile()));
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
    const QString fileName = QFileDialog::getOpenFileName(this,
                                                          "Open File", QString(), "STL Files (*.*)");
    m_updateOperation->updateScene(fileName.toStdString());
}

void MainWidget::dragEnterEvent( QDragEnterEvent *event )
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

UpdateOperation::UpdateOperation()
    :osg::Operation("update operation",true)
    ,m_loadedFlag(true)
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
    if(m_loadedFlag)
        return;
    OsgWidget* viewer = dynamic_cast<OsgWidget*>(callingObject);
    if (viewer&&!m_nodeFileName.empty()){
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        osgDB::ReaderWriter::ReadResult r = osgDB::readNodeFile (m_nodeFileName);
        osg::ref_ptr<osg::Node> node = r.getNode();
        if(node){
            viewer->updateScene(node);
            OSG_WARN<<m_nodeFileName<<" load successfully.\n";
        }else{
            OSG_WARN<<m_nodeFileName<<" load failed.\n";
        }
        m_loadedFlag=true;
    }
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
