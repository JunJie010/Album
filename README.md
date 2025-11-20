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
***
## 关于图片显示
**LCD显示屏**为800*480像素，开发板LCD设备文件路径(/dev/fb0)

**一切皆文件：**
Linux对数据文件(*.mp3、*.bmp)，程序文件(*.c、*.h、*.o)，设备文件（LCD、触摸屏、鼠标），网络文件( socket ) 等的管理都抽象为文件，使用统一的方式方法管理；
Linux系统把每个设备都映射成一个文件，这就是设备文件。它是用于向I/O设备提供连接的一种文件，分为字符设备和块设备文件；
字符设备的存取以一个字符为单位，块设备的存取以字符块为单位。每一种I/O设备对应一个设备文件，存放在/dev目录中，如行式打印机对应/dev/lp，第一个软盘驱动器对应/dev/fd0；

在Ubuntu上编写代码，编写好代码后，在Ubuntu上交叉编译，把Ubuntu上交叉编译生产的可执行文件通过CRT上传到开发板，在开发板上执行可执行文件

### 技术栈
* FrameBuffer机制：直接操作显示内存，绕过X Window等图形服务器，实现最高效的图形渲染
* BMP格式解析：手动处理24位BMP格式，从文件头跳转到直接读取RGB数据
* 颜色空间转换：将24位RGB(888)转换为32位ARGB(8888)，适应FrameBuffer格式
* 内存映射：使用mmap将显存映射到用户空间，避免频繁的read/write系统调用
### 步骤
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

交叉编译：```arm-linux-gcc 代码文件```;交叉编译并修改可生成的执行文件：```arm-linux-gcc 代码文件 -o 目标名字```;

* 把Ubuntu上交叉编译生产的可执行文件通过CRT上传到开发板
* 在开发板上执行可执行文件

运行：```./ 可执行文件名```;
***
## 关于触摸屏交互实现
**触摸屏**尺寸为800*480，触摸屏设备文件路径(/dev/input/event0)

触摸屏通常分为电阻式触摸屏和电容式触摸屏，其中电容触摸驱动IC采用OTT2001A，最多支持208个通道，可支持IIC/SPI接口。
* **手势ID寄存器(00H)**：用于告诉MCU，哪些点有效，哪些点无效，从而读取对应的数据，如果读到的全是0，则说明没有任何触摸
* **传感器控制寄存器(ODH)**：8位寄存器，仅最高位有效，其他位保留，当最高位为 1 的时候，打开传感器（开始检测），当最高位设置为 0 的时候，关闭传感器（停止检测）
* **坐标数据寄存器**：每个坐标的值，可以通过 4 个寄存器读出

输入子系统：结构体：```input_event```
```
struct input_event
{
    struct timval time;
    _u16 type;            //设备类型：EV_ABS 触摸屏
    _u16 code;            //设备产生信息类型：x:ABS_X  y:ABS_Y 压力值：BTN_TOUCH
    _s32 value;           //信息具体的数值
}
```
### 技术栈
* Linux输入子系统：使用标准化的输入事件接口
* 坐标校准：将触摸屏原始坐标(0-1024, 0-600)映射到屏幕分辨率(800×480)
* 事件驱动架构：通过读取input_event结构体实现精确的触摸响应

### 步骤
* 输入子系统
```
struct touch_xy
{
	int x;
	int y;
};
struct input_event ts;      //Linux标准输入事件结构体
struct touch_xy tu;
```
* open打开触摸屏```int touch_fp = open("/dev/input/event0", O_RDWR);```
* read读取触摸屏```rd_num = read(touch_fp, &ts, sizeof(struct input_event));```
* 坐标映射
```
if(ts.type == EV_ABS && ts.code == ABS_X) x = ts.value*800/1024;
if(ts.type == EV_ABS && ts.code == ABS_Y) y = ts.value*480/600;
```
* 触摸释放事件
```
if (ts.type == EV_KEY && ts.code == BTN_TOUCH && ts.value == 0)
		{
			printf("松开坐标x=%d , y=%d\n", x, y);
			tu.x = x;
			tu.y = y;
			break;
		}
```
* 关闭触摸屏设备，```int cl_return = close(touch_fp);```
***
## 完成功能模块的编写
再完成了硬件模块的驱动之后，就可以实现具体功能的编写了，此处我们实现了**电子相册和幻灯片**的功能

### 触摸屏测试
```
struct touchxy tu;       //定义一个结构体存放读取触摸屏产生的坐标
if(ts.type == EV_KEY && ts.code == BTN_TOUCH && ts.value == 0)
		{
			printf("x=%d , y=%d \n",x,y);       //把x,y赋值给tu结构体的成员
			tu.x = x;
			tu.y = y;
			break;
		}  
```
通过触摸屏测试代码，可以实现串口打印触摸屏触摸位置，为应用模块打下基础
### 区域选择
- 主界面进入相册(tu.x > 580 && tu.x < 800 && tu.y > 100 && tu.y < 330)
- 主界面进入幻灯片(tu.x > 0 && tu.x < 150 && tu.y > 0 && tu.y < 150)
- 相册界面上一张(tu.x > 0 && tu.x < 90)
- 相册界面下一张(tu.x > 710 && tu.x < 800)
- 相册界面返回(tu.x > 250 && tu.x < 550 && tu.y > 360 && tu.y < 480)
- 幻灯片界面开始(tu.x > 250 && tu.x < 550 && tu.y > 0 && tu.y < 150)
- 幻灯片界面返回(tu.x > 250 && tu.x < 550 && tu.y > 360 && tu.y < 480)
### 电子相册幻灯片函数
```
void photo(int touch_fp);          //相册函数的编写
void hdp(int touch_fp);            //幻灯片函数的编写
//注意，此处只做简单展示，详细代码请看控制代码
```
***
***
后面会继续更新，请等待
