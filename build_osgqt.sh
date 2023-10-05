#!/bin/sh
cd `dirname $0`
pwd
hostname
export BUILDROOT="`pwd`"
export PKG_CONFIG_PATH=/data/proj/diana/udunits_2_2_26/pkgconfig:$BUILDROOT/local/lib64/pkgconfig:$BUILDROOT/local/lib/pkgconfig:/usr/lib64/pkgconfig
rm -rf build
mkdir -p build
mkdir -p local
# clean up
rm -rf local/bin/*
rm -rf local/include/*
rm -rf local/lib64/*
rm -rf local/share/*
mkdir -p metlibs
cd metlibs
rm -rf miLogger
git clone https://github.com/metno/metlibs-milogger miLogger
cd miLogger
cmake -DDO_NOT_USE_STD_ATOMIC=TRUE -DDO_NOT_USE_POOL_ALLOCATOR=TRUE -DCMAKE_INSTALL_PREFIX=$BUILDROOT/local -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_RELWITHDEBINFO_POSTFIX="" .
make install
cd ..
rm -rf puCtools
git clone https://github.com/metno/metlibs-puctools puCtools
cd puCtools
cmake -DCMAKE_INSTALL_PREFIX=$BUILDROOT/local -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_RELWITHDEBINFO_POSTFIX="" .
make install
cd ..
rm -rf puTools
git clone https://github.com/metno/metlibs-putools puTools
cd puTools
cmake -DCMAKE_INSTALL_PREFIX=$BUILDROOT/local -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_RELWITHDEBINFO_POSTFIX="" .
make install
cd ..
rm -rf qUtilities
git clone https://github.com/metno/metlibs-qutilities qUtilities
cd qUtilities
cmake -DCMAKE_INSTALL_PREFIX=$BUILDROOT/local -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_RELWITHDEBINFO_POSTFIX="" .
make install
cd ..
rm -rf coserver
git clone https://github.com/metno/metlibs-coserver coserver
cd coserver
cmake -DCMAKE_INSTALL_PREFIX=$BUILDROOT/local -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_RELWITHDEBINFO_POSTFIX="" .
make install
cd ..
cd ..
cd build
make clean 
cmake -D CMAKE_INSTALL_PREFIX=$BUILDROOT/local -DBUILD_OSG_EXAMPLES=1 -D DESIRED_QT_VERSION=5 -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_RELWITHDEBINFO_POSTFIX="" ..
make install
