/*********************************************************************************************************
* 模块名称：Main.c
* 摘    要：主文件，包含软硬件初始化函数和main函数
* 当前版本：1.0.0
* 作    者：SZLY(COPYRIGHT 2018 - 2020 SZLY. All rights reserved.)
* 完成日期：2020年01月01日
* 内    容：
* 注    意：注意勾选Options for Target 'Target1'->Code Generation->Use MicroLIB，否则printf无法使用                                                                  
**********************************************************************************************************
* 取代版本：
* 作    者：
* 完成日期：
* 修改内容：
* 修改文件：
*********************************************************************************************************/

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "cJSON.h"
#include "String.h"
#include "procHostCmd.h"
#include <stdlib.h>
#include "Main.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/


/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static uint16 Smp_Period = 1500;//x ms

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static  void  InitSoftware(void);   //初始化软件相关的模块
static  void  InitHardware(void);   //初始化硬件相关的模块
static  void  Proc2msTask(void);    //2ms处理任务
static  void  Proc1SecTask(void);   //1s处理任务

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitSoftware
* 函数功能：所有的软件相关的模块初始化函数都放在此函数中
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月11日
* 注    意：
*********************************************************************************************************/
static  void  InitSoftware(void)
{
  InitPackUnpack();       //初始化PackUnpack模块
  InitProcHostCmd();      //初始化ProcHostCmd模块
  InitSendDataToHost();   //初始化SendDataToHost模块
  InitRoute();            //初始化Route模块
}

/*********************************************************************************************************
* 函数名称：InitHardware
* 函数功能：所有的硬件相关的模块初始化函数都放在此函数中
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月11日
* 注    意：
*********************************************************************************************************/
static  void  InitHardware(void)
{  
  SystemInit();       //系统初始化
  InitRCC();          //初始化RCC模块
  InitNVIC();         //初始化NVIC模块
  InitUART1(9600);    //初始化UART模块
#if (defined SINK) && (SINK == TRUE)//汇聚节点
  InitUART2(115200);
#endif
  InitTimer();        //初始化Timer模块
  InitLED();          //初始化LED模块
  InitSysTick();      //初始化SysTick模块
  //InitDAC();          //初始化DAC模块
  InitADC();          //初始化ADC模块
  InitLORA();         //初始化LORA模块
}

/*********************************************************************************************************
* 函数名称：Proc2msTask
* 函数功能：2ms处理任务 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月11日
* 注    意：
*********************************************************************************************************/
static  void  Proc2msTask(void)
{  
  uint8  uart1RecData; //串口数据
  uint16 adcData;      //队列数据
  float waveData;     //波形数据

  static uint16 s_iCnt4 = 0;   //计数器
  static uint8 s_iPointCnt = 0;        //温度数据包的点计数器
  static uint8 s_arrData[70] = {0}; //初始化数组
  
  if(Get2msFlag())  //判断2ms标志状态
  {
    if(ReadUART1(&uart1RecData, 1)) //读串口接收数据
    {       
      ProcHostCmd(uart1RecData);  //处理命令      
    }

    srand(millis());
    s_iCnt4++;      //计数增加
    if(s_iCnt4 >= Smp_Period/2 + rand()%120+20)  //达到Smp_Period (ms)
    {
      if(ReadADCBuf(&adcData))  //从缓存队列中取出1个数据到adcData
      {
        ClearADCBuf();
        waveData = (float)adcData*(3.3 / 4096);
        waveData = (1.43 - waveData)/0.0043 + 25.0;  //计算获取温度的值，12位ADC，2^12=4095，参考电压3.3V
        s_arrData[s_iPointCnt] = (uint8)(int)waveData;  //存放到数组
        s_arrData[s_iPointCnt+1] = Smp_Period/100;
        s_arrData[s_iPointCnt+2] = getAddress()>>8;
        s_arrData[s_iPointCnt+3] = getAddress();
        
        s_iPointCnt++;  //温度数据包的点计数器加1操作

        if(s_iPointCnt >= 1)  //接收到x个点
        {
          #if (defined SINK) && (SINK == TRUE)//汇聚节点
          #else
          SendDateToParent(s_arrData, 2);
          //SendDateToParent(s_arrData, 70);
          //SendDateToParent(s_arrData, 70);
          #endif
          s_iPointCnt = 0;  //计数器清零
        }
      }
      s_iCnt4 = 0;  //准备下次的循环
    }
    
    LEDFlicker(250);//调用闪烁函数     
    Clr2msFlag();   //清除2ms标志
  }
}

/*********************************************************************************************************
* 函数名称：Proc1SecTask
* 函数功能：1s处理任务 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月11日
* 注    意：
*********************************************************************************************************/
static  void  Proc1SecTask(void)
{ 
  //char a[50] = {0};
  //int b=0;
  static uint8 s_iCnt = 0;   //计数器
  
  uint8 arrData[10] = {0};
  arrData[0] = 0x00;
  arrData[1] = 0x01;
  arrData[2] = 0xc0;
  arrData[3] = 0x00;
  arrData[4] = 0x02;
  arrData[5] = 0;
  
  if(Get1SecFlag()) //判断1s标志状态
  {
    //printf发送到UART1，即到LOAR模块
    //debug("This is the first STM32F103 Project, by Zhangsan\r\n");
    #if (defined SINK) && (SINK == TRUE)//汇聚节点
    ProcCloudCmd();    
    
    //SendCmdPack(0x00, 0x01, 0xff, 0x0001, 0);
    
    #else
    
    #endif
    s_iCnt++;
    
    if(s_iCnt >= 3 + rand()%3)//每n秒执行一次
    {
      s_iCnt = 0;
      RouteTimerTasks();
    }
    
    Clr1SecFlag();  //清除1s标志
  }    
}

/*********************************************************************************************************
* 函数名称：SetSmpPrd
* 函数功能：设置采样周期
* 输入参数：void
* 输出参数：void
* 返 回 值：uint8
* 创建日期：2022年3月18日23:19:30
* 注    意：
*********************************************************************************************************/
uint8  SetSmpPrd(uint8 Period)
{
  uint16 min = 1000;
  Smp_Period = 100*Period>min? 100*Period:min;//最快1S采样1次
}

/*********************************************************************************************************
* 函数名称：main
* 函数功能：主函数 
* 输入参数：void
* 输出参数：void
* 返 回 值：int
* 创建日期：2021年07月11日
* 注    意：
*********************************************************************************************************/
int main(void)
{ 
  InitHardware();   //初始化硬件相关函数
  InitSoftware();   //初始化软件相关函数
  
  debug("Init System has been finished.\r\n" );  //打印系统状态

  while(1)
  {
    Proc2msTask();  //2ms处理任务
    Proc1SecTask(); //1s处理任务   
  }
}
