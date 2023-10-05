
export BASE=$PWD
rm -f SMHI-osgQt.tar.gz
rm -rf /local_disk/build1/SOURCES/SMHI-osgQt
mkdir -p /local_disk/build1/SOURCES/SMHI-osgQt

tar cvzf SMHI-osgQt.tar.gz bin include lib64
cp SMHI-osgQt.tar.gz  /local_disk/build1/SOURCES/SMHI-osgQt

rpmbuild -bb SMHI-osgqt.spec
