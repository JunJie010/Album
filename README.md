# 基于Linux的触摸屏电子相册

## 项目预览
![项目展示](制作过程纪念/成品展示.jpg)

## 项目职责
* 通过Linux FrameBuffer机制，mmap内存映射，调用文件IO函数**在开发板上显示图片**
* 利用Linux输入子系统，捕获解析struct input_event，**实现基于坐标判别的触摸交互**
***
**本项目是一个典型的三层嵌入式GUI应用**
* **硬件层**：LCD显示设备，触摸屏
* **Linux内核层**：通过设备文件(/dev/fb0, /dev/input/event0)暴露硬件接口
* **应用层**：直接操作底层设备的C语言程序

本项目开发环境为VM虚拟机下使用Ubuntu，使用SecureCRT实现数据的串口传输

其中串口传输命令为```rx 上传的文件名```，后把传输文件拖至CRT窗口，等待发送完成即可，也可以在X/Y/Zmodem修改串口发送速度

## 关于图片显示
LCD显示屏为**800*480像素**，开发板LCD设备文件路径(/dev/fb0)

一切皆文件：
Linux对数据文件(*.mp3、*.bmp)，程序文件(*.c、*.h、*.o)，设备文件（LCD、触摸屏、鼠标），网络文件( socket ) 等的管理都抽象为文件，使用统一的方式方法管理；
Linux系统把每个设备都映射成一个文件，这就是设备文件。它是用于向I/O设备提供连接的一种文件，分为字符设备和块设备文件；
字符设备的存取以一个字符为单位，块设备的存取以字符块为单位。每一种I/O设备对应一个设备文件，存放在/dev目录中，如行式打印机对应/dev/lp，第一个软盘驱动器对应/dev/fd0；

在Ubuntu上编写代码，编写好代码后，在Ubuntu上交叉编译，把Ubuntu上交叉编译生产的可执行文件通过CRT上传到开发板，在开发板上执行可执行文件

步骤：
* 准备一张图片
* 在Ubuntu上编写代码，执行文件IO操作
```
int fp = open(path_name, O_RDONLY);                 //打开FrameBuffer设备，open函数打开图片文件
lseek(fp, 54, SEEK_SET);                            //跳过BMP文件头(14字节)和信息头(40字节)
ssize_t re_return = read(fp, buff, w * h * 3);      //read函数读取图片像素
int cl_return = close(fp);                          //使用close函数关闭图片文件
int lcd_fp = open("/dev/fb0", O_RDWR);              //使用open函数打开开发板上LCD设备文件
int* p = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fp, 0);        //内存映射
*(q + x/m + ((h-1)-y)/m * 800) = buff[3*(x+y*w)]<<0 | buff[3*(x+y*w)+1]<<8 | buff[3*(x+y*w)+2]<<16;   //RGB转ARGB
int cl_2_return = close(lcd_fp);                    //关闭LCD显示屏
//注意，此处只做简单展示，详细代码请看控制代码
```
* 在Ubuntu上交叉编译

交叉编译```arm-linux-gcc 代码文件```;交叉编译并修改可生成的执行文件```arm-linux-gcc 代码文件 -o 目标名字```;运行```./ 可执行文件名```;

* 把

