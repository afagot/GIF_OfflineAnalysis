#!/bin/bash

export ROOTSYS=/usr/local/root/
export PATH=$ROOTSYS/bin:$PATH
export PATH=~/bin:./bin:.:$PATH
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=$ROOTSYS/lib:$DYLD_LIBRARY_PATH
export PYTHONPATH=$ROOTSYS/lib:$PYTHONPATH

cd /home/onanalysis/software/GIF_OfflineAnalysis/
bin/offlineanalysis $1
