#!/bin/bash
# Get number of available gpu cores and randomly select one for use
#
# Adam Winberg

# Get number of available gpu cores
nr_of_gpucores=$(nvidia-smi -L | grep GPU | wc -l)
if [[ $nr_of_gpucores -eq 0 ]];then
  echo "Error getting gpu cores from nvidia-smi, aborting..."
  exit 2
fi


# Randomly select gpu core fore use

core_to_use=$(( ( RANDOM % $nr_of_gpucores ) ))

echo "$core_to_use"

