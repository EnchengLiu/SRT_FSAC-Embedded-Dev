[needed_library]
1. utility: lib_emuc2_32.a & lib_emuc2_64.a
2. multi-lib: $ sudo apt-get install gcc-multilib

[notice]
1. build with root

[build]
(use root)
1. $ make clean
2. $ make

[install]
1. sudo insmod emuc2socketcan.ko
2. sudo ./emucd_64 -s9 ttyACM0
3. sudo ip link set emuccan0 up qlen 1000
4. sudo ip link set emuccan1 up qlen 1000

[test]
1. use open source can-utils
$ sudo apt install can-utils

[remove]
1. sudo pkill -2 emucd_64
2. sudo rmmod emuc2socketcan.ko

[notice]
1. driver version >= 2.5; utility version >= 2.7
    EMUC device will auto active after setting up "two" CAN port