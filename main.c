/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "base.h"
#include "lcd.h"
//#include "24cxx.h"
#include "touch.h"
#include "math.h"
#include  "stdio.h"
#include  "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */	
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
u8 num_off=0;
u8 num_off1=0;//操作数1坐标偏移
u8 num_off2=0;
u8 num_off21=0;//操作数2坐标偏移
u8 img_cnt=0;//虚数i最多只能出现两次
u8 pos_num;//定位末尾数字，用于退格
u8 op_1[7][1];//操作数1
u8 op_2[7][1];//操作数2
u8 is_optor;//判断是否按下运算符
int result=0;//保存结果
u32 op_num1 = 0;
u32 op_num2 = 0;//整型操作数
u16 cnt_1=0;
u16 cnt_2=0;
int len=0;//长度
u16 which_op=0;//判断哪一种运算
u16 len_data;//数据长度
int temp_int;//数值
u8 *arr_data;
/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
////////////////////////////////////////////////////////////////////////////////
//电容触摸屏专有部分
//画水平线
//x0,y0:坐标
//len:线长??
//color:颜色
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//画实心圆
//x0,y0:坐标
//r:半径
//color:颜色
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);//??   
 	POINT_COLOR=BLUE;//???????? 
	LCD_ShowString(lcddev.width-24,0,200,16,16,(u8 *)"RST");//??????
  POINT_COLOR=RED;//?????? 
}
void rtp_test(void)
{
	u8 key;
	u8 i=0;	  
	while(1)
	{
	 	key=KEY_Scan(0);
		tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)			//
		{	
		 	if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height)
			{	
				if(tp_dev.x[0]>(lcddev.width-24)&&tp_dev.y[0]<16)Load_Drow_Dialog();//
				else TP_Draw_Big_Point(tp_dev.x[0],tp_dev.y[0],RED);		//			   
			}
		}else delay_ms(10);	//
		if(key==KEY0_PRES)	//
		{
			LCD_Clear(WHITE);	//
		    TP_Adjust();  		//
			TP_Save_Adjdata();	 
			Load_Drow_Dialog();
		}
		i++;
		if(i%20==0)LED0_T;
	}
}
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{											  
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}

//两个数之差的绝对值
//x1,x2：需取差值的两个坐标
//返回值：|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//画一条粗??
//(x1,y1),(x2,y2):线条的起始坐??
//size：线条的粗细程度
//color：线条的颜色
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		gui_fill_circle(uRow,uCol,size,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}   
////////////////////////////////////////////////////////////////////////////////
//5个触控点的颜??(电容触摸屏用)												 
const u16 POINT_COLOR_TBL[5]={RED,GREEN,BLUE,BROWN,GRED}; 
void show_kb_num()//显示数字和运算符
{
LCD_Show_MyNum(25,290,0);
LCD_Show_MyNum(25,250,1);
LCD_Show_MyNum(25,210,4);
LCD_Show_MyNum(25,170,7);
LCD_Show_MyNum(25,130,12);//第一列

LCD_Show_MyNum(85,290,14);
LCD_Show_MyNum(85,250,2);
LCD_Show_MyNum(85,210,5);
LCD_Show_MyNum(85,170,8);
LCD_Show_HZ_Font(80,130,8);//第二列

LCD_Show_MyNum(145,290,15);
LCD_Show_MyNum(145,250,3);
LCD_Show_MyNum(145,210,6);
LCD_Show_MyNum(145,170,9);
LCD_Show_HZ_Font(139,130,7);//第三列

LCD_Show_MyNum(205,290,13);
LCD_Show_HZ_Font(201,250,6);
LCD_Show_HZ_Font(201,210,5);
LCD_Show_MyNum(205,170,11);
LCD_Show_MyNum(205,130,10);//第四列
LCD_Show_HZ_Font(190,0,9);
LCD_Show_HZ_Font(215,0,10);//提示返回
}
void draw_keyboard(){//用直线作格子

	for(int i=0;i<5;i++)
{
LCD_DrawLine(0,120+i*40,lcddev.width,120+i*40);
}
 for(int j=0;j<3;j++)
{
LCD_DrawLine(60+j*(lcddev.width/4),120,60+j*(lcddev.width/4),320);
}
 show_kb_num();
}
u16 getlenth(u32 num)//获取长度
{
while(num/10!=0)
{
	num=num/10;
  len++;
}
return len;
}
void usart_receive_data()//从串口获取数据
{
	{
	 u8 times=0;
    {	
       if(USART_RX_STA&0x8000)
		{		
			//LCD_Fill(35,50,220,80,WHITE);		//清空上次数据   
			len_data=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
			printf("\r\n您发送的消息为:\r\n");
		  HAL_UART_Transmit(&UART1_Handler,(uint8_t*)USART_RX_BUF,len_data,1000);	//发送接收到的数据
			while(__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_TC)!=SET);	//等待发送结束
			printf("\r\n\r\n");//插入换行
			USART_RX_STA=0;
		}else
		{
			times++;
			if(times%5000==0)
			{
				printf("-----串口正在工作-----");
			}
			if(times%200==0)printf("请输入数据,以回车键结束\r\n");  
			delay_ms(10);   
		} 
}

}
}

char *Reverse(char *s)//反转字符串
{
    char temp;
    char *p = s; //首字母
    char *q=s+len;
    while (q>p) 
    {
        temp = *p;
        *p++ = *q;
        *q-- = temp;
    }
    return s; 
}

void My_atoI(){//字符转数字
	  temp_int=0;
    for(int j=0;j<len_data;j++)
    {
        temp_int = pow(10, (len_data - j - 1))*(arr_data[j]- 48) + temp_int;
  }
}


u8 *My_Itoa(int n)//数值转字符
{
    int i = 0;
    static char s[10];
    do{
        s[i++] = n % 10 + '0';
        n = n / 10;
    } while (n>0);
    s[i] = '\0';
   return Reverse(s);
}

void My_Itoa_Usart(int n)//数值转字符
{
	int i=len_data;
    do{
        arr_data[i-1] = n % 10 + '0';
        n = n / 10;
			  i--;
    } while (n>0);
}

//乘法
void mul(u8 op1[][1], u8 op2[][1])
{
    for (int i = 1; i<7; i++)
    {
        if (op1[i][0] != '\0')cnt_1++;
        if (op2[i][0] != '\0')cnt_2++;
    }

    for (int i = 1; i<=cnt_1; i++)
    {
        op_num1 = ((int)op1[i][0] - '0')*pow(10,cnt_1-i)+op_num1;
    }
    for (int j = 1; j<=cnt_2; j++)
    {
        op_num2 = ((int)op2[j][0] - '0')*pow(10,cnt_2-j)+op_num2;
    }
    result=op_num1*op_num2;
		len=getlenth(result);
}


//除法
void divi(u8 op1[][1], u8 op2[][1])
{
   for (int i = 1; i<7; i++)
    {
        if (op1[i][0] != '\0')cnt_1++;
        if (op2[i][0] != '\0')cnt_2++;
    }
    for (int i = 1; i<=cnt_1; i++)
    {
        op_num1 = ((int)op1[i][0] - '0')*pow(10,cnt_1-i)+op_num1;
    }
    for (int j = 1; j<=cnt_2; j++)
    {
        op_num2 = ((int)op2[j][0] - '0')*pow(10,cnt_2-j)+op_num2;
    }
		if(op_num2!=0)//除数为0
		result=op_num1/op_num2;
		else
		result=-1;
}


//减法
void dif(u8 op1[][1],u8 op2[][1]){
	 for (int i = 1; i<7; i++)
    {
        if (op1[i][0] != '\0')cnt_1++;
        if (op2[i][0] != '\0')cnt_2++;
    }

    for (int i = 1; i<=cnt_1; i++)
    {
        op_num1 = ((int)op1[i][0] - '0')*pow(10,cnt_1-i)+op_num1;
    }
    for (int j = 1; j<=cnt_2; j++)
    {
        op_num2 = ((int)op2[j][0] - '0')*pow(10,cnt_2-j)+op_num2;
    }
    result=op_num1-op_num2;
		len=getlenth(result);
}


//加法
void add(u8 op1[][1],u8 op2[][1]){
	 for (int i = 1; i<7; i++)
    {
        if (op1[i][0] != '\0')cnt_1++;
        if (op2[i][0] != '\0')cnt_2++;
    }

    for (int i = 1; i<=cnt_1; i++)
    {
        op_num1 = ((int)op1[i][0] - '0')*pow(10,cnt_1-i)+op_num1;
    }
    for (int j = 1; j<=cnt_2; j++)
    {
        op_num2 = ((int)op2[j][0] - '0')*pow(10,cnt_2-j)+op_num2;
    }
    result=op_num1+op_num2;
		len=getlenth(result);

}
void my_show()//显示
	{
	POINT_COLOR=RED;
	LCD_ShowString(30,40,200,24,24, (u8 *)"20201111335");
	LCD_ShowString(30,70,200,16,16, (u8 *)"LiangHaojian");
	LCD_Show_Pi(29,91);//圆周率
	LCD_ShowString(45,90,200,16,16, (u8 *)"=3.1415926");
	LCD_ShowString(30,110,200,12,12,(u8 *)"2021/10/21");
	LCD_Draw_Circle(50,200,30);
	LCD_Show_HZ_Font(lcddev.width-200,190,0);//开	
	LCD_Draw_Circle(150,200,30);
	LCD_Show_HZ_Font(lcddev.width-100,190,1);//关
	
	//矩形1
	LCD_DrawLine(lcddev.width-225,165,lcddev.width-55,165);
	LCD_DrawLine(lcddev.width-225,240,lcddev.width-55,240);//长
	LCD_DrawLine(lcddev.width-225,165,lcddev.width-225,240);
	LCD_DrawLine(lcddev.width-55,165,lcddev.width-55,240);//宽
	
  //矩形2
	LCD_DrawLine(lcddev.width-225,255,lcddev.width-140,255);
	LCD_DrawLine(lcddev.width-225,295,lcddev.width-140,295);//长
	LCD_DrawLine(lcddev.width-225,255,lcddev.width-225,295);
	LCD_DrawLine(lcddev.width-140,255,lcddev.width-140,295);//宽
	
	//矩形3
	LCD_DrawLine(lcddev.width-105,255,lcddev.width-55,255);
	LCD_DrawLine(lcddev.width-105,295,lcddev.width-55,295);//长
	LCD_DrawLine(lcddev.width-105,255,lcddev.width-105,295);
	LCD_DrawLine(lcddev.width-55,255,lcddev.width-55,295);//宽
	
	//计算器
	LCD_Show_HZ_Font(lcddev.width-220,260,2);
	LCD_Show_HZ_Font(lcddev.width-195,260,3);
	LCD_Show_HZ_Font(lcddev.width-170,260,4);
	
	//串口
	LCD_Show_HZ_Font(lcddev.width-102,260,11);
	LCD_Show_HZ_Font(lcddev.width-80,260,12);
	}
	
void usart_show()//串口界面显示
{
	 LCD_Show_HZ_Font(190,0,9);
   LCD_Show_HZ_Font(215,0,10);//提示返回
	 LCD_DrawLine(25,97,50,97);
	 LCD_DrawLine(25,125,50,125);//长
	 LCD_DrawLine(25,97,25,125);
	 LCD_DrawLine(50,97,50,125);//宽
	
	 LCD_DrawLine(205,97,230,97);
	 LCD_DrawLine(205,125,230,125);//长
	 LCD_DrawLine(205,97,205,125);
	 LCD_DrawLine(230,97,230,125);//宽
	
   LCD_Show_MyNum(30,100,10);//显示加号		
   LCD_Show_MyNum(210,100,11);//显示减号	
	
   //LCD_ShowString(0,0,200,24,24,(u8*)"RES");		



}
void scan_num1()//识别数字1
{ 
	tp_dev.scan(0);//扫描屏幕坐标
	HAL_Delay(200);
		 if(tp_dev.x[0]>0&&tp_dev.x[0]<60&&tp_dev.y[0]>160&&tp_dev.y[0]<200)//数字7
	{
	 LCD_Show_MyBigNum(0+num_off*20,80,7);
	 num_off++;//显示之后才偏移
	 num_off1++;
	 op_1[num_off][0]='7';//利用num__off存放操作数
	}
		else if(tp_dev.x[0]>0&&tp_dev.x[0]<60&&tp_dev.y[0]>200&&tp_dev.y[0]<240)//数字4
	{
	  LCD_Show_MyBigNum(0+num_off*20,80,4);
		num_off++;
		num_off1++;
		op_1[num_off][0]='4';//利用num__off存放操作数
	}
		else if(tp_dev.x[0]>0&&tp_dev.x[0]<60&&tp_dev.y[0]>240&&tp_dev.y[0]<280)//数字1
	{
	 LCD_Show_MyBigNum(0+num_off*20,80,1);
	 num_off++;
	 num_off1++;
		op_1[num_off][0]='1';//利用num__off存放操作
	}
		else if(tp_dev.x[0]>0&&tp_dev.x[0]<60&&tp_dev.y[0]>280&&tp_dev.y[0]<320)//数字0
	{
	 LCD_Show_MyBigNum(0+num_off*20,80,0);
	 num_off++;
	 num_off1++;
	 op_1[num_off][0]='0';//利用num__off存放操作数
	}
		 else if(tp_dev.x[0]>60&&tp_dev.x[0]<120&&tp_dev.y[0]>160&&tp_dev.y[0]<200)//数字8
	{
	 LCD_Show_MyBigNum(0+num_off*20,80,8);
	 num_off++;
	 num_off1++;
	 op_1[num_off][0]='8';//利用num__off存放操作数

	}
		else if(tp_dev.x[0]>60&&tp_dev.x[0]<120&&tp_dev.y[0]>200&&tp_dev.y[0]<240)//数字5
	{
	 LCD_Show_MyBigNum(0+num_off*20,80,5);
	 num_off++;
	 num_off1++;
	 op_1[num_off][0]='5';//利用num__off存放操作数
		
	}
		else if(tp_dev.x[0]>60&&tp_dev.x[0]<120&&tp_dev.y[0]>240&&tp_dev.y[0]<280)//数字2
	{
	 LCD_Show_MyBigNum(0+num_off*20,80,2);
	 num_off++;
	 num_off1++;
	 op_1[num_off][0]='2';//利用num__off存放操作数
		
	}
		else if(tp_dev.x[0]>60&&tp_dev.x[0]<120&&tp_dev.y[0]>280&&tp_dev.y[0]<320&&img_cnt<2)//虚数单位i
	{
	 LCD_Show_MyNum(0+num_off*20,85,14);
	 num_off++;
	 num_off1++;
	 img_cnt++;
		
	}
	 else if(tp_dev.x[0]>120&&tp_dev.x[0]<160&&tp_dev.y[0]>160&&tp_dev.y[0]<200)//数字9
	{
	 LCD_Show_MyBigNum(0+num_off*20,80,9);
	 num_off++;
	 num_off1++;
	 op_1[num_off][0]='9';//利用num__off存放操作数
		
	}
	 else if(tp_dev.x[0]>120&&tp_dev.x[0]<160&&tp_dev.y[0]>200&&tp_dev.y[0]<240)//数字6
	{
	 LCD_Show_MyBigNum(0+num_off*20,80,6);
	 num_off++;
	 num_off1++;
	 op_1[num_off][0]='6';//利用num__off存放操作数
	}
	 else if(tp_dev.x[0]>120&&tp_dev.x[0]<160&&tp_dev.y[0]>240&&tp_dev.y[0]<280)//数字3
	{
	 LCD_Show_MyBigNum(0+num_off*20,80,3);
	 num_off++;
	 num_off1++;
	 op_1[num_off][0]='3';//利用num__off存放操作数
	}
	 else if(tp_dev.x[0]>120&&tp_dev.x[0]<160&&tp_dev.y[0]>280&&tp_dev.y[0]<320)//小数点
	{
	 LCD_Show_MyNum(0+num_off*20,80,15);
	 num_off++;
	 num_off1++;
	}
	 else if(tp_dev.x[0]>0&&tp_dev.x[0]<60&&tp_dev.y[0]>120&&tp_dev.y[0]<160)//清空键
	{
	LCD_Fill(0,60,250,115,WHITE);//清空数字
	LCD_Fill(0,30,180,60,WHITE);//清空数字
			num_off=0;
	    num_off1=0;
		  num_off2=0;
	    num_off21=0;
		  cnt_1=0;
		  cnt_2=0;
	    img_cnt=0;
		  op_num1=0;
	    op_num2=0;//重置操作数
	    is_optor=0;
				for(int i=1;i<7;i++)
		  {
			op_1[i][0]=0;
		  op_2[i][0]=0;
			}
	}
	 /*else if(tp_dev.x[0]>120&&tp_dev.x[0]<180&&tp_dev.y[0]>120&&tp_dev.y[0]<160)//退格
	{
	  pos_num=0+(num_off1)*20;
    LCD_Fill(pos_num,85,pos_num+20,112,WHITE);
		op_1[num_off1][0]='\0';//清空该数字
		if(num_off1>0)
		{
			num_off1--;
			if(num_off-num_off1==2)
			{
				num_off--;
			}
		}防止取值越界
		
	}*/
		 if(tp_dev.x[0]>180&&tp_dev.x[0]<240&&tp_dev.y[0]>200&&tp_dev.y[0]<240)//乘
	{		
	
		LCD_Fill(0,60,250,115,WHITE);//清空操作数1
	  LCD_Show_HZ_Font(3,60,5);//显示乘号
	 for(int i=1;i<=num_off;i++)//操作数1上移
	{
		LCD_ShowChar(-5+12*i,40,op_1[i][0],16,0);
		}
		img_cnt=0;//显示后重置
		is_optor++;
	}
	
	 if(tp_dev.x[0]>180&&tp_dev.x[0]<240&&tp_dev.y[0]>160&&tp_dev.y[0]<200)//减
	{
			LCD_Fill(0,60,250,115,WHITE);//清空操作数1
	  LCD_Show_MyNum(3,60,11);//显示减号
	 for(int i=1;i<=num_off;i++)//操作数1上移
	{
		LCD_ShowChar(-5+12*i,40,op_1[i][0],16,0);
		}
		img_cnt=0;//显示后重置
		is_optor++;
	}
	
	 if(tp_dev.x[0]>180&&tp_dev.x[0]<240&&tp_dev.y[0]>120&&tp_dev.y[0]<160)//加
	{
		LCD_Fill(0,60,250,115,WHITE);//清空操作数1
	  LCD_Show_MyNum(3,60,10);//显示加号
	 for(int i=1;i<=num_off;i++)//操作数1上移
	{
		LCD_ShowChar(-5+12*i,40,op_1[i][0],16,0);
		}
		img_cnt=0;//显示后重置
		is_optor++;
	}
}
	
void scan_num2()
{	
  tp_dev.scan(0);//扫描屏幕坐标
	HAL_Delay(200);
		if(tp_dev.x[0]>0&&tp_dev.x[0]<60&&tp_dev.y[0]>160&&tp_dev.y[0]<200)//数字7
	{
	 LCD_Show_MyBigNum(0+num_off*20,80,7);
	 num_off2++;//显示之后才偏移
	 num_off21++;
	 op_2[num_off2][0]='7';//利用num__off存放操作数
	}
		if(tp_dev.x[0]>0&&tp_dev.x[0]<60&&tp_dev.y[0]>200&&tp_dev.y[0]<240)//数字4
	{
	  LCD_Show_MyBigNum(0+num_off2*20,80,4);
		num_off2++;
		num_off21++;
		op_2[num_off2][0]='4';//利用num__off存放操作数
	}
		if(tp_dev.x[0]>0&&tp_dev.x[0]<60&&tp_dev.y[0]>240&&tp_dev.y[0]<280)//数字1
	{
	 LCD_Show_MyBigNum(0+num_off2*20,80,1);
	 num_off2++;
	 num_off21++;
		op_2[num_off2][0]='1';//利用num__off存放操作数
	}
		if(tp_dev.x[0]>0&&tp_dev.x[0]<60&&tp_dev.y[0]>280&&tp_dev.y[0]<320)//数字0
	{
	 LCD_Show_MyBigNum(0+num_off2*20,80,0);
	 num_off2++;
	 num_off21++;
	 op_2[num_off2][0]='0';//利用num__off存放操作数
	}
		 if(tp_dev.x[0]>60&&tp_dev.x[0]<120&&tp_dev.y[0]>160&&tp_dev.y[0]<200)//数字8
	{
	 LCD_Show_MyBigNum(0+num_off2*20,80,8);
	 num_off2++;
	 num_off21++;
	 op_2[num_off2][0]='8';//利用num__off存放操作数
	}
		if(tp_dev.x[0]>60&&tp_dev.x[0]<120&&tp_dev.y[0]>200&&tp_dev.y[0]<240)//数字5
	{
	 LCD_Show_MyBigNum(0+num_off2*20,80,5);
	 num_off2++;
	 num_off21++;
	 op_2[num_off2][0]='5';//利用num__off存放操作数
		
	}
		if(tp_dev.x[0]>60&&tp_dev.x[0]<120&&tp_dev.y[0]>240&&tp_dev.y[0]<280)//数字2
	{
	 LCD_Show_MyBigNum(0+num_off2*20,80,2);
	 num_off2++;
	 num_off21++;
	 op_2[num_off2][0]='2';//利用num__off存放操作数
	}
		if(tp_dev.x[0]>60&&tp_dev.x[0]<120&&tp_dev.y[0]>280&&tp_dev.y[0]<320&&img_cnt<2)//虚数单位i
	{
	 LCD_Show_MyNum(0+num_off2*20,85,14);
	 num_off2++;
	 num_off21++;
	 img_cnt++;
	 op_2[num_off2][0]='i';//利用num__off存放操作数
	}
	 if(tp_dev.x[0]>120&&tp_dev.x[0]<160&&tp_dev.y[0]>160&&tp_dev.y[0]<200)//数字9
	{
	 LCD_Show_MyBigNum(0+num_off2*20,80,9);
	 num_off2++;
	 num_off21++;
	 op_2[num_off2][0]='9';//利用num__off存放操作数
	}
		if(tp_dev.x[0]>120&&tp_dev.x[0]<160&&tp_dev.y[0]>200&&tp_dev.y[0]<240)//数字6
	{
	 LCD_Show_MyBigNum(0+num_off2*20,80,6);
	 num_off2++;
	 num_off21++;
	 op_2[num_off2][0]='6';//利用num__off存放操作数
	}
		if(tp_dev.x[0]>120&&tp_dev.x[0]<160&&tp_dev.y[0]>240&&tp_dev.y[0]<280)//数字3
	{
	 LCD_Show_MyBigNum(0+num_off2*20,80,3);
	 num_off2++;
	 num_off21++;
	 op_2[num_off2][0]='3';//利用num__off存放操作数
	}
		if(tp_dev.x[0]>120&&tp_dev.x[0]<160&&tp_dev.y[0]>280&&tp_dev.y[0]<320)//小数点
	{
	 LCD_Show_MyNum(0+num_off2*20,80,15);
	 num_off2++;
	 num_off21++;
	}
	  if(tp_dev.x[0]>0&&tp_dev.x[0]<60&&tp_dev.y[0]>120&&tp_dev.y[0]<160)//清空键
	{
	LCD_Fill(0,30,250,115,WHITE);//清空数字
	LCD_Fill(0,30,180,60,WHITE);//清空数字
			num_off=0;
	    num_off1=0;
		  num_off2=0;
	    num_off21=0;
		  cnt_1=0;
		  cnt_2=0;
	    img_cnt=0;
		  op_num1=0;
	    op_num2=0;//重置操作数
	    is_optor=0;
				for(int i=1;i<7;i++)
		  {
			op_1[i][0]=0;
		  op_2[i][0]=0;
			}
	}
	 /*if(tp_dev.x[0]>120&&tp_dev.x[0]<180&&tp_dev.y[0]>120&&tp_dev.y[0]<160)//退格
	{
	  pos_num=0+(num_off21)*20;
    LCD_Fill(pos_num,85,pos_num+20,112,WHITE);
		op_2[num_off21][0]='\0';//清空该数字
		if(num_off21>0)
		{
			num_off21--;
			if(num_off2-num_off21==2)
			{
				num_off2--;
			}
		}//防止取值越界
		
		
	}*/
	if(tp_dev.x[0]>180&&tp_dev.x[0]<240&&tp_dev.y[0]>240&&tp_dev.y[0]<280)//除
	 {
			LCD_Fill(0,60,250,115,WHITE);//清空操作数1
	  LCD_Show_HZ_Font(3,60,6);//显示除号
	 for(int i=1;i<=num_off;i++)//操作数1上移
	{
		LCD_ShowChar(-5+12*i,40,op_1[i][0],16,0);
	}
	  num_off2=0;
		img_cnt=0;
	  is_optor++;
    //显示后重置
	which_op=1;//1代表除法
	}
	 
	 if(tp_dev.x[0]>180&&tp_dev.x[0]<240&&tp_dev.y[0]>200&&tp_dev.y[0]<240)//乘
	{		
	
		LCD_Fill(0,60,250,115,WHITE);//清空操作数1
	  LCD_Show_HZ_Font(3,60,5);//显示乘号
	 for(int i=1;i<=num_off;i++)//操作数1上移
	{
		LCD_ShowChar(-5+12*i,40,op_1[i][0],16,0);
		}
	  num_off2=0;
		img_cnt=0;//显示后重置
		is_optor++;
		which_op=2;//2代表乘法
	}
	
	 if(tp_dev.x[0]>180&&tp_dev.x[0]<240&&tp_dev.y[0]>160&&tp_dev.y[0]<200)//减
	{
			LCD_Fill(0,60,250,115,WHITE);//清空操作数1
	  LCD_Show_MyNum(3,60,11);//显示减号
	 for(int i=1;i<=num_off;i++)//操作数1上移
	{
		LCD_ShowChar(-5+12*i,40,op_1[i][0],16,0);
		}
	  num_off2=0;
		img_cnt=0;//显示后重置
		is_optor++;
			which_op=3;//3代表减法
	}
	
	 if(tp_dev.x[0]>180&&tp_dev.x[0]<240&&tp_dev.y[0]>120&&tp_dev.y[0]<160)//加
	{
		LCD_Fill(0,60,250,115,WHITE);//清空操作数1
	  LCD_Show_MyNum(3,60,10);//显示加号
	 for(int i=1;i<=num_off;i++)//操作数1上移
	{
		LCD_ShowChar(-5+12*i,40,op_1[i][0],16,0);
		}
	  num_off2=0;
		img_cnt=0;//显示后重置
		is_optor++;
			which_op=4;//4代表加法
	}
	if(tp_dev.x[0]>180&&tp_dev.x[0]<240&&tp_dev.y[0]>280&&tp_dev.y[0]<320)//等于
	{
		switch(which_op)
  {
			case 1:{
			divi(op_1,op_2);
				if(result!=-1)
				{
			u8* result_str=My_Itoa(result);//整形转字符串
		  LCD_ShowString(95,60,200,24,24,(u8*)"=");
		  LCD_ShowString(120,60,200,24,24,result_str);
			num_off=0;
	    num_off1=0;
		  num_off2=0;
	    num_off21=0;
		  cnt_1=0;
		  cnt_2=0;
	    img_cnt=0;
		  op_num1=0;
	    op_num2=0;
		  len=0;//重置操作数
					}
			else 
			{
				LCD_ShowString(95,60,200,24,24,(u8*)"Error!");//除数为0
			}
		for(int i=1;i<7;i++)
		  {
			op_1[i][0]=0;
		  op_2[i][0]=0;
			}
			}break;
			case 2:{//乘法
			mul(op_1,op_2);
		  u8* result_str=My_Itoa(result);//整形转字符串
		  LCD_ShowString(95,60,200,24,24,(u8*)"=");
		  LCD_ShowString(120,60,200,24,24,result_str);
			num_off=0;
	    num_off1=0;
		  num_off2=0;
	    num_off21=0;
		  cnt_1=0;
		  cnt_2=0;
	    img_cnt=0;
		  op_num1=0;
	    op_num2=0;
		  len=0;//重置操作数
		for(int i=1;i<7;i++)
		  {
			op_1[i][0]=0;
		  op_2[i][0]=0;
			}
			}break;
			
			case 3:{//减法
			dif(op_1,op_2);
			u8* result_str=My_Itoa(result);//整形转字符串
		  LCD_ShowString(95,60,200,24,24,(u8*)"=");
		  LCD_ShowString(120,60,200,24,24,result_str);
			num_off=0;
	    num_off1=0;
		  num_off2=0;
	    num_off21=0;
		  cnt_1=0;
		  cnt_2=0;
	    img_cnt=0;
		  op_num1=0;
	    op_num2=0;
		  len=0;//重置操作数
				for(int i=1;i<7;i++)
		  {
			op_1[i][0]=0;
		  op_2[i][0]=0;
			}
			}break;
			
			case 4:{//加法
				add(op_1,op_2);
		  u8* result_str=My_Itoa(result);//整形转字符串
		  LCD_ShowString(95,60,200,24,24,(u8*)"=");
		  LCD_ShowString(120,60,200,24,24,result_str);
			num_off=0;
	    num_off1=0;
		  num_off2=0;
	    num_off21=0;
		  cnt_1=0;
		  cnt_2=0;
	    img_cnt=0;
		  op_num1=0;
	    op_num2=0;
		  len=0;//重置操作数
			for(int i=1;i<7;i++)
		  {
			op_1[i][0]=0;
		  op_2[i][0]=0;
			}
			}break;

}
			}
	}

 //计算器 
void calculator()
	{		
		draw_keyboard();//画键盘
		is_optor=0;
		while(1)
		{
			usart_receive_data();
			tp_dev.scan(0);//扫描屏幕坐标
	  if(tp_dev.x[0]<180&&!is_optor)//扫描屏幕坐标,未按下操作符扫描数字1
	{   
		scan_num1();
	}
	else
	{
	  scan_num2();
	 }
	if(temp_int==3)
 {
	LCD_Clear(WHITE);
	num_off=0;
	num_off1=0;
	img_cnt=0;
	break;
 }//退出计算器
	if(tp_dev.x[0]>180&&tp_dev.x[0]<240&&tp_dev.y[0]>0&&tp_dev.y[0]<30)//返回
	{
	LCD_Clear(WHITE);
	num_off=0;
	num_off1=0;
	img_cnt=0;
	break;
	}
		}
}
	


//显示并处理从串口处理的数据	

void show_data()
{ 
	while(1)
	{
		usart_receive_data();
		arr_data=USART_RX_BUF;
	  My_atoI();//字符转数值
		usart_show();
		
 	  LCD_ShowString(95,100,200,24,24,arr_data);	
  	tp_dev.scan(0);//扫描屏幕坐标			
   if(tp_dev.x[0]>25&&tp_dev.x[0]<50&&tp_dev.y[0]>95&&tp_dev.y[0]<125)//加1
			{
				HAL_Delay(200);
				temp_int=temp_int+1;
			
				LCD_Fill(55,95,200,120,WHITE);
				My_Itoa_Usart(temp_int);//转成字符
   			LCD_ShowString(95,100,200,24,24,arr_data);
				tp_dev.x[0]=0;
				tp_dev.y[0]=0;
			}
			 else if(tp_dev.x[0]>205&&tp_dev.x[0]<230&&tp_dev.y[0]>95&&tp_dev.y[0]<125)//减1
			 {
				 HAL_Delay(200);
				temp_int=temp_int-1;
			
				LCD_Fill(55,95,200,120,WHITE);
				My_Itoa_Usart(temp_int);//转成字符

   			LCD_ShowString(95,100,200,24,24,arr_data);
				tp_dev.x[0]=0;
				tp_dev.y[0]=0;
			 }
			 if(temp_int==0)//控制LED，0表示灭灯
				 {
			 HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_SET);//关灯
			 }
				 else if(temp_int==1)
					{
				 HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET);//开灯
				 }
					else if(temp_int==2)//计算器
						{
					LCD_Clear(WHITE);
					calculator();
					}
//			if(tp_dev.x[0]>0&&tp_dev.x[0]<25&&tp_dev.y[0]>0&&tp_dev.y[0]<25) 		//复位	 
//			{
//				LCD_Fill(55,95,210,120,WHITE);		
//				u16 len_data=0;//数据长度
//				int temp_int=0;//数值
//				usart_receive_data();
//			}
		if(tp_dev.x[0]>180&&tp_dev.x[0]<240&&tp_dev.y[0]>0&&tp_dev.y[0]<30)//返回
			{
		    LCD_Clear(WHITE);
				u16 len_data=0;//数据长度
				int temp_int=0;//数值
				for(int i=0;i<=len_data;i++)
				USART_RX_BUF[i]='\0';//清空
		   break;
		}
	}
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)

{
  /* USER CODE BEGIN 1 */
	/* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
	MX_GPIO_Init();
	uart_init(115200);					//初始化串口
  /* USER CODE BEGIN 2 */
  delay_init(72);
	LCD_Init();//初始化屏幕
  tp_dev.init();
  //rtp_test();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
			u8 LED_Sta=0;
  while (1)
 {
		my_show();//显示基本信息
		u16 Touch_XY[2][2];//触屏坐标
		//功能
		//触屏实现开灯和关灯	  

			tp_dev.scan(0);//扫描屏幕坐标
			HAL_Delay(50);//防误触
			Touch_XY[LED_Sta][0]=tp_dev.x[0];
			Touch_XY[LED_Sta][1]=tp_dev.y[0];
			if(Touch_XY[LED_Sta][0]>20&&Touch_XY[LED_Sta][0]<80&&Touch_XY[LED_Sta][1]>170&&Touch_XY[LED_Sta][1]<230)
		{
			HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET);//开灯
		}
			if(Touch_XY[LED_Sta][0]>120&&Touch_XY[LED_Sta][0]<180&&Touch_XY[LED_Sta][1]>170&&Touch_XY[LED_Sta][1]<230)
		{
			HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_SET);//关灯
		}
		
		if(tp_dev.x[0]>(lcddev.width-225)&&tp_dev.x[0]<(lcddev.width-140)&&tp_dev.y[0]>255&&tp_dev.y[0]<295)//进入计算器
		{
			LCD_Clear(WHITE);
			calculator();
		};
		
		if(tp_dev.x[0]>(lcddev.width-105)&&tp_dev.x[0]<(lcddev.width-55)&&tp_dev.y[0]>255&&tp_dev.y[0]<295)//进入串口通信
		{
		   LCD_Clear(WHITE);
			 show_data();
		}
  /* USER CODE END 3 */

/**
  * @brief System Clock Configuration
  * @retval None
*/
}
 }
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
   // Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    //Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}



#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
