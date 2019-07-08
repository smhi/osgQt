#ifndef _OSGWIDGET_H_
#define _OSGWIDGET_H_
 
//#include <QtGui/QtGui>
#include <osg/ArgumentParser>
#include <osgViewer/Viewer>
#include <osgGA/StateSetManipulator>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/PolygonMode>
#include <osgUtil/LineSegmentIntersector>
#include <osgQt/GraphicsWindowQt>
#include <OpenThreads/Thread>
#include <osg/Light>
#include <QMainWindow> 
#include <QThread>
#include <QStringList>
#include <QTimer>
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QToolButton>
#include <QDropEvent>
#include <QFileDialog>
#include <QApplication>
#include <QLabel>
#include <QScrollArea>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <ClientSelection.h>
#include "osgModelManager.h"
#include "friendlyDialog.h"

// Thread that runs the viewer's frame loop as we can't run Qt in the background... 
class ViewerFrameThread : public QThread
{
 public:
 
  ViewerFrameThread(osgViewer::ViewerBase* viewerBase, bool doQApplicationExit):
     _viewerBase(viewerBase),
     _doQApplicationExit(doQApplicationExit) {}
 
  ~ViewerFrameThread();
  int cancel();
  void run();
  osg::ref_ptr<osgViewer::ViewerBase> _viewerBase;
  bool _doQApplicationExit;
};
 
class OsgWidget :public osgViewer::Viewer
{
 public:
 
  OsgWidget(osg::ArgumentParser& arguments, QWidget *parent = NULL);
  virtual ~OsgWidget(void) {}
 
  QWidget* getWidget();
  void updateScene(osg::Node*, std::string oldFileName, std::string newFileName);
 protected:
  QWidget* addViewWidget(osg::Camera*,osgGA::CameraManipulator* manipulator);
  osg::Camera* createCamera( int x, int y, int w, int h );

 private:
  QWidget* m_widget;
  osg::ref_ptr<osg::Group> m_root;
  osg::Light* myLight1;
};
 
class UpdateOperation : public osg::Operation
{
 public:
  UpdateOperation();
  void updateScene(const std::string&);
  void operator () (osg::Object* callingObject);
  std::string getNodeFileName(); 
 private:
  std::string m_nodeFileName;
  std::string m_oldnodeFileName;
  bool m_loadedFlag;
  bool m_newScene;
  OpenThreads::Mutex                  _mutex;
};
 
class MainWidget : public QMainWindow
{
    Q_OBJECT
 public:
  MainWidget(int argc, char *argv[]);
  static bool inUpdate;
  ModelManager * getManager() {return m_ModelManager;};
  virtual ~MainWidget(void) {if (m_ModelManager) delete m_ModelManager;};
 protected:
 
  void dragEnterEvent(QDragEnterEvent *event);
 
  void dropEvent(QDropEvent *event);
 protected slots:
  void openFile();
  void fileQuit();

  void openFriendlyDialog(); //open the friendlydialog
  //void hideFriendlyDialog();

  void showHelp();
  void stopAnimation();
  void animationLoop();

  void timerEvent(QTimerEvent *e);
  void animation();
  void animationBack();
  void animationStop();
  void stepforward();
  void stepback();
  void autoUpdate();
  void connectionClosed();
  void processLetter(int fromId, const miQMessage&);
  void setInstanceName(QString instancename);
 private:
  std::string getNewFile(QString & directory);
  QScopedPointer<ViewerFrameThread> m_viewThread;
  osg::ref_ptr<UpdateOperation> m_updateOperation;
  int timeron;
  bool timeloop;
  bool doAutoUpdate;
  int animationTimer;        ///> the main timer id
  int timeout_ms;            ///> animation timeout in millisecs
  int currentIndex;
  QStringList m_fileNames;
  QToolBar * timerToolbar;
  QAction * openAct;
  QAction * fileQuitAction;
  
  QAction * friendlyDialogAction; //friendlyfileloader UI Action
  
  QAction * helpDocAction;
  QAction * timeBackwardAction;
  QAction * timeForewardAction;
  QAction * timeStepBackwardAction;
  QAction * timeStepForewardAction;
  QAction * timeStopAction;
  QAction * timeLoopAction;
  QAction * autoUpdateAction;
  // Connect to filewatcher/coserver
  ClientSelection   * pluginB;
  ModelManager * m_ModelManager;
  friendlyDialog * m_friendlyDialog;
};
#endif // _OSGWIDGET_H_
