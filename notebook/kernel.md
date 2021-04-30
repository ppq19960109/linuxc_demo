1.内核启动流程
取得压缩的uimage、zimage等
自解压缩
查找入口函数
内核引导阶段：
cpu进入svc超级用户模式
获取cpu id
检测系统是否支持此cpu
创建核心页表
使能mmu
跳转start_kernel
内核初始化阶段：
cpu相关初始化

架构相关初始化
	设备树(dtb) compatible查找内核是否支持单板
	处理bootargs放入command_line
	内存初始化
	开启MMU，创建内核页表
中断的初始化
系统调度器初始化
定时器、时钟初始化
控制台初始化
编译进内核的模块初始化
内核Cache初始化

rest_init
	创建kernel_init进程 pid=1
		启动用户进程_init 从内核态到用户态的转变
	创建kthreadd进程 pid=2 负责所有内核进程的调度和管理
cpu_idle

bootcmd:uboot默认执行命令，启动kernel相关的命令，最终调用boot、bootz、bootm
bootargs:kernel需要，console控制台参数，root：根文件系统位置，rootfstype：根文件系统类型
内核配置的CONFIG_CMDLINE参数
由bootloader传递给内核
在设备树的chosen/bootargs下描述

2.字符驱动
module_init
driver_register
probe
获取设备树io配置
申请主设备号次设备号
cdev_init
class_create
device_create

pinctrl:复用、上下拉、速度、驱动能力
gpio:输入输出