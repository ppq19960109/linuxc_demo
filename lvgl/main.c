
#include "lvgl.h"
#include "lv_examples.h"
#include "lv_test_theme_1.h"
#include "commom.h"
#include "lv_hal_disp.h"

#define RED_COLOR888_32bpp 0x00ff0000
#define GREEN_COLOR888_32bpp 0x0000ff00
#define BLUE_COLOR888_32bpp 0x000000ff

int *screen_ptr = NULL;
int screen_width = 0;
int screen_x, screen_y;
int screen_press = 0;
struct input_event t;

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    int x, y;
    for (y = area->y1; y <= area->y2; y++)
    {
        for (x = area->x1; x <= area->x2; x++)
        {
            //set_pixel(x, y, *color_p);  /* Put a pixel to the display.*/
            *(screen_ptr + y * screen_width + x) = color_p->full;
            color_p++;
        }
    }

    lv_disp_flush_ready(disp); /* Indicate you are ready with the flushing*/
}

bool my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    // (t.type == EV_ABS&&t.code == ABS_Y)
    /*Save the state and save the pressed coordinate*/
    // printf("my_touchpad_read start:%d screen_press:%d\n", data->state, screen_press);
    data->state = screen_press == 1 ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    // printf("my_touchpad_read end:%d\n", data->state);
    //    if(data->state == LV_INDEV_STATE_PR) touchpad_get_xy(&last_x, &last_y);
    if (data->state == LV_INDEV_STATE_PR) //´¥ÃþÆÁ±»°´ÏÂ,ÅÐ¶ÏÀïµÄÄÚÈÝÒª×Ô¼ºÊµÏÖ
    {
        //TP_Draw_Big_Point(tp_dev.x[0],tp_dev.y[0],RED);		//»­Í¼
        last_x = screen_x;
        last_y = screen_y;
    }
    /*Set the coordinates (if released use the last pressed coordinates)*/
    data->point.x = last_x;
    data->point.y = last_y;

    return false; /*Return `false` because we are not buffering and no more data to read*/
}

static void BttonEventCb(lv_obj_t *obj, lv_event_t event)
{
}

void timer_thread(union sigval v)
{
    // printf("timer_thread function!\n");
    lv_tick_inc(5);
}

int timer_start()
{
    timer_t timerid;
    struct sigevent evp;
    memset(&evp, 0, sizeof(struct sigevent)); //清零初始化

    evp.sigev_value.sival_int = 111;          //也是标识定时器的，回调函数可以获得
    evp.sigev_notify = SIGEV_THREAD;          //线程通知的方式，派驻新线程
    evp.sigev_notify_function = timer_thread; //线程函数地址

    if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)
    {
        perror("fail to timer_create");
        exit(-1);
    }

    /* 第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长,就是说it.it_value变0的时候会>装载it.it_interval的值 */
    struct itimerspec it;
    it.it_interval.tv_sec = 0; // 回调函数执行频率为1s运行1次
    it.it_interval.tv_nsec = 5000000;
    it.it_value.tv_sec = 1; // 倒计时3秒开始调用回调函数
    it.it_value.tv_nsec = 0;

    if (timer_settime(timerid, 0, &it, NULL) == -1)
    {
        perror("fail to timer_settime");
        exit(-1);
    }
    return 0;
}

int main(void)
{
    int fd_fb = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    int screen_size = 0;

    int x = 0, y = 0;

    fd_fb = open("/dev/fb0", O_RDWR);
    if (!fd_fb)
    {
        printf("Error:cannot open framebuffer device.\n");
        return -1;
    }

    /* Get fixed screen info */
    if (ioctl(fd_fb, FBIOGET_FSCREENINFO, &finfo))
    {
        printf("Error reading fixed information.\n");
        return -2;
    }

    /* Get variable screen info */
    if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &vinfo))
    {
        printf("Error reading variable information.\n");
        return -3;
    }

    screen_size = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    printf("screen_size = %d, vinfo.xres = %d, vinfo.yres = %d, vinfo.bits_per_pixel = %d\n", screen_size, vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    /* map framebuffer to user memory */
    screen_ptr = (int *)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
    screen_width = vinfo.xres;

    if ((int)screen_ptr == -1)
    {
        printf("Error: failed to map framebuffer device to memory.\n");
        return -4;
    }

    if (vinfo.bits_per_pixel == 32)
    {
        printf("32 bpp framebuffer\n");
        //Red Screen
        printf("Red Screen\n");
        for (y = 0; y < vinfo.yres / 3; y++)
        {
            for (x = 0; x < vinfo.xres; x++)
            {
                *(screen_ptr + y * vinfo.xres + x) = RED_COLOR888_32bpp;
            }
        }
        //Green Screen
        printf("Green Screen\n");
        for (y = vinfo.yres / 3; y < (vinfo.yres * 2) / 3; y++)
        {
            for (x = 0; x < vinfo.xres; x++)

            {
                *(screen_ptr + y * vinfo.xres + x) = GREEN_COLOR888_32bpp;
            }
        }
        //Blue Screen
        printf("Blue Screen\n");
        for (y = (vinfo.yres * 2) / 3; y < vinfo.yres; y++)
        {
            for (x = 0; x < vinfo.xres; x++)

            {
                *(screen_ptr + y * vinfo.xres + x) = BLUE_COLOR888_32bpp;
            }
        }
    }
    else
    {
        printf("Warnning: bpp is not 16 and 24 and 32\n");
    }

    int keys_fd;

    keys_fd = open("/dev/input/event1", O_RDONLY); //打开TP设备
    if (keys_fd <= 0)
    {
        printf("open /dev/input/event0 device error!\n");
        return 0;
    }

    lv_init();
    timer_start();

    static lv_disp_buf_t disp_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * 10];                  /*Declare a buffer for 10 lines*/
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10); /*Initialize the display buffer*/

    lv_disp_drv_t disp_drv;            /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);       /*Basic initialization*/
    disp_drv.flush_cb = my_disp_flush; /*Set your driver function*/
    disp_drv.buffer = &disp_buf;       /*Assign the buffer to the display*/
    lv_disp_drv_register(&disp_drv);   /*Finally register the driver*/

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Descriptor of a input device driver*/
    indev_drv.type = LV_INDEV_TYPE_POINTER; /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = my_touchpad_read;   /*Set your driver function*/
    lv_indev_drv_register(&indev_drv);      /*Finally register the driver*/

    // lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL); //åœ¨å½“å‰screenå¯¹è±¡ä¸Šåˆ›å»ºbtn1
    // lv_obj_set_event_cb(btn1, BttonEventCb);            //è®¾ç½®å½“å‰æŒ‰é”®çš„ äº‹ä»¶å›žè°ƒå‡½æ•°
    // lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, -40);  //é…ç½®btn1åœ¨å…¶çˆ¶ç±»ï¼ˆscreenï¼‰ä¸­æ˜¾ç¤ºä½ç½®

    // lv_obj_t *label1;                     //åˆ›å»ºæŒ‰é”®1ä¸Šé¢æ˜¾ç¤ºçš„label1
    // label1 = lv_label_create(btn1, NULL); //åœ¨çˆ¶ç±»btn1ä¸­åˆ›å»ºlabel1å®žä½“
    // lv_label_set_text(label1, "Button");  //é…ç½®labelçš„text

	lv_theme_t * th = lv_theme_night_init(20, NULL);
	lv_test_theme_1(th);


int flags=fcntl(keys_fd,F_GETFL,0); 
flags |=O_NONBLOCK;
 
fcntl(keys_fd,F_SETFL,flags);
    fd_set fds;
    // struct timeval timeout = {0, 0}; //select等待3秒，3秒轮询，要非阻塞就置0
    while (1)
    {
        //  printf("while\n");
        FD_ZERO(&fds);         //每次循环都要清空集合，否则不能检测描述符变化
        FD_SET(keys_fd, &fds); //添加描述符
        // if (select(keys_fd + 1, &fds, NULL, NULL, &timeout))
        {
            if (read(keys_fd, &t, sizeof(t)) == sizeof(t))
            {
                if (t.type == EV_KEY)
                {
                    printf("  type: EV_KEY, event = %s, value = %d \r\n",
                           t.code == BTN_TOUCH ? "BTN_TOUCH" : "Unkown", t.value);
                    screen_press = t.value;
                }
                else if (t.type == EV_ABS)
                {
                    // printf("  type: EV_ABS, event = %s, value = %d \r\n",
                    //        t.code == ABS_X ? "ABS_X" : t.code == ABS_Y ? "ABS_Y" : t.code == ABS_PRESSURE ? "ABS_PRESSURE" : "Unkown", t.value);
                    if (t.code == ABS_X)
                        screen_x = t.value;
                    else if (t.code == ABS_Y)
                        screen_y = t.value;
                }
            }
        }
        lv_task_handler();
    }

    munmap(screen_ptr, screen_size);
    close(fd_fb);
    close(keys_fd);
    return 0;
}
