%define appname SMHI-osgQt
%define name SMHI-osgQt
%define version 3.5.6
%define release 1.1
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
/usr/bin/select_gpu.sh
/usr/bin/startViewer.sh
/usr/bin/osgQtWidget
/usr/bin/osgqfont
/usr/include/osgQt/Export
/usr/include/osgQt/GraphicsWindowQt
/usr/include/osgQt/QFontImplementation
/usr/include/osgQt/QGraphicsViewAdapter
/usr/include/osgQt/QWebViewImage
/usr/include/osgQt/QWidgetImage
/usr/include/osgQt/Version
/usr/lib64/libosgQt.so
/usr/lib64/libosgQt.so.146
/usr/lib64/libosgQt.so.3.5.6
/usr/lib64/osgdb_qfont.so
/usr/lib64/pkgconfig/openscenegraph-osgQt.pc


%changelog
* Thu Dec 15 2016 Yngve Einarsson <Yngve.Einarsson@smhi.se>
- Initial build.

