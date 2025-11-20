#include<stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/input.h>

struct touch_xy
{
	int x;
	int y;
};
struct input_event ts;
struct touch_xy tu;
  
char *bmps_buff[] = {"/root/bmp/sun.bmp","/root/bmp/water.bmp","/root/bmp/gold.bmp","/root/bmp/earth.bmp","/root/bmp/fire.bmp","/root/bmp/wood.bmp","/root/bmp/soil.bmp"};


void showup(char* path_name, int zs_x, int zs_y, int w, int h, int m);
int touch_init(void);
void touch(int touch_fp);
void touch_end(int touch_fp);
void photo(int touch_fp);
void hdp(int touch_fp);
unsigned int sleep(unsigned int second);

void main()
{
        int touch_fp =  touch_init();
	while(1)
        {
              showup("/root/bmp/zhu.bmp",0,0,800,480,1);
              touch(touch_fp);
	      if(tu.x > 580 && tu.x < 800 && tu.y > 100 && tu.y < 330)
              {
                 printf("开始相册\n");
 		 photo(touch_fp);
              }
	      else if(tu.x > 0 && tu.x < 150 && tu.y > 0 && tu.y < 150)
	      {
	      	 printf("开始幻灯片");
		 hdp(touch_fp);
	      }
	      
              

	}
        void touch_end(int touch_fp);
}

//第一步：显示主界面函数
void showup(char* path_name, int zs_x, int zs_y, int w, int h, int m)
{
	int fp = open(path_name, O_RDONLY);
	if (fp != -1) {
		printf("打开%s成功\n", path_name);
	}
	else {
		printf("打开%s失败\n", path_name);
	}

	lseek(fp, 54, SEEK_SET);

	char buff[w * h * 3];
	ssize_t re_return = read(fp, buff, w * h * 3);
	if (re_return != -1) {
		printf("读取%s成功\n", path_name);
	}
	else {
		printf("读取%s失败\n", path_name);
	}

	int cl_return = close(fp);
	if (cl_return != -1) {
		printf("关闭%s成功\n", path_name);
	}
	else {
		printf("关闭%s失败\n", path_name);
	}

	int lcd_buff[w * h];
	int lcd_fp = open("/dev/fb0", O_RDWR);
	if (lcd_fp != -1) {
		printf("打开lcd成功\n");
	}
	else {
		printf("打开lcd失败\n");
	}

	int* p = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fp, 0);
	if (p != MAP_FAILED) {
		printf("映射成功\n");
	}
	else {
		printf("映射失败\n");
	}

	int* q = p + (zs_x + zs_y * 800);

	int x, y;
	for (x = 0; x < w; x += m)
	{
		for (y = 0; y < h; y += m)
		{
			*(q + x / m + ((h - 1) - y) / m * 800) = buff[3 * (x + y * w)] << 0 | buff[3 * (x + y * w) + 1] << 8 | buff[3 * (x + y * w) + 2] << 16;
		}
	}

	int cl_2_return = close(lcd_fp);
	if (cl_2_return != -1) {
		printf("关闭lcd成功\n");
	}
	else {
		printf("关闭lcd失败\n");
	}

	int mun_ret = munmap(p, 800 * 480 * 4);
	if (mun_ret != -1) {
		printf("取消映射成功\n");
	}

}

int touch_init(void)
{
	int touch_fp = open("/dev/input/event0", O_RDWR);
	if (touch_fp != -1) {
		printf("打开触摸屏成功\n");
	}
	else {
		printf("打开触摸屏失败\n");
	}
	return touch_fp;
}

void touch(int touch_fp)
{
	int x, y;
	ssize_t rd_num;

	while (1)
	{
		rd_num = read(touch_fp, &ts, sizeof(struct input_event)); 
		if (rd_num != -1) {
			printf("读取触摸屏成功\n");
		}
		else {
			printf("读取触摸屏失败\n");
		}

		if(ts.type == EV_ABS && ts.code == ABS_X) x = ts.value*800/1024;
		if(ts.type == EV_ABS && ts.code == ABS_Y) y = ts.value*480/600;

		if (ts.type == EV_KEY && ts.code == BTN_TOUCH && ts.value == 0)
		{
			printf("松开坐标x=%d , y=%d\n", x, y);
			tu.x = x;
			tu.y = y;
			break;
		}
	}
}

void touch_end(int touch_fp)
{
	int cl_return = close(touch_fp);
	if(cl_return != -1){
		printf("关闭触摸屏成功\n");
	}else{
		printf("关闭触摸屏失败\n");
	}
}

void photo(int touch_fp)
{
	int bmp_num = 0;
	showup("/root/bmp/sun.bmp",0,0,800,480,1);
	while(1)
	{
		
		touch(touch_fp);
                if(tu.x > 0 && tu.x < 90)
		{
			printf("上一张\n");
			bmp_num--;
			if(bmp_num < 0)
    			{
	     		   bmp_num = 6;
			}
			showup(bmps_buff[bmp_num],0,0,800,480,1);
		}
		else if(tu.x > 710 && tu.x < 800)
		{
			printf("下一张\n");
			bmp_num++;
			if(bmp_num > 6)
    			{
	     		   bmp_num = 0;
			}
			showup(bmps_buff[bmp_num],0,0,800,480,1);
		}
		else if(tu.x > 250 && tu.x < 550 && tu.y > 360 && tu.y < 480)
		{
			printf("返回");
			break;
		}
	}
        
}

//幻灯片界面
void hdp(int touch_fp)
{
	showup("/root/bmp/soil.bmp",0,0,800,480,1);
	while(1)
	{
		touch(touch_fp);
		if(tu.x > 250 && tu.x < 550 && tu.y > 0 && tu.y < 150)
		{
			int i;
			for(i=0;i<7;i++)
			{
			showup(bmps_buff[i],0,0,800,480,1);
			sleep(2);
			}
			
		}
		if(tu.x > 250 && tu.x < 550 && tu.y > 360 && tu.y < 480)
			{
			printf("返回");
			break;
			}		
		
	}			
}
	





