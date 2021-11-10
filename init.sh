sudo ifconfig dpdk0 192.168.1.30 up
export RTE_SDK=`echo $PWD`/dpdk
export RTE_TARGET=x86_64-native-linuxapp-gcc
./configure --with-dpdk-lib=$RTE_SDK/$RTE_TARGET
make
