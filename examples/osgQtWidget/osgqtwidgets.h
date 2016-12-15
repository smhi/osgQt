#ifndef _OSGWIDGET_H_
#define _OSGWIDGET_H_
 
#include <QtGui/QtGui>
#include <osg/ArgumentParser>
#include <osgViewer/Viewer>
#include <osgGA/StateSetManipulator>
#include <osgQt/GraphicsWindowQt>
#include <OpenThreads/Thread>
#include <osg/Light>
#include <QThread>

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
  void updateScene(osg::Node*);
 protected:
  QWidget* addViewWidget(osg::Camera*);
  osg::Camera* createCamera( int x, int y, int w, int h );

 private:
  QWidget* m_widget;
  osg::Group* m_root;
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
  bool m_loadedFlag;
  OpenThreads::Mutex                  _mutex;
};
 
class MainWidget:public QMainWindow
{
    Q_OBJECT
 public:
  MainWidget(int argc, char *argv[]);
 protected:
 
  void dragEnterEvent(QDragEnterEvent *event);
 
  void dropEvent(QDropEvent *event);
 protected slots:
  void openFile();
  void filequit();

 private:
  QScopedPointer<ViewerFrameThread> m_viewThread;
  osg::ref_ptr<UpdateOperation> m_updateOperation;
};
#endif // _OSGWIDGET_H_
