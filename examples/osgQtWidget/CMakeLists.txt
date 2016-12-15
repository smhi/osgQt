SET(SOURCES_H
  osgqtwidgets.h
  )

IF ( NOT Qt5Widgets_FOUND )
    IF( QT4_FOUND )
    SET(TARGET_EXTERNAL_LIBRARIES ${QT_QTCORE_LIBRARY_RELEASE} ${QT_QTGUI_LIBRARY_RELEASE}
        ${QT_QTOPENGL_LIBRARY_RELEASE} )
    ADD_DEFINITIONS(-DUSE_QT4)
    #QT4_CREATE_MOC_COMMAND(${SOURCES_H} "moc_osgqtwidgets.cxx" "" "" "")

    QT4_WRAP_CPP( SOURCES_H_MOC ${SOURCES_H} OPTIONS "-fosgqtwidgets.h" )
    ELSE( QT4_FOUND )
    SET(TARGET_EXTERNAL_LIBRARIES ${QT_LIBRARIES} )
    ENDIF( QT4_FOUND )

    INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
             ${QT_QTOPENGL_INCLUDE_DIR} )
ENDIF()

SET(TARGET_SRC
  main.cpp
  osgqtwidgets.cpp
  ${SOURCES_H_MOC}
  )
  
SET(TARGET_H
  osgqtwidgets.h
  )


SET(TARGET_ADDED_LIBRARIES
    osgWidget
    osgViewer
    osgUtil
    osgDB
    osgGA
    osg
    OpenThreads
    osgQt)
    
SET( EXAMPLE_NAME osgQtWidget )

SETUP_EXAMPLE( ${EXAMPLE_NAME} )


