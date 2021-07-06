线程同步：互斥锁 自旋锁 读写锁，条件变量，信号量
进程间通信：管道(Pipe)和有名管道(FIFO)、信号(Signal)、消息队列(Message Queue)、共享内存(Shared Memory)、信号量(Semaphore)、套接字(Socket)
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/libu86
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/userdata:/userdata/lib
echo $LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/nfs/valgrind/lib/valgrind
export PATH=$PATH:/userdata/valgrind/bin
echo $PATH

tcpdump -i eth0 tcp port 5683 -w log.cap

sudo blkid /dev/sdb1

echo 4 4 1 7 > /proc/sys/kernel/printk

#!/bin/bash

name="hilinkapp"

while true
do
    count=`ps -ef | grep $name | grep -v "grep" | wc -l`
    if [[ $count == 0 ]];then
        echo "process not exist"
    else
        echo "process exist"
    fi
    sleep 3
done