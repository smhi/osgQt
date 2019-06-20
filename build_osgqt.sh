#!/bin/sh
source /opt/rh/devtoolset-3/enable
cd `dirname $0`
pwd
hostname
if [ "x${BUILDROOT}" == "x" ]
then
  export BUILDROOT="`pwd`"
fi
mkdir -p build
mkdir -p local
cd build
make clean 
cmake3 -D CMAKE_INSTALL_PREFIX=$BUILDROOT/local -DBUILD_OSG_EXAMPLES=1 -D DESIRED_QT_VERSION=4 -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_RELWITHDEBINFO_POSTFIX="" ..
make install