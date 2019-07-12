#include <QTranslator>
#include <QMessageBox>

#include "diLocalSetupParser.h"
#include "miSetupParser.h"

#include <puTools/miStringFunctions.h>

#include <QDir>
#include <QInputDialog>
#include <QStringList>
#include <QString>

#include <boost/locale/generator.hpp>

#include <iostream>

#define MILOGGER_CATEGORY "osgqt.main_gui"
#include <miLogger/miLogging.h>

#include "osgqtwidgets.h"
#include <QApplication>

void printUsage()
{
  std::cout << "----------------------------------------------------------"    << std::endl
       << "osgQtWidget - a 3D presentation system for meteorological data"     << std::endl
       << "including fields, satellite- and radar3dmodels"   << std::endl
       << "Copyright (C) 2019 smhi.se" << std::endl
       << "----------------------------------------------------------" << std::endl
       << "Command line arguments:"                                    << std::endl
       << "  -h            :  Show help"                               << std::endl
       << "  -s <filename> :  name of setupfile (def. diana.setup)"    << std::endl
       << "----------------------------------------------------------" << std::endl;
}

int main(int argc, char *argv[])
{

#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
  QCoreApplication::setAttribute(Qt::AA_X11InitThreads);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
  QCoreApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);
#endif

  QApplication a(argc, argv);
  std::string setupfile;
  std::map<std::string, std::string> user_variables;

  user_variables["PVERSION"]= "1.0";
  user_variables["SYSCONFDIR"]= "/etc";
    // parsing command line arguments
  int ac= 1;
  while (ac < argc){
    const std::string sarg = argv[ac];

    if (sarg == "-h" || sarg == "--help"){
      printUsage();
      return 0;

    } else if (sarg=="-s" || sarg=="--setup") {
      ac++;
      if (ac >= argc) {
        printUsage();
        return 0;
      }
      setupfile= argv[ac];

    }
    ac++;
  } // command line parameters
  
  miutil::SetupParser::setUserVariables(user_variables);
  if (!LocalSetupParser::parse(setupfile)){
    if (setupfile.empty()) {
      METLIBS_LOG_ERROR("No setup file specified.");
      QMessageBox::critical(0, QString("osgqt %1").arg("1.0"),
        QString("No setup file specified."));
    } else {
      METLIBS_LOG_ERROR("An error occurred while reading setup: " << setupfile);
      QMessageBox::critical(0, QString("osgqt %1").arg("1.0"),
        QString("An error occurred while reading setup: %1").arg(QString::fromStdString(setupfile)));
    }
    return 0;
  }
  
  MainWidget w(argc, argv);
  w.show();
  return a.exec();
}
