# /bin/sh

task_nums=`expr $(cat /proc/cpuinfo | grep processor | wc -l) \* 2`

#build protobuf

#build example
test -d tmp || mkdir tmp
cd tmp
cmake .. && make -j $task_nums || exit -1
cd -