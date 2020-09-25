
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>

#define GPIOLED_CNT 3        /* 设备号个数 */
#define GPIOLED_NAME "rkled" /* 名字 */
#define LEDOFF 0             /* 关灯 */
#define LEDON 1              /* 开灯 */

const char *led_name[] = {"rkled0", "rkled1", "rkZigbeeRst"};
/* gpioled 设备结构体 */
struct gpioled_dev
{
    dev_t devid;                        /* 设备号 */
    struct cdev cdev;                   /* cdev */
    struct class *class;                /* 类 */
    struct device *device[GPIOLED_CNT]; /* 设备 */
    int major;                          /* 主设备号 */
    int minor;                          /* 次设备号 */
    struct device_node *nd;             /* 设备节点 */
    int led_gpio[GPIOLED_CNT];          /* led 所使用的 GPIO 编号 */
};

struct gpioled_dev gpioled; /* led 设备 */

/*
* @description : 打开设备
* @param – inode : 传递给驱动的 inode
* @param – filp : 设备文件， file 结构体有个叫做 private_data 的成员变量
* 一般在 open 的时候将 private_data 指向设备结构体。
* @return : 0 成功;其他 失败
*/
static int led_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &gpioled; /* 设置私有数据 */
    return 0;
}

/*
 * @description : 从设备读取数据
 * @param – filp : 要打开的设备文件(文件描述符)
 * @param - buf : 返回给用户空间的数据缓冲区
 * @param - cnt : 要读取的数据长度
 * @param – offt : 相对于文件首地址的偏移
 * @return : 读取的字节数，如果为负值，表示读取失败
 */
static ssize_t led_read(struct file *filp, char __user *buf,
                        size_t cnt, loff_t *offt)
{
    return 0;
}

/*
 * @description : 向设备写数据
 * @param - filp : 设备文件，表示打开的文件描述符
 * @param - buf : 要写给设备写入的数据
 * @param - cnt : 要写入的数据长度
 * @param – offt : 相对于文件首地址的偏移
 * @return : 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t led_write(struct file *filp, const char __user *buf,
                         size_t cnt, loff_t *offt)
{
    int retvalue;
    unsigned char recvbuf[2];
    //-----次设备号
    struct inode *m_inode;
    int minor;
    //
    struct gpioled_dev *gpiodev;

    //获取inode对象
    m_inode = filp->f_inode;
    //获取次设备号
    minor = MINOR(m_inode->i_rdev);
    printk("write minor%d\r\n", minor);

    gpiodev = filp->private_data;

    if (cnt > sizeof(recvbuf))
    {
        printk("kernel Write data length overflow!\r\n");
        return -EFAULT;
    }
    retvalue = copy_from_user(recvbuf, buf, cnt);
    if (retvalue < 0)
    {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }

    if (recvbuf[0] <= 1 && recvbuf[0] >= 0)
    {
        gpio_set_value(gpiodev->led_gpio[minor], recvbuf[0]); /* 打开 LED 灯 */
    }
    else
    {
        printk("kernel write value invalid!\r\n");
    }

    return 0;
}

/*
* @description : 关闭/释放设备
* @param – filp : 要关闭的设备文件(文件描述符)
* @return : 0 成功;其他 失败
*/
static int led_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/* 设备操作函数 */
static struct file_operations gpioled_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

/*
* @description : flatform 驱动的 probe 函数，当驱动与
* 设备匹配以后此函数就会执行
* @param - dev : platform 设备
* @return : 0，成功;其他负值,失败
*/
static int led_probe(struct platform_device *dev)
{
    int ret = 0, i;

    /* 设置 LED 所使用的 GPIO */
    /* 1、获取设备节点： gpioled */
    gpioled.nd = of_find_node_by_path("/gpio-rkled");
    if (gpioled.nd == NULL)
    {
        printk("gpioled node cant not found!\r\n");
        return -EINVAL;
    }
    else
    {
        printk("gpioled node has been found!\r\n");
    }

    for (i = 0; i < GPIOLED_CNT; i++)
    {
        /* 2、 获取设备树中的 gpio 属性，得到 LED 所使用的 LED 编号 */
        gpioled.led_gpio[i] = of_get_named_gpio(gpioled.nd, "led-gpio", i);
        if (gpioled.led_gpio[i] < 0)
        {
            printk("can't get led-gpio");
            return -EINVAL;
        }
        printk("led-gpio num = %d\r\n", gpioled.led_gpio[i]);

        ret = gpio_request(gpioled.led_gpio[i], led_name[i]);
        if (ret < 0)
        {
            printk("can't gpio_request!\r\n");
            goto fail_gpio;
        }
        /* 3、设置 GPIO1_IO03 为输出，并且输出高电平，默认关闭 LED 灯 */
        ret = gpio_direction_output(gpioled.led_gpio[i], 1);
        if (ret < 0)
        {
            printk("can't set gpio!\r\n");
            goto fail_gpio;
        }
    }

    /* 注册字符设备驱动 */
    /* 1、创建设备号 */
    if (gpioled.major)
    { /* 定义了设备号 */
        gpioled.devid = MKDEV(gpioled.major, 0);
        register_chrdev_region(gpioled.devid, GPIOLED_CNT,
                               GPIOLED_NAME);
    }
    else
    { /* 没有定义设备号 */
        alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT,
                            GPIOLED_NAME);    /* 申请设备号 */
        gpioled.major = MAJOR(gpioled.devid); /* 获取分配号的主设备号 */
        gpioled.minor = MINOR(gpioled.devid); /* 获取分配号的次设备号 */
    }
    printk("gpioled major=%d,minor=%d\r\n", gpioled.major,
           gpioled.minor);

    /* 2、初始化 cdev */
    gpioled.cdev.owner = THIS_MODULE;
    cdev_init(&gpioled.cdev, &gpioled_fops);

    /* 3、添加一个 cdev */
    cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);

    /* 4、创建类 */
    gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);
    if (IS_ERR(gpioled.class))
    {
        ret = PTR_ERR(gpioled.class);
        goto fail_class;
    }

    /* 5、创建设备 */
    for (i = 0; i < GPIOLED_CNT; i++)
    {
        gpioled.device[i] = device_create(gpioled.class, NULL,
                                          MKDEV(MAJOR(gpioled.devid), i), NULL, led_name[i]);
        if (IS_ERR(gpioled.device[i]))
        {
            ret = PTR_ERR(gpioled.device[i]);
            goto fail_device;
        }
    }
    return ret;

fail_device:
    for (i = 0; i < GPIOLED_CNT; i++)
    {
        device_destroy(gpioled.class, MKDEV(MAJOR(gpioled.devid), i));
    }
fail_class:
    class_destroy(gpioled.class);
    cdev_del(&gpioled.cdev);                              /* 删除 cdev */
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT); /* 注销 */
fail_gpio:
    for (i = 0; i < GPIOLED_CNT; i++)
    {
        gpio_free(gpioled.led_gpio[i]);
    }
    return ret;
}

/*
 * @description : remove 函数，移除 platform 驱动的时候此函数会执行
 * @param - dev : platform 设备
 * @return : 0，成功;其他负值,失败
 */
static int led_remove(struct platform_device *dev)
{
    /* 注销字符设备驱动 */
    int i;
    for (i = 0; i < GPIOLED_CNT; i++)
    {
        device_destroy(gpioled.class, MKDEV(MAJOR(gpioled.devid), i));
    }
    class_destroy(gpioled.class);

    cdev_del(&gpioled.cdev);                              /* 删除 cdev */
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT); /* 注销 */
    for (i = 0; i < GPIOLED_CNT; i++)
    {
        gpio_free(gpioled.led_gpio[i]);
    }
    return 0;
}

/* 匹配列表 */
static const struct of_device_id led_of_match[] = {
    {.compatible = "rk-led"},
    {/* Sentinel */}};

/* platform 驱动结构体 */
static struct platform_driver led_driver = {
    .driver = {
        .name = "led",                  /* 驱动名字，用于和设备匹配 */
        .of_match_table = led_of_match, /* 设备树匹配表 */
    },
    .probe = led_probe,
    .remove = led_remove,
};

/*
* @description : 驱动入口函数
* @param : 无
* @return : 无
*/
static int __init led_init(void)
{
    return platform_driver_register(&led_driver);
}

/*
* @description : 驱动出口函数
* @param : 无
* @return : 无
*/
static void __exit led_exit(void)
{
    platform_driver_unregister(&led_driver);
}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ppq");

// gpio-rkled {
// 	compatible = "rk-led";
// 	pinctrl-names = "default";
// 	pinctrl-0 = <&zigbee_pins &wifi_pins &zigbee_rst_pins>;
// 	led-gpio = <&gpio0 RK_PC0 GPIO_ACTIVE_LOW &gpio0 RK_PB7 GPIO_ACTIVE_LOW &gpio0 RK_PB6 GPIO_ACTIVE_HIGH>;
// 	status = "okay";
// };
