#######################################################
# this are setting used in SETUP_EXAMPLE macro
#######################################################
SET(TARGET_DEFAULT_PREFIX "example_")
SET(TARGET_DEFAULT_LABEL_PREFIX "Examples")


IF(ANDROID)
    # ADD_SUBDIRECTORY(osgandroidsimple)
ELSE(ANDROID)

IF(DYNAMIC_OSGQT)

    IF   ( (QT4_FOUND OR Qt5Widgets_FOUND) AND NOT OSG_GLES1_AVAILABLE AND NOT OSG_GLES2_AVAILABLE AND NOT OSG_GL3_AVAILABLE)

        IF ( QT4_FOUND OR Qt5Widgets_FOUND)
            #ADD_SUBDIRECTORY(osgviewerQt)
            #ADD_SUBDIRECTORY(osgqfont)
            ADD_SUBDIRECTORY(osgQtWidget)
        ENDIF()

        IF ( (QT4_FOUND AND QT_QTWEBKIT_FOUND) OR Qt5WebKitWidgets_FOUND )
            #ADD_SUBDIRECTORY(osgQtWidgets)
            #ADD_SUBDIRECTORY(osgQtBrowser)
        ENDIF()

    ENDIF()

    #to add subject to find socket#ADD_SUBDIRECTORY(osgcluster)

ELSE(DYNAMIC_OSGQT)
    #needed on win32 or the linker get confused by _declspec declarations
    ADD_DEFINITIONS(-DOSGQT_LIBRARY_STATIC)

    ADD_SUBDIRECTORY(osgstaticviewer)

    # IPHONE_PORT@tom
    IF(OSG_BUILD_PLATFORM_IPHONE OR OSG_BUILD_PLATFORM_IPHONE_SIMULATOR)
        ADD_SUBDIRECTORY(osgviewerIPhone)
    ENDIF()
     # IPHONE_PORT@tom

ENDIF(DYNAMIC_OSGQT)

ENDIF(ANDROID)

