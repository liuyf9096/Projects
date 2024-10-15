#/bin/bash
source /etc/profile.d/tslib_env.sh
source /etc/profile.d/qt_env.sh
export LD_LIBRARY_PATH=/usr/local/mysql/lib:$LD_LIBRARY_PATH
cd /reetoo/rtshowme
./rtshowme &
