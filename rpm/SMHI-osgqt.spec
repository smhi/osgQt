%define appname SMHI-osgQt
%define name SMHI-osgQt
%define version 3.6.3
%define release 1.6
%define vendor	smhi.se

Summary: Systemd unit file for %{appname}
Name: %{appname}
Version: %{version}
Release: %{release}
License: SMHI
Group: Utilities
URL: http://www.smhi.se/
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot

%description
osgQt lib and executables %{appname}

%prep

%build

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr
cd $RPM_BUILD_ROOT/usr
tar xvzf $RPM_SOURCE_DIR/SMHI-osgQt.tar.gz

%clean
rm -rf $RPM_BUILD_ROOT

%post

%preun

%files
%defattr(-,root,root,-)
/usr/bin/osgQtWidget
   /usr/bin/select_gpu.sh
   /usr/bin/startViewer.sh
   /usr/include/metlibs/coserver/ClientButton.h
   /usr/include/metlibs/coserver/ClientSelection.h
   /usr/include/metlibs/coserver/CoClient.h
   /usr/include/metlibs/coserver/QLetterCommands.h
   /usr/include/metlibs/coserver/coserverVersion.h
   /usr/include/metlibs/coserver/miMessage.h
   /usr/include/metlibs/coserver/miMessageIO.h
   /usr/include/metlibs/miLogger/miLoggerVersion.h
   /usr/include/metlibs/miLogger/miLogging.h
   /usr/include/metlibs/miLogger/miLoggingLog4Cpp.h
   /usr/include/metlibs/miLogger/miLoggingLogger.h
   /usr/include/metlibs/miLogger/miLoggingNone.h
   /usr/include/metlibs/miLogger/miLoggingScopes.h
   /usr/include/metlibs/miLogger/miLoggingSimple.h
   /usr/include/metlibs/miLogger/miLoggingStringRecord.h
   /usr/include/metlibs/miLogger/miLoggingSystem.h
   /usr/include/metlibs/miLogger/miLoggingUtils.h
   /usr/include/metlibs/puCtools/bool.h
   /usr/include/metlibs/puCtools/deprecated.h
   /usr/include/metlibs/puCtools/getopt.h
   /usr/include/metlibs/puCtools/glob_cache.h
   /usr/include/metlibs/puCtools/mkdir.h
   /usr/include/metlibs/puCtools/porttypes.h
   /usr/include/metlibs/puCtools/puCglob.h
   /usr/include/metlibs/puCtools/puCtools.h
   /usr/include/metlibs/puCtools/puCtoolsVersion.h
   /usr/include/metlibs/puCtools/puMath.h
   /usr/include/metlibs/puCtools/safety.h
   /usr/include/metlibs/puCtools/sleep.h
   /usr/include/metlibs/puCtools/stat.h
   /usr/include/metlibs/puCtools/stringmanip.h
   /usr/include/metlibs/puCtools/strlcpy.h
   /usr/include/metlibs/puCtools/timemanip.h
   /usr/include/metlibs/puTools/TimeFilter.h
   /usr/include/metlibs/puTools/miClock.h
   /usr/include/metlibs/puTools/miCommandLine.h
   /usr/include/metlibs/puTools/miDate.h
   /usr/include/metlibs/puTools/miDirtools.h
   /usr/include/metlibs/puTools/miRing.h
   /usr/include/metlibs/puTools/miSort.h
   /usr/include/metlibs/puTools/miString.h
   /usr/include/metlibs/puTools/miStringBuilder.h
   /usr/include/metlibs/puTools/miStringFunctions.h
   /usr/include/metlibs/puTools/miTime.h
   /usr/include/metlibs/puTools/minmax.h
   /usr/include/metlibs/puTools/puAlgo.h
   /usr/include/metlibs/puTools/puMathAlgo.h
   /usr/include/metlibs/puTools/puToolsVersion.h
   /usr/include/metlibs/puTools/ttycols.h
   /usr/include/metlibs/qUtilities/miLogFile.h
   /usr/include/metlibs/qUtilities/miLoggingQt.h
   /usr/include/metlibs/qUtilities/miSliderWidget.h
   /usr/include/metlibs/qUtilities/qUtilitiesVersion.h
   /usr/include/metlibs/qUtilities/qtHelpDialog.h
   /usr/include/osgQt/Export
   /usr/include/osgQt/GraphicsWindowQt
   /usr/include/osgQt/QFontImplementation
   /usr/include/osgQt/QGraphicsViewAdapter
   /usr/include/osgQt/QWebViewImage
   /usr/include/osgQt/QWidgetImage
   /usr/include/osgQt/Version
   /usr/lib64/libmetlibs-coserver-qt5.so
   /usr/lib64/libmetlibs-coserver-qt5.so.3
   /usr/lib64/libmetlibs-coserver-qt5.so.3.0.2
   /usr/lib64/libmetlibs-milogger.so
   /usr/lib64/libmetlibs-milogger.so.6
   /usr/lib64/libmetlibs-milogger.so.6.0.3
   /usr/lib64/libmetlibs-puctools.so
   /usr/lib64/libmetlibs-puctools.so.6
   /usr/lib64/libmetlibs-puctools.so.6.0.0
   /usr/lib64/libmetlibs-putools.so
   /usr/lib64/libmetlibs-putools.so.8
   /usr/lib64/libmetlibs-putools.so.8.1.1
   /usr/lib64/libmetlibs-qutilities-qt5.so
   /usr/lib64/libmetlibs-qutilities-qt5.so.8
   /usr/lib64/libmetlibs-qutilities-qt5.so.8.0.0
   /usr/lib64/libosgQt.so
   /usr/lib64/libosgQt.so.158
   /usr/lib64/libosgQt.so.3.6.3
   /usr/lib64/pkgconfig/coserver.pc
   /usr/lib64/pkgconfig/metlibs-coserver-qt5.pc
   /usr/lib64/pkgconfig/metlibs-milogger.pc
   /usr/lib64/pkgconfig/metlibs-puctools.pc
   /usr/lib64/pkgconfig/metlibs-putools.pc
   /usr/lib64/pkgconfig/metlibs-qutilities-qt5.pc
   /usr/lib64/pkgconfig/miLogger.pc
   /usr/lib64/pkgconfig/openscenegraph-osgQt.pc
   /usr/lib64/pkgconfig/puCtools.pc
   /usr/lib64/pkgconfig/puTools.pc
   /usr/lib64/pkgconfig/qUtilities.pc


%changelog
* Wed Jul 10 2019 Yngve Einarsson <Yngve.Einarsson@smhi.se>
- OpenSceneGraph 3.6.3, Qt 5.8, setupfile, select referencetime i dialog like filedialog in Diana
* Mon Apr 23 2018 Yngve Einarsson <Yngve.Einarsson@smhi.se>
- OpenSceneGraph 3.6.0, remember volume properties when loading volumes in sequence
* Thu Apr 6 2017 Yngve Einarsson <Yngve.Einarsson@smhi.se>
- OpenSceneGraph update
* Tue Feb 28 2017 Yngve Einarsson <Yngve.Einarsson@smhi.se>
- Automatic updates implemented, using the metlibs coserver library.
* Mon Dec 19 2016 Yngve Einarsson <Yngve.Einarsson@smhi.se>
- Initial build.
- Step, sequence, drag and drop and help.

