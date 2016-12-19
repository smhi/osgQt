#!/bin/sh
cd `dirname $0`

# Note:Your cwd must be the dirname $0 directory when using select_gpu.sh

# The script echo the GPU number to stdout
gpu_to_use=`./select_gpu.sh`

VGLRUN=""
VGLTEST=`which vglrun 2>&1 | grep no`
if [ "x$VGLTEST" == "x" ]; then
  VGLRUN="vglrun -d :0.$gpu_to_use"
  echo "$VGLRUN"
fi

$VGLRUN ./osgQtWidget --run-max-frame-rate 10
exit
