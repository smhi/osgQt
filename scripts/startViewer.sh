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
# If no DIANAROOT, use defaults
if [ "x${DIANAROOT}" == "x" ]
then
  RADAR3D=/data/24/dianasys/datadir/radar3d
  DIANAETC=/data/24/dianasys/server/local/etc/diana
  if [ "$SMHI_MODE" == "utv" ]; then
    export RADAR3D=/data/utv/diana/datadir/radar3d
    export DIANAETC=/data/utv/diana/diana_elin6/current/server/local/etc/diana
  elif [ "$SMHI_MODE" == "test" ]; then
    export RADAR3D=/data/prodtest/dianasys/datadir/radar3d
    export DIANAETC=/data/prodtest/dianasys/server/local/etc/diana
  elif [ "$SMHI_MODE" == "prod" ]; then
    export RADAR3D=/data/24/dianasys/datadir/radar3d
    export DIANAETC=/data/24/dianasys/server/local/etc/diana
  else
    export RADAR3D=/data/24/dianasys/datadir/radar3d
    export DIANAETC=/data/24/dianasys/server/local/etc/diana
  fi
else
  RADAR3D=${DIANAROOT}/datadir/radar3d
  DIANAETC=${DIANAROOT}/server/local/etc/diana
fi
$VGLRUN ./osgQtWidget --run-on-demand -s ${DIANAETC}/diana.setup-SMHI-3D
exit
