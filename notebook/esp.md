[share]
   comment = share folder
   browseable = yes
   path = /home/ppq/share
   create mask = 0777
   directory mask = 0777
   public = yes
   writeable = yes
   available = yes
   
 http://mcook.marssenger.com?mcu_current_version=1&model=Q6&timestamp=1&wifi_mac=1234567890
$IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/ttyS0 -b 921600 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x30000 build/app-x7.bin 0x3a2000 demo_anyone_can_use.bin

idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py -p /dev/ttyUSB0 flash
idf.py -p /dev/ttyUSB0 monitor
clear && idf.py -p /dev/ttyUSB0 flash monitor
clear && idf.py -p /dev/ttyUSB0 app-flash monitor
clear && idf.py -p /dev/ttyUSB0 monitor

idf.py app
idf.py app-flash  
idf.py clean
idf.py fullclean


/home/ppq/git/esp-idf4.4/tools/xtensa-esp32-elf/esp-2021r2-8.4.0/xtensa-esp32-elf/bin/xtensa-esp32-elf-addr2line -pfiaC -e build/esp-uart.elf 0x40081c72:0x3ffec1300x400915fd:0x3ffec150 0x40098a65:0x3ffec170 0x40092a46:0x3ffec290 0x400fcf11:0x3ffec2b0 0x4012688c:0x3ffec2d0 0x401268e4:0x3ffec2f0 0x4010ca16:0x3ffec310 0x40112dfb:0x3ffec330 0x4011319b:0x3ffec350 0x400fcbe9:0x3ffec370 0x40095039:0x3ffec390

$IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/ttyUSB0 -b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x24000 build/ota_data_initial.bin 0x30000 build/app-x7.bin 0x3a2000 testx7.bin
e6 e6 00 01 01 00 00 01 02 6e 6e
e6 e6 00 02 39 00 04 02 67 14 13 01 02 6e 6e
export IDF_TOOLS_PATH=$HOME/git/esp-idf4.3
export IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"
./install.sh

alias espidf4.3-export='export IDF_TOOLS_PATH=$HOME/git/esp-idf4.3;. $HOME/git/esp-idf4.3/esp-idf/export.sh'

$HOME/git/esp-idf4.3/esp-idf/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate iot_test_01.csv iot_test_01.bin 0x4000
$IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate test.csv test1.bin 0x4000
$IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py --input testq6.csv --output testq6.bin --size 0x4000
$IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/ttyUSB0 -b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x11000 build/ota_data_initial.bin 0x20000 build/esp-uart.bin 0x2a0000 test1029.bin
$IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/ttyUSB0 -b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x11000 build/ota_data_initial.bin 0x20000 build/esp-uart.bin 0x2a0000 test1021.bin
$IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/ttyUSB0 -b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x11000 build/ota_data_initial.bin 0x20000 build/esp-uart.bin 0x360000 testq6bc.bin
$IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/ttyUSB0 -b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x11000 build/ota_data_initial.bin 0x20000 build/esp-uart.bin 0x360000 X502_test1.bin

$IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/ttyUSB0 -b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x24000 build/ota_data_initial.bin 0x30000 build/d50.bin 0x3a2000 x502_test1.bin
$IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/ttyUSB0 -b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x24000 build/ota_data_initial.bin 0x30000 build/X7BC.bin 0x3a2000 test1.bin

export IDF_TOOLS_PATH=$HOME/git/esp-idf3.3
export IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"
./install.sh
alias espidf3.3-export='export IDF_TOOLS_PATH=~/git/esp-idf3.3;. $HOME/git/esp-idf3.3/esp-idf/export.sh'

git clone -b release/v3.3 --recursive git@github.com:espressif/esp-idf.git

git checkout 0b71a0a46d67cce681ec55973b020d950d8596bd
git reset --hard 0b71a0a46d67cce681ec55973b020d950d8596bd

alias python='/usr/bin/python3.6'



sudo apt-get install git-core gnupg flex bison gperf build-essential zip curl zlib1g-dev gcc-multilib g++-multilib libc6-dev-i386 lib32ncurses5-dev x11proto-core-dev libx11-dev lib32z1-dev ccache libgl1-mesa-dev libxml2-utils xsltproc unzip device-tree-compiler liblz4-tool

sudo apt-get install libfile-which-perl sed make binutils gcc g++ bash patch gzip bzip2 perl tar cpio python unzip rsync file bc libmpc3 git repo texinfo pkg-config cmake tree texinfo

export IDF_TOOLS_PATH=~/git/q6
export IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"
./install.sh
export IDF_PATH=~/git/q6/esp-adf/esp-idf
export ADF_PATH=~/git/q6/esp-adf
$IDF_PATH/components/esptool_py/esptool/esptool.py -p  /dev/ttyUSB0 -b 460800 --after hard_reset write_flash --flash_mode dio --flash_size detect --flash_freq 80m 0x1000 build/bootloader/bootloader.bin 0xa000 build/partition_table/partition-table.bin 0xe000 build/ota_data_initial.bin 0x10000 build/Q6.bin 0x490000 testq6.bin

e6 e6 00 02 3a 00 02 f1  03 49 c6 6e 6e 
e6 e6 00 04 3a 00 03 22  00 6f 34 5f 6e 6e
[wrn] iotx_cloud_conn_mqtt_event_handle(178): bypass 139 bytes on [/sys/a11kDm6Qj30/testx7/_thing/event/notify]

大锅干烧 max:36.30 min:27.00
多水干烧 max:7.30 min:1.30
少水干烧 max:11.60 min:1.50
无水干烧 max:37.00 min:18.60
$IDF_PATH/components/spiffs/spiffsgen.py 0x4000 spiffs_image spiffs.bin 
$IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/ttyUSB0 -b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x11000 build/ota_data_initial.bin 0x20000 build/esp-uart.bin 0x364000 spiffs.bin

Property Set Received, Devid: 0, Request: {"CookbookParam":[{"RemindText":"","FanTime":0,"Temp":100,"Timer":20,"Valid":1,"SteamTime":0,"Mode":1,"WaterTime":0,"Paused":0},{"RemindText":"","FanTime":0,"Temp":100,"Timer":20,"Valid":1,"SteamTime":0,"Mode":1,"WaterTime":0,"Paused":0}],"CookbookName":"好想","CookbookIntro":{"Steps":"","Intro":"","Materials":""}}
I (26081) linkkit_solo: Property Set Received, Devid: 0, Request: {"MultiMode":2,"StOvOperation":0}

e6 e6 00 04 03 00 2f f6  00 05 01 0a 00 00 00 02 0b 02 13 02 14 00 31 01  32 00 34 00 38 03 52 05 53 00 64 54 00 61 55 00  1e 56 00 1e 57 00 00 58 00 00 59 00 60 01 5b b0  6e 6e e6 e6 00 04 03 00 0b f6 00 05 01 0a 00 00  00 02 0b 02 02 73 6e 6e

e6 e6 00 05 0d 00 5a 01  02 02 01 03 02 04 01 05 01 0a 00 00 00 02 0b 02  11 00 12 00 13 02 14 00 31 01 32 00 34 00 38 03  42 00 43 00 44 00 00 45 00 38 46 00 00 47 00 00  48 00 00 49 00 00 4a 00 4b 00 4f 00 50 00 00 00  00 52 05 53 00 64 54 00 61 55 00 1e 56 00 1e 57  00 00 58 00 00 59 00 60 01 74 19 6e 6e

e6 e6 00 05 03 00 23 f6  00 05 01 0a 00 00 00 00 0b 00 52 03 53 00 64 54  00 61 55 00 1e 56 00 1e 57 00 00 58 00 00 59 00  60 01 d8 0a 6e 6e e6 e6 00 05 03 00 0b f6 00 05  01 0a 00 00 00 00 0b 00 03 92 6e 6e

