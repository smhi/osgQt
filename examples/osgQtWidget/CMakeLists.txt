SET(SOURCES_H
  osgqtwidgets.h
  )
IF ( Qt5Widgets_FOUND )
    SET(TARGET_EXTERNAL_LIBRARIES ${QT_LIBRARIES} )
    QT5_WRAP_CPP( SOURCES_H_MOC ${SOURCES_H} OPTIONS
          "-fosgqtwidgets.h" )
    set(OSGQT_QT_VERSION 5)
ELSE()
    SET(TARGET_EXTERNAL_LIBRARIES ${QT_QTCORE_LIBRARY_RELEASE} ${QT_QTGUI_LIBRARY_RELEASE}
        ${QT_QTOPENGL_LIBRARY_RELEASE} )
    ADD_DEFINITIONS(-DUSE_QT4)
    #QT4_CREATE_MOC_COMMAND(${SOURCES_H} "moc_osgqtwidgets.cxx" "" "" "")

    QT4_WRAP_CPP( SOURCES_H_MOC ${SOURCES_H} OPTIONS "-fosgqtwidgets.h" )
    set(OSGQT_QT_VERSION 4)
ENDIF()


#IF ( NOT Qt5Widgets_FOUND )
#    IF( QT4_FOUND )
#    SET(TARGET_EXTERNAL_LIBRARIES ${QT_QTCORE_LIBRARY_RELEASE} ${QT_QTGUI_LIBRARY_RELEASE}
#        ${QT_QTOPENGL_LIBRARY_RELEASE} )
#    ADD_DEFINITIONS(-DUSE_QT4)
    #QT4_CREATE_MOC_COMMAND(${SOURCES_H} "moc_osgqtwidgets.cxx" "" "" "")

#    QT4_WRAP_CPP( SOURCES_H_MOC ${SOURCES_H} OPTIONS "-fosgqtwidgets.h" )
#    ELSE( QT4_FOUND )
#    SET(TARGET_EXTERNAL_LIBRARIES ${QT_LIBRARIES} )
#    ENDIF( QT4_FOUND )
#
#    
#ENDIF()
set(METLIBS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/local/include/metlibs)
set(MILOGGER_LIBRARY metlibs-milogger)
set(MILOGGER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/local/include/metlibs/miLogger)
set(MILOGGER_LIB_DIR ${CMAKE_SOURCE_DIR}/local/lib64)
set(COSERVER_LIBRARY metlibs-coserver-qt5)
set(COSERVER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/local/include/metlibs/coserver)
set(COSERVER_LIB_DIR ${CMAKE_SOURCE_DIR}/local/lib64)
set(PUCTOOLS_LIBRARY metlibs-puctools)
set(PUCTOOLS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/local/include/metlibs/puCtools)
set(PUCTOOLS_LIB_DIR ${CMAKE_SOURCE_DIR}/local/lib64)
set(PUTOOLS_LIBRARY metlibs-putools)
set(PUTOOLS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/local/include/metlibs/puTools)
set(PUTOOLS_LIB_DIR ${CMAKE_SOURCE_DIR}/local/lib64)

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/examples/osgQtWidget ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
             ${QT_QTOPENGL_INCLUDE_DIR} ${METLIBS_INCLUDE_DIR} ${MILOGGER_INCLUDE_DIR} ${COSERVER_INCLUDE_DIR} ${PUCTOOLS_INCLUDE_DIR} ${PUTOOLS_INCLUDE_DIR})

LINK_DIRECTORIES(${MILOGGER_LIB_DIR} ${COSERVER_LIB_DIR} ${PUCTOOLS_LIB_DIR} ${PUTOOLS_LIB_DIR})

SET(TARGET_SRC
  main.cpp
  osgqtwidgets.cpp
  diLocalSetupParser.cc
  diUtilities.cc
  miSetupParser.cc
  osgModelManager.cc
  diCommonFieldTypes.cc
  util/charsets.cc
  util/diKeyValue.cc
  util/diLineMerger.cc
  util/string_util.cc
  ${SOURCES_H_MOC}
  )
  
SET(TARGET_H
  osgqtwidgets.h
  diLocalSetupParser.h
  diUtilities.h
  miSetupParser.h
  osgModelManager.h
  diCommonFieldTypes.h  
  util/charsets.h
  util/diKeyValue.h
  util/diLineMerger.h
  util/string_util.h
  util/misc_util.h  
  )
  
SET(TARGET_IMG
  back.xpm
  forward.xpm
  loop.xpm
  slutt.xpm
  start.xpm
  stopp.xpm
  autoupdate.xpm
)


SET(TARGET_ADDED_LIBRARIES
    curl
    ${MILOGGER_LIBRARY}
    ${PUCTOOLS_LIBRARY}
    ${PUTOOLS_LIBRARY}
    ${COSERVER_LIBRARY}
    osgVolume
    osgWidget
    osgViewer
    osgUtil
    osgDB
    osgGA
    osg
    OpenThreads
    osgQt
    )
    
SET( EXAMPLE_NAME osgQtWidget )

SETUP_EXAMPLE( ${EXAMPLE_NAME} )


