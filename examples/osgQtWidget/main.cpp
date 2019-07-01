#include "osgqtwidgets.h"
#include <QApplication>

int main(int argc, char *argv[])
{

#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
  QCoreApplication::setAttribute(Qt::AA_X11InitThreads);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
  QCoreApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);
#endif
  QApplication a(argc, argv);
  MainWidget w(argc, argv);
  w.show();
  return a.exec();
}
